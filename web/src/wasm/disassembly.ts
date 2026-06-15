// web/src/wasm/disassembly.ts
import type { DisassembledInstruction } from "./types";

/**
 * Parse the raw disassembly string produced by the C++ debugger.
 * Instructions are separated by "#"; fields within an instruction are
 * separated by "|" in the fixed order:
 *   address|opcode|mnemonic|operand|formatted|bytes|cycles
 * A trailing "#" may be present. When an entry's address is missing or 0,
 * it is backfilled as previous.address + previous.bytes.
 */
export function parseDisassembly(raw: string): DisassembledInstruction[] {
  if (!raw) {
    return [];
  }

  const entries = raw.split(/#(?=\d|$)/);
  const result: DisassembledInstruction[] = [];

  for (const entry of entries) {
    if (entry.trim() === "") {
      continue;
    }

    const fields = entry.split("|");
    if (fields.length < 7) {
      continue;
    }

    const address = parseInt(fields[0], 10);
    const opcode = parseInt(fields[1], 10);
    const mnemonic = fields[2];
    const operand = parseInt(fields[3], 10);
    const formatted = fields[4];
    const bytes = parseInt(fields[5], 10);
    const cycles = parseInt(fields[6], 10);

    const prev = result[result.length - 1];
    const resolvedAddress =
      Number.isNaN(address) || address === 0
        ? prev
          ? prev.address + prev.bytes
          : 0
        : address;

    result.push({
      address: resolvedAddress,
      opcode: Number.isNaN(opcode) ? 0 : opcode,
      mnemonic,
      operand: Number.isNaN(operand) ? 0 : operand,
      formatted,
      bytes: Number.isNaN(bytes) ? 0 : bytes,
      cycles: Number.isNaN(cycles) ? 0 : cycles,
    });
  }

  return result;
}
