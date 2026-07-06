// web/src/emulator/useFrameLoop.ts
import { useCallback, useEffect, useRef, useState } from "react";
import type { Debugger } from "../wasm/bridge";
import type { EmulatorSnapshot } from "../wasm/types";

// run_frame reason codes (mirror the C++ Debugger::run_frame contract).
export const RUN_FRAME_OK = 0;
export const RUN_FRAME_BREAKPOINT = 1;
export const RUN_FRAME_BRK = 2;

// Throttle the (relatively expensive) debugger snapshot so the registers /
// disassembly panels update ~10 Hz while the canvas blits every frame.
export const SNAPSHOT_INTERVAL_MS = 100;

export function useFrameLoop(args: {
  dbg: Debugger | null;
  onFrame: (framebuffer: Uint8ClampedArray) => void;
  onSnapshot: (s: EmulatorSnapshot) => void;
  onBreak: () => void;
  onBrk: () => void;
}): { start(): void; stop(): void; running: boolean } {
  const { dbg, onFrame, onSnapshot, onBreak, onBrk } = args;
  const [running, setRunning] = useState(false);
  const rafRef = useRef<number | null>(null);
  const lastSnapshotRef = useRef(-Infinity);

  // Keep callbacks/bridge in refs so the frame closure sees the latest values.
  const onFrameRef = useRef(onFrame);
  const onSnapshotRef = useRef(onSnapshot);
  const onBreakRef = useRef(onBreak);
  const onBrkRef = useRef(onBrk);
  const dbgRef = useRef(dbg);
  onFrameRef.current = onFrame;
  onSnapshotRef.current = onSnapshot;
  onBreakRef.current = onBreak;
  onBrkRef.current = onBrk;
  dbgRef.current = dbg;

  const cancel = useCallback(() => {
    if (rafRef.current !== null) {
      cancelAnimationFrame(rafRef.current);
      rafRef.current = null;
    }
  }, []);

  const stop = useCallback(() => {
    cancel();
    setRunning(false);
  }, [cancel]);

  const frame = useCallback(() => {
    const bridge = dbgRef.current;
    if (!bridge) {
      cancel();
      setRunning(false);
      return;
    }

    const reason = bridge.runFrame();
    onFrameRef.current(bridge.getFramebuffer());

    const t = performance.now();
    if (t - lastSnapshotRef.current >= SNAPSHOT_INTERVAL_MS) {
      lastSnapshotRef.current = t;
      onSnapshotRef.current(bridge.getSnapshot());
    }

    if (reason !== RUN_FRAME_OK) {
      cancel();
      setRunning(false);
      // Final snapshot so the panels reflect the halt point.
      onSnapshotRef.current(bridge.getSnapshot());
      if (reason === RUN_FRAME_BRK) onBrkRef.current();
      else onBreakRef.current();
      return;
    }

    rafRef.current = requestAnimationFrame(frame);
  }, [cancel]);

  const start = useCallback(() => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    if (rafRef.current !== null) return;
    lastSnapshotRef.current = -Infinity;
    setRunning(true);
    rafRef.current = requestAnimationFrame(frame);
  }, [frame]);

  useEffect(() => cancel, [cancel]);

  return { start, stop, running };
}
