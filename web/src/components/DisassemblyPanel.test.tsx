// web/src/components/DisassemblyPanel.test.tsx
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { describe, it, expect, vi, beforeEach } from "vitest";
import { DisassemblyPanel } from "./DisassemblyPanel";
import type { EmulatorContextValue } from "../emulator/EmulatorProvider";
import type { DisassembledInstruction } from "../wasm/types";

let mockInstrs: DisassembledInstruction[];
vi.mock("../emulator/useDisassembly", () => ({
  useDisassembly: () => mockInstrs,
}));

vi.mock("../emulator/EmulatorProvider", async (orig) => {
  const actual = await orig<typeof import("../emulator/EmulatorProvider")>();
  return { ...actual, useEmulator: () => mockCtx };
});

let mockCtx: EmulatorContextValue;
const toggleBreakpoint = vi.fn();

function instr(address: number, mnemonic: string): DisassembledInstruction {
  return { address, opcode: 0xea, mnemonic, operand: 0, formatted: mnemonic, bytes: 1, cycles: 2 };
}

function makeCtx(pc: number, breakpoints: number[]): EmulatorContextValue {
  return {
    status: "ready",
    snapshot: {
      registers: { a: 0, x: 0, y: 0, sp: 0xfd, pc, status: 0x24 },
      flags: { n: false, v: false, u: true, b: false, d: false, i: true, z: false, c: false },
      stats: { instructionCount: 0, cycleCount: 0 },
      running: false,
    },
    breakpoints,
    running: false,
    framebuffer: null,
    movie: { playing: false, frame: 0, total: 0 },
    liveAgent: { connected: false, frame: 0 },
    dbg: null,
    actions: { toggleBreakpoint } as unknown as EmulatorContextValue["actions"],
  };
}

beforeEach(() => {
  toggleBreakpoint.mockClear();
});

describe("DisassemblyPanel", () => {
  it("renders one row per instruction, marking the current PC and breakpoints", () => {
    mockInstrs = [instr(0x0c00, "NOP"), instr(0x0c01, "NOP"), instr(0x0c02, "NOP")];
    mockCtx = makeCtx(0x0c01, [0x0c02]);
    render(<DisassemblyPanel />);
    expect(screen.getAllByTestId(/^disasm-row-/)).toHaveLength(3);
    expect(screen.getByTestId("disasm-row-0x0c01").getAttribute("data-current")).toBe("true");
    expect(screen.getByTestId("disasm-row-0x0c00").getAttribute("data-current")).toBe("false");
    expect(screen.getByTestId("disasm-row-0x0c02").getAttribute("data-breakpoint")).toBe("true");
  });

  it("calls actions.toggleBreakpoint when a row is clicked", async () => {
    const user = userEvent.setup();
    mockInstrs = [instr(0x0c00, "NOP")];
    mockCtx = makeCtx(0x0c00, []);
    render(<DisassemblyPanel />);
    await user.click(screen.getByTestId("disasm-row-0x0c00"));
    expect(toggleBreakpoint).toHaveBeenCalledWith(0x0c00);
  });

  it("shows an empty state when there are no instructions", () => {
    mockInstrs = [];
    mockCtx = makeCtx(0x0c00, []);
    render(<DisassemblyPanel />);
    expect(screen.getByText("No disassembly available")).toBeInTheDocument();
  });
});
