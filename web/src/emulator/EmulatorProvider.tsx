// web/src/emulator/EmulatorProvider.tsx
import {
  createContext,
  useCallback,
  useContext,
  useEffect,
  useMemo,
  useRef,
  useState,
  type ReactNode,
} from "react";
import { createBridge, type Debugger, type WasmModule } from "../wasm/bridge";
import { loadEmulatorModule } from "../wasm/loader";
import { parseOpcodes } from "../wasm/opcodes";
import type { EmulatorSnapshot, EmulatorStatus } from "../wasm/types";
import { useFrameLoop } from "./useFrameLoop";
import { NesAudio } from "./audio";
import { parseMovie } from "./movie";
import { useToast } from "../components/toast/ToastProvider";

// The live-agent server (python -m nesenv.live) streams Server-Sent Events to here.
// This is a local dev / research feature: in production the default points at the
// visitor's own machine (and an HTTPS page blocks plain-http localhost anyway), so
// the Spawn Agent button is hidden unless VITE_LIVE_AGENT_URL is set (see Toolbar).
const LIVE_AGENT_URL =
  (import.meta.env.VITE_LIVE_AGENT_URL as string | undefined) ??
  "http://localhost:8000/stream";

function base64ToBytes(b64: string): Uint8Array {
  const bin = atob(b64);
  const out = new Uint8Array(bin.length);
  for (let i = 0; i < bin.length; i += 1) out[i] = bin.charCodeAt(i);
  return out;
}

export interface EmulatorActions {
  step(): void;
  run(): void;
  stop(): void;
  reset(): void;
  addBreakpoint(addr: number): void;
  removeBreakpoint(addr: number): void;
  toggleBreakpoint(addr: number): void;
  writeMemory(addr: number, value: number): void;
  loadROM(data: Uint8Array): void;
  loadRom(data: Uint8Array): number;
  loadOpcodes(text: string): void;
  setController(state: number, port?: number): void;
  playMovie(data: Uint8Array): void;
  stopMovie(): void;
  connectLiveAgent(url?: string): void;
  disconnectLiveAgent(): void;
}

export interface EmulatorContextValue {
  status: EmulatorStatus;
  snapshot: EmulatorSnapshot | null;
  breakpoints: number[];
  running: boolean;
  framebuffer: Uint8ClampedArray | null;
  movie: { playing: boolean; frame: number; total: number };
  liveAgent: { connected: boolean; frame: number };
  dbg: Debugger | null;
  actions: EmulatorActions;
}

const EmulatorContext = createContext<EmulatorContextValue | null>(null);

export function EmulatorProvider(props: {
  children: ReactNode;
  loadModule?: () => Promise<WasmModule>;
}): JSX.Element {
  const { loadModule = loadEmulatorModule } = props;
  const { addToast } = useToast();
  const dbgRef = useRef<Debugger | null>(null);
  const [dbg, setDbg] = useState<Debugger | null>(null);
  const [status, setStatus] = useState<EmulatorStatus>("loading");
  const [snapshot, setSnapshot] = useState<EmulatorSnapshot | null>(null);
  const [breakpoints, setBreakpoints] = useState<number[]>([]);
  const [framebuffer, setFramebuffer] = useState<Uint8ClampedArray | null>(
    null,
  );
  const [movie, setMovie] = useState({ playing: false, frame: 0, total: 0 });
  const movieRafRef = useRef<number | null>(null);
  const [liveAgent, setLiveAgent] = useState({ connected: false, frame: 0 });
  const liveSourceRef = useRef<EventSource | null>(null);
  const liveRomRef = useRef<Uint8Array | null>(null);
  // Mirror the breakpoints state in a ref so toggleBreakpoint can read the
  // current set synchronously without doing side-effects inside a setState
  // updater (updaters must stay pure).
  const breakpointsRef = useRef<number[]>([]);
  useEffect(() => {
    breakpointsRef.current = breakpoints;
  }, [breakpoints]);

  const publishSnapshot = useCallback((s: EmulatorSnapshot) => {
    setSnapshot(s);
  }, []);

  const handleBreak = useCallback(() => {
    addToast("Breakpoint hit", "warning");
  }, [addToast]);

  const handleBrk = useCallback(() => {
    addToast("Program terminated with BRK", "info");
  }, [addToast]);

  const audioRef = useRef(new NesAudio());

  const handleFrame = useCallback((fb: Uint8ClampedArray) => {
    // Copy out of the WASM heap view so React state holds a stable buffer
    // (the heap view is reused/invalidated by the next frame).
    setFramebuffer(new Uint8ClampedArray(fb));
    // Drain this frame's audio and queue it for playback.
    const bridge = dbgRef.current;
    if (bridge) {
      const samples = bridge.audioDrain(4096);
      if (samples.length) audioRef.current.pump(samples);
    }
  }, []);

  // Release the audio context when the provider unmounts.
  useEffect(() => {
    const audio = audioRef.current;
    return () => audio.close();
  }, []);

  const {
    start: startLoop,
    stop: stopLoop,
    running,
  } = useFrameLoop({
    dbg,
    onFrame: handleFrame,
    onSnapshot: publishSnapshot,
    onBreak: handleBreak,
    onBrk: handleBrk,
  });

  useEffect(() => {
    let cancelled = false;
    loadModule()
      .then((module) => {
        if (cancelled) return;
        const bridge = createBridge(module);
        dbgRef.current = bridge;
        setDbg(bridge);
        setSnapshot(bridge.getSnapshot());
        // Publish the framebuffer once at load so the Screen shows the PPU's
        // current output (or the DEV demo image) at idle — the run loop only
        // calls onFrame while running, so without this the canvas stays black
        // until the user hits Run. Copy out of the heap view (it's reused).
        try {
          const fb = bridge.getFramebuffer();
          setFramebuffer(new Uint8ClampedArray(fb));
        } catch {
          /* no framebuffer export (stale build) — leave canvas blank */
        }
        setStatus("ready");
      })
      .catch(() => {
        if (cancelled) return;
        dbgRef.current = null;
        setDbg(null);
        setStatus("error");
      });
    return () => {
      cancelled = true;
    };
  }, [loadModule]);

  const refresh = useCallback(() => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    setSnapshot(bridge.getSnapshot());
  }, []);

  const step = useCallback(() => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    bridge.step();
    refresh();
  }, [refresh]);

  const run = useCallback(() => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    bridge.run();
    audioRef.current.resume();  // user-gesture-initiated; unlocks WebAudio
    addToast("Execution started", "info");
    startLoop();
  }, [addToast, startLoop]);

  const stopMovie = useCallback(() => {
    if (movieRafRef.current !== null) {
      cancelAnimationFrame(movieRafRef.current);
      movieRafRef.current = null;
    }
    setMovie((m) => (m.playing ? { ...m, playing: false } : m));
  }, []);

  const disconnectLiveAgent = useCallback(() => {
    if (liveSourceRef.current) {
      liveSourceRef.current.close();
      liveSourceRef.current = null;
    }
    setLiveAgent((s) => (s.connected ? { connected: false, frame: 0 } : s));
  }, []);

  const stop = useCallback(() => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    stopMovie();
    disconnectLiveAgent();
    stopLoop();
    bridge.stop();
    audioRef.current.suspend();
    refresh();
  }, [refresh, stopLoop, stopMovie, disconnectLiveAgent]);

  const reset = useCallback(() => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    stopMovie();
    stopLoop();
    bridge.reset();
    refresh();
  }, [refresh, stopLoop, stopMovie]);

  // Replay a self-contained .nesmovie (ROM + one controller byte per frame).
  // Because the core is deterministic, this reproduces exactly what the agent
  // (or any recorder) did when the movie was captured.
  const playMovie = useCallback(
    (bytes: Uint8Array) => {
      const bridge = dbgRef.current;
      if (!bridge) return;
      stopLoop();
      if (movieRafRef.current !== null) {
        cancelAnimationFrame(movieRafRef.current);
        movieRafRef.current = null;
      }
      let parsed;
      try {
        parsed = parseMovie(bytes);
      } catch (e) {
        addToast(`Invalid movie: ${(e as Error).message}`, "danger");
        return;
      }
      if (bridge.loadRom(parsed.rom) !== 0) {
        addToast("Movie ROM was rejected", "danger");
        return;
      }
      audioRef.current.resume(); // user-gesture-initiated; unlocks WebAudio
      const inputs = parsed.inputs;
      setMovie({ playing: true, frame: 0, total: inputs.length });
      addToast(`Playing movie — ${inputs.length} frames`, "info");

      let i = 0;
      const tick = () => {
        const b = dbgRef.current;
        if (!b) {
          movieRafRef.current = null;
          setMovie((m) => ({ ...m, playing: false }));
          return;
        }
        if (i >= inputs.length) {
          movieRafRef.current = null;
          setMovie((m) => ({ ...m, playing: false }));
          setSnapshot(b.getSnapshot());
          addToast("Movie finished", "info");
          return;
        }
        b.setController(inputs[i]);
        b.runFrame();
        setFramebuffer(new Uint8ClampedArray(b.getFramebuffer()));
        const samples = b.audioDrain(4096);
        if (samples.length) audioRef.current.pump(samples);
        i += 1;
        if (i % 6 === 0) setMovie((m) => ({ ...m, frame: i }));
        movieRafRef.current = requestAnimationFrame(tick);
      };
      movieRafRef.current = requestAnimationFrame(tick);
    },
    [addToast, stopLoop],
  );

  // Watch a live agent: connect to the SSE stream and re-simulate the streamed
  // actions on the local core. Because the core is deterministic, the browser
  // reproduces exactly what the agent is doing on the server, frame for frame.
  const connectLiveAgent = useCallback(
    (url: string = LIVE_AGENT_URL) => {
      const bridge = dbgRef.current;
      if (!bridge) return;
      stopLoop();
      stopMovie();
      disconnectLiveAgent();
      audioRef.current.resume();

      let frames = 0;
      const es = new EventSource(url);
      liveSourceRef.current = es;

      es.addEventListener("rom", (ev) => {
        const rom = base64ToBytes((ev as MessageEvent).data);
        liveRomRef.current = rom;
        bridge.loadRom(rom);
      });
      es.addEventListener("reset", () => {
        if (liveRomRef.current) bridge.loadRom(liveRomRef.current); // re-boot
      });
      es.addEventListener("step", (ev) => {
        const mask = parseInt((ev as MessageEvent).data, 10) || 0;
        bridge.setController(mask);
        bridge.runFrame();
        setFramebuffer(new Uint8ClampedArray(bridge.getFramebuffer()));
        const samples = bridge.audioDrain(4096);
        if (samples.length) audioRef.current.pump(samples);
        frames += 1;
        if (frames % 6 === 0) setLiveAgent({ connected: true, frame: frames });
      });
      es.onopen = () => {
        setLiveAgent({ connected: true, frame: 0 });
        addToast("Live agent connected", "info");
      };
      es.onerror = () => {
        addToast("Live agent not reachable (is `python -m nesenv.live` running?)", "danger");
        disconnectLiveAgent();
      };
    },
    [addToast, stopLoop, stopMovie, disconnectLiveAgent],
  );

  // BRK (opcode 0x00) handling: the C++ core dispatches `nes-brk-encountered`
  // when a BRK is stepped. Auto-stop the loop and surface a toast. (§6, preserve.)
  useEffect(() => {
    const onBrk = () => {
      stopLoop();
      const bridge = dbgRef.current;
      if (bridge) {
        bridge.stop();
        setSnapshot(bridge.getSnapshot());
      }
      addToast("Program terminated with BRK", "info");
    };
    window.addEventListener("nes-brk-encountered", onBrk);
    return () => window.removeEventListener("nes-brk-encountered", onBrk);
  }, [addToast, stopLoop]);

  const addBreakpoint = useCallback((addr: number) => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    bridge.addBreakpoint(addr);
    setBreakpoints((prev) =>
      prev.includes(addr) ? prev : [...prev, addr].sort((a, b) => a - b),
    );
  }, []);

  const removeBreakpoint = useCallback((addr: number) => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    bridge.removeBreakpoint(addr);
    setBreakpoints((prev) => prev.filter((b) => b !== addr));
  }, []);

  const toggleBreakpoint = useCallback(
    (addr: number) => {
      const bridge = dbgRef.current;
      if (!bridge) return;
      // Read the current set synchronously from the ref, perform bridge
      // side-effects + the toast here, and hand a pure value to setState.
      const current = breakpointsRef.current;
      if (current.includes(addr)) {
        bridge.removeBreakpoint(addr);
        setBreakpoints((prev) => prev.filter((b) => b !== addr));
      } else {
        bridge.addBreakpoint(addr);
        setBreakpoints((prev) =>
          prev.includes(addr) ? prev : [...prev, addr].sort((a, b) => a - b),
        );
      }
      addToast(
        `Breakpoint toggled at $${addr.toString(16).toUpperCase().padStart(4, "0")}`,
        "info",
      );
    },
    [addToast],
  );

  const writeMemory = useCallback(
    (addr: number, value: number) => {
      const bridge = dbgRef.current;
      if (!bridge) return;
      bridge.writeMemory(addr, value);
      refresh();
    },
    [refresh],
  );

  const loadROM = useCallback(
    (data: Uint8Array) => {
      const bridge = dbgRef.current;
      if (!bridge) return;
      bridge.loadROM(data);
      refresh();
    },
    [refresh],
  );

  const loadRom = useCallback(
    (data: Uint8Array): number => {
      const bridge = dbgRef.current;
      if (!bridge) return -1;
      stopLoop();
      const status = bridge.loadRom(data);
      if (status === 0) {
        // Do NOT reset() here: load_rom already boots the cartridge (PC <- reset
        // vector). A second reset() would send PC back to $FFFC and the ROM would
        // never run. Just publish the freshly-rendered framebuffer.
        setFramebuffer(new Uint8ClampedArray(bridge.getFramebuffer()));
      }
      refresh();
      return status;
    },
    [refresh, stopLoop],
  );

  const loadOpcodes = useCallback(
    (text: string) => {
      const bytes = parseOpcodes(text);
      loadROM(Uint8Array.from(bytes));
    },
    [loadROM],
  );

  const setController = useCallback((state: number, port = 0) => {
    dbgRef.current?.setController(state, port);
  }, []);

  const actions = useMemo<EmulatorActions>(
    () => ({
      step,
      run,
      stop,
      reset,
      addBreakpoint,
      removeBreakpoint,
      toggleBreakpoint,
      writeMemory,
      loadROM,
      loadRom,
      loadOpcodes,
      setController,
      playMovie,
      stopMovie,
      connectLiveAgent,
      disconnectLiveAgent,
    }),
    [
      step,
      run,
      stop,
      reset,
      addBreakpoint,
      removeBreakpoint,
      toggleBreakpoint,
      writeMemory,
      loadROM,
      loadRom,
      loadOpcodes,
      setController,
      playMovie,
      stopMovie,
      connectLiveAgent,
      disconnectLiveAgent,
    ],
  );

  const value = useMemo<EmulatorContextValue>(
    () => ({
      status,
      snapshot,
      breakpoints,
      running,
      framebuffer,
      movie,
      liveAgent,
      dbg,
      actions,
    }),
    [status, snapshot, breakpoints, running, framebuffer, movie, liveAgent, dbg, actions],
  );

  return (
    <EmulatorContext.Provider value={value}>
      {props.children}
    </EmulatorContext.Provider>
  );
}

export function useEmulator(): EmulatorContextValue {
  const ctx = useContext(EmulatorContext);
  if (!ctx) {
    throw new Error("useEmulator must be used within an EmulatorProvider");
  }
  return ctx;
}
