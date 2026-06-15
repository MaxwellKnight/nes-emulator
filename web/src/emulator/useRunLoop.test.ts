// web/src/emulator/useRunLoop.test.ts
import { describe, it, expect, beforeEach, afterEach, vi } from "vitest";
import { renderHook, act } from "@testing-library/react";
import {
  useRunLoop,
  TIME_SLICE_MS,
  MAX_INSTR_PER_FRAME,
} from "./useRunLoop";
import type { Debugger, WasmModule } from "../wasm/bridge";
import type { EmulatorSnapshot } from "../wasm/types";

function makeSnapshot(running: boolean): EmulatorSnapshot {
  return {
    registers: { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0, status: 0 },
    flags: { n: false, v: false, u: true, b: false, d: false, i: false, z: false, c: false },
    stats: { instructionCount: 0, cycleCount: 0 },
    running,
  };
}

interface FakeBridge extends Debugger {
  steps: number;
}

function makeFakeBridge(opts?: {
  stopAfter?: number;
}): FakeBridge {
  let running = true;
  let steps = 0;
  const stopAfter = opts?.stopAfter ?? Infinity;
  const bridge = {
    steps: 0,
    step() {
      steps++;
      bridge.steps = steps;
      if (steps >= stopAfter) running = false;
    },
    run() {
      running = true;
    },
    stop() {
      running = false;
    },
    reset() {},
    isRunning() {
      return running;
    },
    addBreakpoint() {},
    removeBreakpoint() {},
    clearBreakpoints() {},
    getRegisters() {
      return { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0, status: 0 };
    },
    getFlags() {
      return { n: false, v: false, u: true, b: false, d: false, i: false, z: false, c: false };
    },
    getStats() {
      return { instructionCount: steps, cycleCount: steps * 2 };
    },
    getSnapshot() {
      return makeSnapshot(running);
    },
    readMemory() {
      return 0;
    },
    writeMemory() {},
    readMemoryRange() {
      return [];
    },
    setPC() {},
    disassembleAroundPC() {
      return [];
    },
    disassembleRange() {
      return [];
    },
    loadROM() {},
  } as unknown as FakeBridge;
  return bridge;
}

// Controllable rAF + performance.now harness.
let rafCallbacks: Array<{ id: number; cb: FrameRequestCallback }> = [];
let rafId = 0;
let now = 0;

function flushFrame(advanceBy = 0) {
  const pending = rafCallbacks;
  rafCallbacks = [];
  now += advanceBy;
  for (const { cb } of pending) {
    cb(now);
  }
}

describe("useRunLoop", () => {
  beforeEach(() => {
    rafCallbacks = [];
    rafId = 0;
    now = 0;
    vi.stubGlobal("requestAnimationFrame", (cb: FrameRequestCallback) => {
      const id = ++rafId;
      rafCallbacks.push({ id, cb });
      return id;
    });
    vi.stubGlobal("cancelAnimationFrame", (id: number) => {
      rafCallbacks = rafCallbacks.filter((r) => r.id !== id);
    });
    vi.stubGlobal("performance", { now: () => now });
  });

  afterEach(() => {
    vi.unstubAllGlobals();
  });

  it("exposes the timing constants", () => {
    expect(TIME_SLICE_MS).toBe(10);
    expect(MAX_INSTR_PER_FRAME).toBe(1000);
  });

  it("is not running before start", () => {
    const dbg = makeFakeBridge();
    const { result } = renderHook(() =>
      useRunLoop({ dbg, onSnapshot: () => {}, onBreak: () => {} }),
    );
    expect(result.current.running).toBe(false);
  });

  it("steps while isRunning within the slice and publishes a snapshot", () => {
    const dbg = makeFakeBridge();
    const onSnapshot = vi.fn();
    const { result } = renderHook(() =>
      useRunLoop({ dbg, onSnapshot, onBreak: () => {} }),
    );
    act(() => {
      result.current.start();
    });
    expect(result.current.running).toBe(true);
    // First frame: performance.now stays 0 inside the loop, so it caps at MAX_INSTR_PER_FRAME.
    act(() => {
      flushFrame();
    });
    expect(dbg.steps).toBe(MAX_INSTR_PER_FRAME);
    expect(onSnapshot).toHaveBeenCalled();
    const last = onSnapshot.mock.calls.at(-1)?.[0] as EmulatorSnapshot;
    expect(last.running).toBe(true);
  });

  it("stops and fires onBreak when the bridge flips isRunning false mid-run", () => {
    const dbg = makeFakeBridge({ stopAfter: 3 });
    const onBreak = vi.fn();
    const onSnapshot = vi.fn();
    const { result } = renderHook(() =>
      useRunLoop({ dbg, onSnapshot, onBreak }),
    );
    act(() => {
      result.current.start();
    });
    act(() => {
      flushFrame();
    });
    expect(dbg.steps).toBe(3);
    expect(onBreak).toHaveBeenCalledTimes(1);
    expect(result.current.running).toBe(false);
    // No further frames are scheduled after the break.
    expect(rafCallbacks).toHaveLength(0);
  });

  it("stop() halts the loop and prevents further stepping", () => {
    const dbg = makeFakeBridge();
    const { result } = renderHook(() =>
      useRunLoop({ dbg, onSnapshot: () => {}, onBreak: () => {} }),
    );
    act(() => {
      result.current.start();
    });
    act(() => {
      result.current.stop();
    });
    expect(result.current.running).toBe(false);
    const before = dbg.steps;
    act(() => {
      flushFrame();
    });
    expect(dbg.steps).toBe(before);
  });

  it("does nothing when dbg is null", () => {
    const { result } = renderHook(() =>
      useRunLoop({ dbg: null, onSnapshot: () => {}, onBreak: () => {} }),
    );
    act(() => {
      result.current.start();
    });
    expect(result.current.running).toBe(false);
    expect(rafCallbacks).toHaveLength(0);
  });

  it("stops stepping at the time slice when frames advance the clock", () => {
    const dbg = makeFakeBridge();
    const { result } = renderHook(() =>
      useRunLoop({ dbg, onSnapshot: () => {}, onBreak: () => {} }),
    );
    // performance.now advances past the slice on the very first read inside the loop.
    let calls = 0;
    vi.stubGlobal("performance", {
      now: () => {
        calls++;
        // First read (loop start baseline) is 0; subsequent reads exceed the slice.
        return calls <= 1 ? 0 : TIME_SLICE_MS + 1;
      },
    });
    act(() => {
      result.current.start();
    });
    act(() => {
      flushFrame();
    });
    // Loop checks elapsed before each step; with elapsed > slice after baseline it stops quickly.
    expect(dbg.steps).toBeLessThan(MAX_INSTR_PER_FRAME);
    expect(dbg.steps).toBeGreaterThan(0);
  });
});

// Silence unused import lint in environments that flag it.
void (null as unknown as WasmModule);
