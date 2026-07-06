// web/src/wasm/types.test.ts
import { describe, it, expect } from "vitest";
import { MEMORY_PAGES, FLAG_BITS } from "./types";

describe("types contract", () => {
  it("exposes exactly 4 memory pages", () => {
    expect(MEMORY_PAGES).toHaveLength(4);
    expect(MEMORY_PAGES.map((p) => p.id)).toEqual([
      "zeropage",
      "stack",
      "ram",
      "vectors",
    ]);
  });

  it("maps the zeropage page to $0000 with 256 bytes", () => {
    const zp = MEMORY_PAGES[0];
    expect(zp.start).toBe(0x0000);
    expect(zp.size).toBe(0x100);
    expect(zp.label).toBe("Zero Page ($0000-$00FF)");
  });

  it("maps the vectors page to $FFFA with 6 bytes", () => {
    const vectors = MEMORY_PAGES[3];
    expect(vectors.start).toBe(0xfffa);
    expect(vectors.size).toBe(6);
  });

  it("places the negative flag at bit 7 and carry at bit 0", () => {
    expect(FLAG_BITS.n).toBe(7);
    expect(FLAG_BITS.c).toBe(0);
    expect(FLAG_BITS.u).toBe(5);
  });
});
