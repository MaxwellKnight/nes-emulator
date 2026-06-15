// web/src/emulator/devModule.test.ts
import { describe, it, expect } from "vitest";
import { createDevModule } from "./devModule";
import { createBridge } from "../wasm/bridge";

describe("createDevModule", () => {
  it("seeds a bridge with non-zero registers and demo state", () => {
    const dbg = createBridge(createDevModule());
    const regs = dbg.getRegisters();
    expect(regs.a).toBe(0x0a);
    expect(regs.x).toBe(0x0a);
    expect(regs.pc).toBe(0x0c00);
    expect(regs.sp).toBe(0xfd);
  });

  it("seeds program memory at $0C00 and zero-page bytes", () => {
    const dbg = createBridge(createDevModule());
    expect(dbg.readMemory(0x0c00)).toBe(0xa2); // LDX
    expect(dbg.readMemory(0x0c01)).toBe(0x0a);
    expect(dbg.readMemory(0x0000)).toBe(0x0a); // zero page seeded
  });

  it("seeds accumulated stats for the toolbar gauges", () => {
    const dbg = createBridge(createDevModule());
    const stats = dbg.getStats();
    expect(stats.instructionCount).toBeGreaterThan(0);
    expect(stats.cycleCount).toBeGreaterThan(0);
  });

  it("provides a non-empty demo disassembly", () => {
    const dbg = createBridge(createDevModule());
    const listing = dbg.disassembleAroundPC(5, 30);
    expect(listing.length).toBeGreaterThan(0);
    expect(listing[0].mnemonic).toBe("LDX");
  });
});
