// web/src/wasm/disassembly.test.ts
import { describe, it, expect } from "vitest";
import { parseDisassembly } from "./disassembly";
import type { DisassembledInstruction } from "./types";

describe("parseDisassembly", () => {
  it("parses a multi-instruction string into typed fields", () => {
    // LDX #$05  ; opcode 162, 2 bytes, 2 cycles at $0C00
    // LDA #$0A  ; opcode 169, 2 bytes, 2 cycles at $0C02
    const raw =
      "3072|162|LDX|5|LDX #$05|2|2#3074|169|LDA|10|LDA #$0A|2|2";
    const result = parseDisassembly(raw);
    expect(result).toHaveLength(2);

    const ldx: DisassembledInstruction = result[0];
    expect(ldx).toEqual({
      address: 3072,
      opcode: 162,
      mnemonic: "LDX",
      operand: 5,
      formatted: "LDX #$05",
      bytes: 2,
      cycles: 2,
    });

    const lda: DisassembledInstruction = result[1];
    expect(lda).toEqual({
      address: 3074,
      opcode: 169,
      mnemonic: "LDA",
      operand: 10,
      formatted: "LDA #$0A",
      bytes: 2,
      cycles: 2,
    });
  });

  it("ignores a trailing # separator", () => {
    const raw = "3072|162|LDX|5|LDX #$05|2|2#";
    const result = parseDisassembly(raw);
    expect(result).toHaveLength(1);
    expect(result[0].mnemonic).toBe("LDX");
  });

  it("backfills a missing/zero address from previous.address + previous.bytes", () => {
    // second entry reports address 0 -> backfilled to 3072 + 2 = 3074
    const raw = "3072|162|LDX|5|LDX #$05|2|2#0|169|LDA|10|LDA #$0A|2|2";
    const result = parseDisassembly(raw);
    expect(result[0].address).toBe(3072);
    expect(result[1].address).toBe(3074);
  });

  it("returns an empty array for an empty string", () => {
    expect(parseDisassembly("")).toEqual([]);
    expect(parseDisassembly("#")).toEqual([]);
  });
});
