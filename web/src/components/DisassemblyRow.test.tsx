// web/src/components/DisassemblyRow.test.tsx
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { describe, it, expect, vi } from "vitest";
import { DisassemblyRow } from "./DisassemblyRow";
import type { DisassembledInstruction } from "../wasm/types";

const ldxImm: DisassembledInstruction = {
  address: 0x0c00,
  opcode: 0xa2,
  mnemonic: "LDX",
  operand: 0x05,
  formatted: "LDX #$05",
  bytes: 2,
  cycles: 2,
};

describe("DisassemblyRow", () => {
  it("renders address, opcode bytes, mnemonic and operand", () => {
    render(
      <DisassemblyRow
        instr={ldxImm}
        isCurrent={false}
        hasBreakpoint={false}
        onToggle={() => {}}
      />,
    );
    const row = screen.getByTestId("disasm-row-0x0c00");
    // Address rendered bare-hex (the $ prefix lives in titles/tooltips now).
    expect(row).toHaveTextContent("0C00");
    expect(row).toHaveTextContent("A2 05");
    expect(row).toHaveTextContent("LDX");
    expect(row).toHaveTextContent("#$05");
  });

  it("colors an immediate operand with the immediate kind class", () => {
    render(
      <DisassemblyRow
        instr={ldxImm}
        isCurrent={false}
        hasBreakpoint={false}
        onToggle={() => {}}
      />,
    );
    const operand = screen.getByTestId("disasm-operand-0x0c00");
    expect(operand.getAttribute("data-kind")).toBe("immediate");
  });

  it("marks the current row and breakpoint row", () => {
    render(
      <DisassemblyRow
        instr={ldxImm}
        isCurrent={true}
        hasBreakpoint={true}
        onToggle={() => {}}
      />,
    );
    const row = screen.getByTestId("disasm-row-0x0c00");
    expect(row.getAttribute("data-current")).toBe("true");
    expect(row.getAttribute("data-breakpoint")).toBe("true");
  });

  it("calls onToggle with the address when clicked", async () => {
    const user = userEvent.setup();
    const onToggle = vi.fn();
    render(
      <DisassemblyRow
        instr={ldxImm}
        isCurrent={false}
        hasBreakpoint={false}
        onToggle={onToggle}
      />,
    );
    await user.click(screen.getByTestId("disasm-row-0x0c00"));
    expect(onToggle).toHaveBeenCalledWith(0x0c00);
  });

  it("does not call onToggle when disabled", async () => {
    const user = userEvent.setup();
    const onToggle = vi.fn();
    render(
      <DisassemblyRow
        instr={ldxImm}
        isCurrent={false}
        hasBreakpoint={false}
        disabled
        onToggle={onToggle}
      />,
    );
    await user.click(screen.getByTestId("disasm-row-0x0c00"));
    expect(onToggle).not.toHaveBeenCalled();
  });
});
