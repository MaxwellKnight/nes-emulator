// web/src/emulator/useRunLoop.ts
import { useCallback, useEffect, useRef, useState } from "react";
import type { Debugger } from "../wasm/bridge";
import type { EmulatorSnapshot } from "../wasm/types";

export const TIME_SLICE_MS = 10;
export const MAX_INSTR_PER_FRAME = 1000;

export function useRunLoop(args: {
  dbg: Debugger | null;
  onSnapshot: (s: EmulatorSnapshot) => void;
  onBreak: () => void;
}): { start(): void; stop(): void; running: boolean } {
  const { dbg, onSnapshot, onBreak } = args;
  const [running, setRunning] = useState(false);
  const rafRef = useRef<number | null>(null);

  // Keep callbacks in refs so the frame closure always sees the latest.
  const onSnapshotRef = useRef(onSnapshot);
  const onBreakRef = useRef(onBreak);
  const dbgRef = useRef(dbg);
  onSnapshotRef.current = onSnapshot;
  onBreakRef.current = onBreak;
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
      stop();
      return;
    }
    const startTime = performance.now();
    let count = 0;
    while (bridge.isRunning() && count < MAX_INSTR_PER_FRAME) {
      bridge.step();
      count++;
      if (performance.now() - startTime >= TIME_SLICE_MS) break;
    }

    onSnapshotRef.current(bridge.getSnapshot());

    if (!bridge.isRunning()) {
      cancel();
      setRunning(false);
      onBreakRef.current();
      return;
    }

    rafRef.current = requestAnimationFrame(frame);
  }, [cancel, stop]);

  const start = useCallback(() => {
    const bridge = dbgRef.current;
    if (!bridge) return;
    if (rafRef.current !== null) return;
    setRunning(true);
    rafRef.current = requestAnimationFrame(frame);
  }, [frame]);

  useEffect(() => cancel, [cancel]);

  return { start, stop, running };
}
