// web/src/wasm/operand-format.test.ts
import { describe, it, expect } from "vitest";
import { formatOperand } from "./operand-format";
import type { DisassembledInstruction } from "./types";

function instr(
  partial: Partial<DisassembledInstruction>
): DisassembledInstruction {
  return {
    address: 0x0c00,
    opcode: 0,
    mnemonic: "NOP",
    operand: 0,
    formatted: "",
    bytes: 1,
    cycles: 2,
    ...partial,
  };
}

describe("formatOperand", () => {
  it("formats an immediate operand (LDA #$0A, opcode 0xA9)", () => {
    const result = formatOperand(
      instr({ opcode: 0xa9, operand: 0x0a, bytes: 2 })
    );
    expect(result.kind).toBe("immediate");
    expect(result.text).toBe("#$0A");
  });

  it("formats LDX #$05 (opcode 0xA2) as immediate", () => {
    const result = formatOperand(
      instr({ opcode: 0xa2, operand: 0x05, bytes: 2 })
    );
    expect(result.kind).toBe("immediate");
    expect(result.text).toBe("#$05");
  });

  it("formats ADC #$10 (opcode 0x69) as immediate", () => {
    const result = formatOperand(
      instr({ opcode: 0x69, operand: 0x10, bytes: 2, formatted: "ADC #$10" })
    );
    expect(result.kind).toBe("immediate");
    expect(result.text).toBe("#$10");
  });

  it("detects an indexed operand from the formatted string ($0200,X)", () => {
    // LDA $0200,X (opcode 0xBD) — absolute,X
    const result = formatOperand(
      instr({
        opcode: 0xbd,
        operand: 0x0200,
        bytes: 3,
        mnemonic: "LDA",
        formatted: "LDA $0200,X",
      })
    );
    expect(result.kind).toBe("indexed");
    expect(result.text).toBe("$0200,X");
  });

  it("detects an indirect operand from the formatted string (($20),Y)", () => {
    // LDA ($20),Y (opcode 0xB1) — indirect indexed
    const result = formatOperand(
      instr({
        opcode: 0xb1,
        operand: 0x20,
        bytes: 2,
        mnemonic: "LDA",
        formatted: "LDA ($20),Y",
      })
    );
    expect(result.kind).toBe("indirect");
    expect(result.text).toBe("($20),Y");
  });

  it("formats a relative branch target ((op & 0x1F) === 0x10)", () => {
    // BNE (0xD0) at $0C00 with offset +4 -> target = 0x0C00 + 2 + 4 = 0x0C06
    const result = formatOperand(
      instr({ address: 0x0c00, opcode: 0xd0, operand: 0x04, bytes: 2 })
    );
    expect(result.kind).toBe("relative");
    expect(result.text).toBe("$0C06");
  });

  it("computes a backward relative branch target with a signed offset", () => {
    // BNE (0xD0) at $0C10 with offset 0xFB (-5) -> 0x0C10 + 2 - 5 = 0x0C0D
    const result = formatOperand(
      instr({ address: 0x0c10, opcode: 0xd0, operand: 0xfb, bytes: 2 })
    );
    expect(result.kind).toBe("relative");
    expect(result.text).toBe("$0C0D");
  });

  it("formats a 2-byte non-immediate operand as zeropage", () => {
    // LDA $44 (opcode 0xA5) zero-page
    const result = formatOperand(
      instr({ opcode: 0xa5, operand: 0x44, bytes: 2 })
    );
    expect(result.kind).toBe("zeropage");
    expect(result.text).toBe("$44");
  });

  it("formats a 3-byte operand as absolute", () => {
    // JMP $C000 (opcode 0x4C)
    const result = formatOperand(
      instr({ opcode: 0x4c, operand: 0xc000, bytes: 3 })
    );
    expect(result.kind).toBe("absolute");
    expect(result.text).toBe("$C000");
  });

  it("formats a 1-byte instruction as none with empty text", () => {
    // NOP (opcode 0xEA)
    const result = formatOperand(instr({ opcode: 0xea, bytes: 1 }));
    expect(result.kind).toBe("none");
    expect(result.text).toBe("");
  });
});
