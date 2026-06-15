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
  // additional immediate-mode arithmetic/logical opcodes
  0x09, 0x29, 0x49, 0x69, 0xe9,
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
 *  - indirect when the formatted operand contains parentheses (e.g. ($20),Y)
 *  - indexed when the formatted operand contains a comma (e.g. $0200,X)
 *  - otherwise a 2-byte instruction is zero-page, a 3-byte instruction is
 *    absolute, and a 1-byte instruction has no operand
 */
export function formatOperand(
  instr: DisassembledInstruction
): FormattedOperand {
  const { opcode, operand, bytes, address, formatted } = instr;

  if (IMMEDIATE_OPCODES.has(opcode)) {
    return { kind: "immediate", text: `#$${hex(operand & 0xff, 2)}` };
  }

  if ((opcode & 0x1f) === 0x10) {
    const signedOffset = operand < 0x80 ? operand : operand - 0x100;
    const target = (address + 2 + signedOffset) & 0xffff;
    return { kind: "relative", text: `$${hex(target, 4)}` };
  }

  // Indexed / indirect addressing modes are inferred from the formatted
  // operand string emitted by the disassembler: a "(" implies an indirect
  // mode (e.g. ($20),Y or ($20,X)), and otherwise a "," implies indexed
  // (e.g. $0200,X). This must run before the byte-length fallbacks.
  const operandText = stripMnemonic(formatted, instr.mnemonic);
  if (operandText.includes("(")) {
    return { kind: "indirect", text: operandText };
  }
  if (operandText.includes(",")) {
    return { kind: "indexed", text: operandText };
  }

  if (bytes >= 3) {
    return { kind: "absolute", text: `$${hex(operand & 0xffff, 4)}` };
  }

  if (bytes === 2) {
    return { kind: "zeropage", text: `$${hex(operand & 0xff, 2)}` };
  }

  return { kind: "none", text: "" };
}

/**
 * Extract just the operand portion of a formatted instruction string by
 * dropping the leading mnemonic (e.g. "LDA $0200,X" -> "$0200,X"). Falls back
 * to the trimmed string when no mnemonic prefix is present.
 */
function stripMnemonic(formatted: string, mnemonic: string): string {
  const trimmed = formatted.trim();
  if (mnemonic && trimmed.toUpperCase().startsWith(mnemonic.toUpperCase())) {
    return trimmed.slice(mnemonic.length).trim();
  }
  return trimmed;
}
