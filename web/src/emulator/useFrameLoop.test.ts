import { describe, it, expect, beforeEach, afterEach, vi } from "vitest";
import { renderHook, act } from "@testing-library/react";
import {
  useFrameLoop,
  SNAPSHOT_INTERVAL_MS,
  RUN_FRAME_OK,
  RUN_FRAME_BREAKPOINT,
  RUN_FRAME_BRK,
} from "./useFrameLoop";
import type { Debugger } from "../wasm/bridge";

interface FakeBridge extends Debugger {
  frames: number;
}

function makeFakeBridge(opts?: {
  reasonAfter?: number;
  reason?: number;
}): FakeBridge {
  let frames = 0;
  const reasonAfter = opts?.reasonAfter ?? Infinity;
  const reason = opts?.reason ?? RUN_FRAME_OK;
  const bridge = {
    frames: 0,
    runFrame() {
      frames += 1;
      bridge.frames = frames;
      return frames >= reasonAfter ? reason : RUN_FRAME_OK;
    },
    frameCount() {
      return frames;
    },
    getFramebuffer() {
      return new Uint8ClampedArray(256 * 240 * 4);
    },
    getSnapshot() {
      return {
        registers: { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0, status: 0 },
        flags: {
          n: false,
          v: false,
          u: true,
          b: false,
          d: false,
          i: false,
          z: false,
          c: false,
        },
        stats: { instructionCount: frames, cycleCount: frames },
        running: true,
      };
    },
    step() {},
    run() {},
    stop() {},
    reset() {},
    isRunning() {
      return true;
    },
    addBreakpoint() {},
    removeBreakpoint() {},
    clearBreakpoints() {},
    getRegisters() {
      return { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0, status: 0 };
    },
    getFlags() {
      return {
        n: false,
        v: false,
        u: true,
        b: false,
        d: false,
        i: false,
        z: false,
        c: false,
      };
    },
    getStats() {
      return { instructionCount: frames, cycleCount: frames };
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
    loadRom() {
      return 0;
    },
    renderPatternTable() {
      return new Uint8ClampedArray(128 * 128 * 4);
    },
    getNametable() {
      return new Uint8Array(2048);
    },
    getPaletteRam() {
      return new Uint8Array(32);
    },
    ppuState() {
      return { ctrl: 0, mask: 0, status: 0, scanline: 0 };
    },
  } as unknown as FakeBridge;
  return bridge;
}

let rafCallbacks: Array<{ id: number; cb: FrameRequestCallback }> = [];
let rafId = 0;
let now = 0;

function flushFrame(advanceBy = 0) {
  const pending = rafCallbacks;
  rafCallbacks = [];
  now += advanceBy;
  for (const { cb } of pending) cb(now);
}

describe("useFrameLoop", () => {
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

  it("exposes its constants", () => {
    expect(RUN_FRAME_OK).toBe(0);
    expect(RUN_FRAME_BREAKPOINT).toBe(1);
    expect(RUN_FRAME_BRK).toBe(2);
    expect(SNAPSHOT_INTERVAL_MS).toBeGreaterThan(0);
  });

  it("is not running before start", () => {
    const dbg = makeFakeBridge();
    const { result } = renderHook(() =>
      useFrameLoop({
        dbg,
        onFrame: () => {},
        onSnapshot: () => {},
        onBreak: () => {},
        onBrk: () => {},
      }),
    );
    expect(result.current.running).toBe(false);
  });

  it("calls runFrame and onFrame(framebuffer) each scheduled frame", () => {
    const dbg = makeFakeBridge();
    const onFrame = vi.fn();
    const { result } = renderHook(() =>
      useFrameLoop({
        dbg,
        onFrame,
        onSnapshot: () => {},
        onBreak: () => {},
        onBrk: () => {},
      }),
    );
    act(() => result.current.start());
    expect(result.current.running).toBe(true);
    act(() => flushFrame());
    expect(dbg.frames).toBe(1);
    expect(onFrame).toHaveBeenCalledTimes(1);
    expect(onFrame.mock.calls[0][0]).toBeInstanceOf(Uint8ClampedArray);
    // Another frame is scheduled.
    expect(rafCallbacks.length).toBe(1);
    act(() => flushFrame());
    expect(dbg.frames).toBe(2);
    expect(onFrame).toHaveBeenCalledTimes(2);
  });

  it("snapshots on the first frame then throttles by SNAPSHOT_INTERVAL_MS", () => {
    const dbg = makeFakeBridge();
    const onSnapshot = vi.fn();
    const { result } = renderHook(() =>
      useFrameLoop({
        dbg,
        onFrame: () => {},
        onSnapshot,
        onBreak: () => {},
        onBrk: () => {},
      }),
    );
    act(() => result.current.start());
    act(() => flushFrame()); // t=0 -> first snapshot
    expect(onSnapshot).toHaveBeenCalledTimes(1);
    act(() => flushFrame(1)); // t=1ms -> throttled, no new snapshot
    expect(onSnapshot).toHaveBeenCalledTimes(1);
    act(() => flushFrame(SNAPSHOT_INTERVAL_MS + 1)); // crossed the interval
    expect(onSnapshot).toHaveBeenCalledTimes(2);
  });

  it("stops and fires onBreak on a breakpoint reason", () => {
    const dbg = makeFakeBridge({ reasonAfter: 2, reason: RUN_FRAME_BREAKPOINT });
    const onBreak = vi.fn();
    const onBrk = vi.fn();
    const { result } = renderHook(() =>
      useFrameLoop({
        dbg,
        onFrame: () => {},
        onSnapshot: () => {},
        onBreak,
        onBrk,
      }),
    );
    act(() => result.current.start());
    act(() => flushFrame()); // frame 1, reason 0
    act(() => flushFrame()); // frame 2, reason BREAKPOINT
    expect(onBreak).toHaveBeenCalledTimes(1);
    expect(onBrk).not.toHaveBeenCalled();
    expect(result.current.running).toBe(false);
    expect(rafCallbacks).toHaveLength(0);
  });

  it("stops and fires onBrk on a BRK reason", () => {
    const dbg = makeFakeBridge({ reasonAfter: 1, reason: RUN_FRAME_BRK });
    const onBreak = vi.fn();
    const onBrk = vi.fn();
    const { result } = renderHook(() =>
      useFrameLoop({
        dbg,
        onFrame: () => {},
        onSnapshot: () => {},
        onBreak,
        onBrk,
      }),
    );
    act(() => result.current.start());
    act(() => flushFrame());
    expect(onBrk).toHaveBeenCalledTimes(1);
    expect(onBreak).not.toHaveBeenCalled();
    expect(result.current.running).toBe(false);
  });

  it("stop() halts the loop", () => {
    const dbg = makeFakeBridge();
    const { result } = renderHook(() =>
      useFrameLoop({
        dbg,
        onFrame: () => {},
        onSnapshot: () => {},
        onBreak: () => {},
        onBrk: () => {},
      }),
    );
    act(() => result.current.start());
    act(() => result.current.stop());
    expect(result.current.running).toBe(false);
    const before = dbg.frames;
    act(() => flushFrame());
    expect(dbg.frames).toBe(before);
  });

  it("does nothing when dbg is null", () => {
    const { result } = renderHook(() =>
      useFrameLoop({
        dbg: null,
        onFrame: () => {},
        onSnapshot: () => {},
        onBreak: () => {},
        onBrk: () => {},
      }),
    );
    act(() => result.current.start());
    expect(result.current.running).toBe(false);
    expect(rafCallbacks).toHaveLength(0);
  });
});
