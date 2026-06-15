// web/src/emulator/useDisassembly.test.tsx
import { describe, it, expect, vi } from "vitest";
import { renderHook, waitFor } from "@testing-library/react";
import type { ReactNode } from "react";
import { EmulatorProvider, useEmulator } from "./EmulatorProvider";
import { useDisassembly } from "./useDisassembly";
import { createMockModule } from "../wasm/testing/mockModule";
import type { Debugger } from "../wasm/bridge";

// address|opcode|mnemonic|operand|formatted|bytes|cycles, joined by "#".
const RAW =
  "3072|169|LDA|1|LDA #$01|2|2#3074|162|LDX|2|LDX #$02|2|2#3076|0|BRK|0|BRK|1|7";

function makeWrapper(mock: ReturnType<typeof createMockModule>) {
  return function Wrapper({ children }: { children: ReactNode }) {
    return (
      <EmulatorProvider loadModule={() => Promise.resolve(mock)}>
        {children}
      </EmulatorProvider>
    );
  };
}

describe("useDisassembly", () => {
  it("parses the disassembly string returned by the module", async () => {
    const mock = createMockModule({ disassembly: RAW });
    const { result } = renderHook(
      () => {
        const emu = useEmulator();
        const instrs = useDisassembly();
        return { emu, instrs };
      },
      { wrapper: makeWrapper(mock) },
    );
    await waitFor(() => expect(result.current.emu.status).toBe("ready"));
    expect(result.current.instrs).toHaveLength(3);
    expect(result.current.instrs[0].mnemonic).toBe("LDA");
    expect(result.current.instrs[0].opcode).toBe(169);
    expect(result.current.instrs[0].address).toBe(3072);
    expect(result.current.instrs[1].opcode).toBe(162);
    expect(result.current.instrs[1].formatted).toBe("LDX #$02");
  });

  it("defaults to before=5 after=30 on disassembleAroundPC", async () => {
    const mock = createMockModule({ disassembly: RAW });
    const spy = vi.fn<Debugger["disassembleAroundPC"]>(() => []);
    let captured: Debugger | null = null;
    const { result } = renderHook(
      () => {
        const emu = useEmulator();
        // Patch the bridge's disassembleAroundPC so we can record args.
        if (emu.dbg && emu.dbg !== captured) {
          captured = emu.dbg;
          emu.dbg.disassembleAroundPC = spy;
        }
        const instrs = useDisassembly();
        return { emu, instrs };
      },
      { wrapper: makeWrapper(mock) },
    );
    await waitFor(() => expect(result.current.emu.status).toBe("ready"));
    await waitFor(() => expect(spy).toHaveBeenCalled());
    expect(spy).toHaveBeenCalledWith(5, 30);
  });

  it("forwards explicit before/after arguments", async () => {
    const mock = createMockModule({ disassembly: RAW });
    const spy = vi.fn<Debugger["disassembleAroundPC"]>(() => []);
    let captured: Debugger | null = null;
    const { result } = renderHook(
      () => {
        const emu = useEmulator();
        if (emu.dbg && emu.dbg !== captured) {
          captured = emu.dbg;
          emu.dbg.disassembleAroundPC = spy;
        }
        const instrs = useDisassembly(2, 8);
        return { emu, instrs };
      },
      { wrapper: makeWrapper(mock) },
    );
    await waitFor(() => expect(result.current.emu.status).toBe("ready"));
    await waitFor(() => expect(spy).toHaveBeenCalled());
    expect(spy).toHaveBeenCalledWith(2, 8);
  });

  it("returns an empty array before the module is ready", () => {
    const mock = createMockModule({ disassembly: RAW });
    let resolveLoad: (m: typeof mock) => void = () => {};
    const loadModule = () =>
      new Promise<typeof mock>((res) => {
        resolveLoad = res;
      });
    const { result } = renderHook(
      () => {
        const emu = useEmulator();
        const instrs = useDisassembly();
        return { emu, instrs };
      },
      {
        wrapper: function Wrapper({ children }: { children: ReactNode }) {
          return (
            <EmulatorProvider loadModule={loadModule}>
              {children}
            </EmulatorProvider>
          );
        },
      },
    );
    expect(result.current.emu.status).toBe("loading");
    expect(result.current.instrs).toEqual([]);
    // resolve to avoid an unhandled pending promise warning
    resolveLoad(mock);
  });
});
