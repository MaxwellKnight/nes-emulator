// web/src/wasm/opcodes.test.ts
import { describe, it, expect } from "vitest";
import { parseOpcodes } from "./opcodes";

describe("parseOpcodes", () => {
  it("parses whitespace-separated hex bytes", () => {
    expect(parseOpcodes("A9 0A 8D 00 02")).toEqual([
      0xa9, 0x0a, 0x8d, 0x00, 0x02,
    ]);
  });

  it("parses comma-separated hex bytes", () => {
    expect(parseOpcodes("A2,05,A9,0A")).toEqual([0xa2, 0x05, 0xa9, 0x0a]);
  });

  it("parses mixed whitespace, newlines, and commas", () => {
    expect(parseOpcodes("A9 0A\n8D, 00\t02")).toEqual([
      0xa9, 0x0a, 0x8d, 0x00, 0x02,
    ]);
  });

  it("strips ; line comments", () => {
    const text = "A9 0A   ; load 10 into A\n8D 00 02 ; store to $0200";
    expect(parseOpcodes(text)).toEqual([0xa9, 0x0a, 0x8d, 0x00, 0x02]);
  });

  it("accepts single hex digits", () => {
    expect(parseOpcodes("A 0 F")).toEqual([0x0a, 0x00, 0x0f]);
  });

  it("returns an empty array for an empty or comment-only string", () => {
    expect(parseOpcodes("")).toEqual([]);
    expect(parseOpcodes("; just a comment")).toEqual([]);
  });

  it("throws on an invalid (non-hex) token", () => {
    expect(() => parseOpcodes("A9 ZZ 0A")).toThrowError(
      "Invalid opcode format: ZZ"
    );
  });

  it("throws on a token longer than 2 hex digits", () => {
    expect(() => parseOpcodes("A9 1234")).toThrowError(
      "Invalid opcode format: 1234"
    );
  });
});
