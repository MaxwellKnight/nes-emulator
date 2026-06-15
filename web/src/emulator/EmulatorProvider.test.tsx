// web/src/emulator/EmulatorProvider.test.tsx
import { describe, it, expect } from "vitest";
import { renderHook, act, waitFor } from "@testing-library/react";
import type { ReactNode } from "react";
import { EmulatorProvider, useEmulator } from "./EmulatorProvider";
import { createMockModule } from "../wasm/testing/mockModule";
import type { WasmModule } from "../wasm/bridge";

function makeWrapper(loadModule: () => Promise<WasmModule>) {
  return function Wrapper({ children }: { children: ReactNode }) {
    return (
      <EmulatorProvider loadModule={loadModule}>{children}</EmulatorProvider>
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
