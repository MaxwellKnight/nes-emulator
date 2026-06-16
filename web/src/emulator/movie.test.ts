import { describe, it, expect } from "vitest";
import { parseMovie } from "./movie";

// Build a minimal valid .nesmovie buffer: magic + version + rom_len + frames +
// rom bytes + input bytes.
function makeMovie(rom: number[], inputs: number[]): Uint8Array {
  const out = new Uint8Array(20 + rom.length + inputs.length);
  out.set([...new TextEncoder().encode("NESMOVIE")], 0);
  const view = new DataView(out.buffer);
  view.setUint32(8, 1, true);
  view.setUint32(12, rom.length, true);
  view.setUint32(16, inputs.length, true);
  out.set(rom, 20);
  out.set(inputs, 20 + rom.length);
  return out;
}

describe("parseMovie", () => {
  it("parses a well-formed movie", () => {
    const m = parseMovie(makeMovie([0xde, 0xad], [0x00, 0x80, 0x81]));
    expect(m.version).toBe(1);
    expect(Array.from(m.rom)).toEqual([0xde, 0xad]);
    expect(Array.from(m.inputs)).toEqual([0x00, 0x80, 0x81]);
  });

  it("rejects a buffer without the magic", () => {
    const bad = new Uint8Array(40);
    expect(() => parseMovie(bad)).toThrow(/not a .nesmovie/);
  });

  it("rejects a truncated movie", () => {
    const m = makeMovie([1, 2, 3], [0, 0, 0, 0]);
    expect(() => parseMovie(m.subarray(0, m.length - 2))).toThrow(/truncated/);
  });

  it("rejects a buffer that is too small", () => {
    expect(() => parseMovie(new Uint8Array(8))).toThrow(/too small/);
  });
});
