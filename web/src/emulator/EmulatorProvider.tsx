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
import { useToast } from "../components/toast/ToastProvider";

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
}

export interface EmulatorContextValue {
  status: EmulatorStatus;
  snapshot: EmulatorSnapshot | null;
  breakpoints: number[];
  running: boolean;
  framebuffer: Uint8ClampedArray | null;
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

  const handleFrame = useCallback((fb: Uint8ClampedArray) => {
    // Copy out of the WASM heap view so React state holds a stable buffer
    // (the heap view is reused/invalidated by the next frame).
    setFramebuffer(new Uint8ClampedArray(fb));
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
    addToast("Execution started", "info");
    startLoop();
  }, [addToast, startLoop]);

  const stop = useCallback(() => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    stopLoop();
    bridge.stop();
    refresh();
  }, [refresh, stopLoop]);

  const reset = useCallback(() => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    stopLoop();
    bridge.reset();
    refresh();
  }, [refresh, stopLoop]);

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
        bridge.reset();
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
    ],
  );

  const value = useMemo<EmulatorContextValue>(
    () => ({
      status,
      snapshot,
      breakpoints,
      running,
      framebuffer,
      dbg,
      actions,
    }),
    [status, snapshot, breakpoints, running, framebuffer, dbg, actions],
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
