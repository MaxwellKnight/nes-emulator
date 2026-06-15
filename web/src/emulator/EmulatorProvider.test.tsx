// web/src/emulator/EmulatorProvider.test.tsx
import { describe, it, expect, vi, beforeEach, afterEach } from "vitest";
import { renderHook, act, waitFor } from "@testing-library/react";
import type { ReactNode } from "react";
import { EmulatorProvider, useEmulator } from "./EmulatorProvider";
import { ToastProvider, useToast } from "../components/toast/ToastProvider";
import { createMockModule } from "../wasm/testing/mockModule";
import type { WasmModule } from "../wasm/bridge";

function makeWrapper(loadModule: () => Promise<WasmModule>) {
  return function Wrapper({ children }: { children: ReactNode }) {
    return (
      <ToastProvider>
        <EmulatorProvider loadModule={loadModule}>{children}</EmulatorProvider>
      </ToastProvider>
    );
  };
}

async function renderReady(opts?: { disassembly?: string }) {
  const mock = createMockModule(opts);
  const loadModule = () => Promise.resolve(mock);
  const utils = renderHook(() => useEmulator(), {
    wrapper: makeWrapper(loadModule),
  });
  await waitFor(() => expect(utils.result.current.status).toBe("ready"));
  return { ...utils, mock };
}

describe("EmulatorProvider", () => {
  it("starts loading then transitions to ready", async () => {
    const mock = createMockModule();
    const utils = renderHook(() => useEmulator(), {
      wrapper: makeWrapper(() => Promise.resolve(mock)),
    });
    expect(utils.result.current.status).toBe("loading");
    await waitFor(() => expect(utils.result.current.status).toBe("ready"));
    expect(utils.result.current.dbg).not.toBeNull();
  });

  it("transitions to error when loadModule rejects", async () => {
    const utils = renderHook(() => useEmulator(), {
      wrapper: makeWrapper(() => Promise.reject(new Error("boom"))),
    });
    await waitFor(() => expect(utils.result.current.status).toBe("error"));
    expect(utils.result.current.dbg).toBeNull();
  });

  it("publishes a snapshot once ready", async () => {
    const { result } = await renderReady();
    expect(result.current.snapshot).not.toBeNull();
    expect(result.current.snapshot?.registers.sp).toBe(0xfd);
  });

  it("step delegates to the bridge and refreshes the snapshot", async () => {
    const { result, mock } = await renderReady();
    mock._state.instructionCount = 0;
    act(() => {
      result.current.actions.step();
    });
    await waitFor(() =>
      expect(result.current.snapshot?.stats.instructionCount).toBe(1),
    );
    expect(mock._state.instructionCount).toBe(1);
  });

  it("addBreakpoint keeps breakpoints sorted ascending", async () => {
    const { result } = await renderReady();
    act(() => {
      result.current.actions.addBreakpoint(0x20);
      result.current.actions.addBreakpoint(0x05);
      result.current.actions.addBreakpoint(0x10);
    });
    await waitFor(() =>
      expect(result.current.breakpoints).toEqual([0x05, 0x10, 0x20]),
    );
  });

  it("removeBreakpoint removes a single address", async () => {
    const { result } = await renderReady();
    act(() => {
      result.current.actions.addBreakpoint(0x10);
      result.current.actions.addBreakpoint(0x20);
      result.current.actions.removeBreakpoint(0x10);
    });
    await waitFor(() => expect(result.current.breakpoints).toEqual([0x20]));
  });

  it("toggleBreakpoint adds when absent and removes when present", async () => {
    const { result } = await renderReady();
    act(() => {
      result.current.actions.toggleBreakpoint(0x42);
    });
    await waitFor(() => expect(result.current.breakpoints).toEqual([0x42]));
    act(() => {
      result.current.actions.toggleBreakpoint(0x42);
    });
    await waitFor(() => expect(result.current.breakpoints).toEqual([]));
  });

  it("writeMemory delegates to the bridge and refreshes the snapshot", async () => {
    const { result, mock } = await renderReady();
    act(() => {
      result.current.actions.writeMemory(0x0200, 0xab);
    });
    await waitFor(() => expect(mock._state.memory[0x0200]).toBe(0xab));
  });

  it("loadOpcodes parses hex then loads via loadROM", async () => {
    const { result, mock } = await renderReady();
    act(() => {
      result.current.actions.loadOpcodes("A9 01 8D 00 02");
    });
    await waitFor(() => {
      expect(mock._state.memory[0x0c00]).toBe(0xa9);
      expect(mock._state.memory[0x0c01]).toBe(0x01);
      expect(mock._state.memory[0x0c02]).toBe(0x8d);
    });
    expect(mock._state.registers.pc).toBe(0x0c00);
  });

  it("reset delegates to the bridge and refreshes the snapshot", async () => {
    const { result, mock } = await renderReady();
    mock._state.instructionCount = 9;
    act(() => {
      result.current.actions.reset();
    });
    await waitFor(() =>
      expect(result.current.snapshot?.stats.instructionCount).toBe(0),
    );
  });

  it("throws when useEmulator is used outside the provider", () => {
    expect(() => renderHook(() => useEmulator())).toThrow();
  });
});

// --- Run-loop / toast integration (controllable rAF harness) ---

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

function comboWrapper(loadModule: () => Promise<WasmModule>) {
  return function Wrapper({ children }: { children: ReactNode }) {
    return (
      <ToastProvider>
        <EmulatorProvider loadModule={loadModule}>{children}</EmulatorProvider>
      </ToastProvider>
    );
  };
}

async function renderWithToast(opts?: { disassembly?: string }) {
  const mock = createMockModule(opts);
  const loadModule = () => Promise.resolve(mock);
  const utils = renderHook(
    () => ({ emu: useEmulator(), toast: useToast() }),
    { wrapper: comboWrapper(loadModule) },
  );
  await waitFor(() => expect(utils.result.current.emu.status).toBe("ready"));
  return { ...utils, mock };
}

describe("EmulatorProvider run loop", () => {
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

  it("run() drives continuous stepping via the rAF loop and shows an Execution started toast", async () => {
    const { result, mock } = await renderWithToast();
    expect(mock._state.instructionCount).toBe(0);

    act(() => {
      result.current.emu.actions.run();
    });

    expect(result.current.emu.running).toBe(true);
    expect(
      result.current.toast.toasts.some(
        (t) => t.message === "Execution started" && t.type === "info",
      ),
    ).toBe(true);

    // Advance one frame past the time slice so the loop steps then yields.
    act(() => {
      flushFrame(20);
    });
    expect(mock._state.instructionCount).toBeGreaterThan(0);
  });

  it("auto-stops and shows a Breakpoint hit toast when isRunning flips false mid-run", async () => {
    const { result, mock } = await renderWithToast();

    act(() => {
      result.current.emu.actions.run();
    });
    // Simulate the C++ core hitting a breakpoint: it clears the running flag.
    act(() => {
      mock._state.running = false;
      flushFrame(20);
    });

    expect(result.current.emu.running).toBe(false);
    expect(
      result.current.toast.toasts.some(
        (t) => t.message === "Breakpoint hit" && t.type === "warning",
      ),
    ).toBe(true);
  });

  it("auto-stops and shows a BRK toast on the nes-brk-encountered event", async () => {
    const { result } = await renderWithToast();

    act(() => {
      result.current.emu.actions.run();
    });
    expect(result.current.emu.running).toBe(true);

    act(() => {
      window.dispatchEvent(new CustomEvent("nes-brk-encountered"));
    });

    expect(result.current.emu.running).toBe(false);
    expect(
      result.current.toast.toasts.some(
        (t) =>
          t.message === "Program terminated with BRK" && t.type === "info",
      ),
    ).toBe(true);
  });

  it("reset() stops any running loop", async () => {
    const { result } = await renderWithToast();

    act(() => {
      result.current.emu.actions.run();
    });
    expect(result.current.emu.running).toBe(true);

    act(() => {
      result.current.emu.actions.reset();
    });
    expect(result.current.emu.running).toBe(false);
    expect(rafCallbacks).toHaveLength(0);
  });
});
