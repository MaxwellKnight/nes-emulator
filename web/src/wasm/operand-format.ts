// web/src/wasm/operand-format.ts
import type { DisassembledInstruction } from "./types";

export type OperandKind =
  | "none"
  | "immediate"
  | "relative"
  | "zeropage"
  | "absolute"
  | "indexed"
  | "indirect";

export interface FormattedOperand {
  text: string;
  kind: OperandKind;
}

const IMMEDIATE_OPCODES = new Set<number>([
  0xa9, 0xa2, 0xa0, 0xc9, 0xe0, 0xc0,
]);

function hex(value: number, width: number): string {
  return value.toString(16).toUpperCase().padStart(width, "0");
}

/**
 * Infer the addressing-mode kind and produce a display string for an operand.
 * Rules (from the design spec §6 / operand-format contract):
 *  - immediate when the opcode is one of the known immediate opcodes
 *  - relative branch when (opcode & 0x1F) === 0x10; target is computed as
 *    address + 2 + signedOffset
 *  - otherwise a 2-byte instruction is zero-page, a 3-byte instruction is
 *    absolute, and a 1-byte instruction has no operand
 */
export function formatOperand(
  instr: DisassembledInstruction
): FormattedOperand {
  const { opcode, operand, bytes, address } = instr;

  if (IMMEDIATE_OPCODES.has(opcode)) {
    return { kind: "immediate", text: `#$${hex(operand & 0xff, 2)}` };
  }

  if ((opcode & 0x1f) === 0x10) {
    const signedOffset = operand < 0x80 ? operand : operand - 0x100;
    const target = (address + 2 + signedOffset) & 0xffff;
    return { kind: "relative", text: `$${hex(target, 4)}` };
  }

  if (bytes >= 3) {
    return { kind: "absolute", text: `$${hex(operand & 0xffff, 4)}` };
  }

  if (bytes === 2) {
    return { kind: "zeropage", text: `$${hex(operand & 0xff, 2)}` };
  }

  return { kind: "none", text: "" };
}
