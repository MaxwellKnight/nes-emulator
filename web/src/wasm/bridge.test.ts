// web/src/wasm/bridge.test.ts
import { describe, it, expect, vi } from "vitest";
import { createBridge } from "./bridge";
import { createMockModule } from "./testing/mockModule";
import { FLAG_BITS } from "./types";

describe("createBridge", () => {
  it("wraps setPC with a numeric arg and actually updates PC", () => {
    const mod = createMockModule();
    const cwrapSpy = vi.spyOn(mod, "cwrap");
    const dbg = createBridge(mod);

    dbg.setPC(0x0c00);

    expect(cwrapSpy).toHaveBeenCalledWith("debugger_set_pc", null, [
      "number",
    ]);
    expect(mod._state.registers.pc).toBe(0x0c00);
    expect(dbg.getRegisters().pc).toBe(0x0c00);
  });

  it("maps getRegisters from the individual register getters", () => {
    const mod = createMockModule();
    mod._state.registers = {
      a: 0x11,
      x: 0x22,
      y: 0x33,
      sp: 0xfd,
      pc: 0x0c00,
      status: 0x81,
    };
    const dbg = createBridge(mod);

    expect(dbg.getRegisters()).toEqual({
      a: 0x11,
      x: 0x22,
      y: 0x33,
      sp: 0xfd,
      pc: 0x0c00,
      status: 0x81,
    });
  });

  it("maps getFlags via FLAG_BITS bit indices", () => {
    const mod = createMockModule();
    // status 0x81 = 1000_0001 => N set (bit 7) and C set (bit 0); rest clear
    mod._state.registers.status = 0x81;
    const dbg = createBridge(mod);

    const flags = dbg.getFlags();
    expect(flags.n).toBe(true);
    expect(flags.c).toBe(true);
    expect(flags.v).toBe(false);
    expect(flags.u).toBe(false);
    expect(flags.b).toBe(false);
    expect(flags.d).toBe(false);
    expect(flags.i).toBe(false);
    expect(flags.z).toBe(false);
    // sanity: the bridge consulted bit 7 for n and bit 0 for c
    expect(FLAG_BITS.n).toBe(7);
    expect(FLAG_BITS.c).toBe(0);
  });

  it("maps getStats and getSnapshot", () => {
    const mod = createMockModule();
    mod._state.instructionCount = 7;
    mod._state.cycleCount = 21;
    mod._state.running = true;
    const dbg = createBridge(mod);

    expect(dbg.getStats()).toEqual({ instructionCount: 7, cycleCount: 21 });

    const snap = dbg.getSnapshot();
    expect(snap.stats).toEqual({ instructionCount: 7, cycleCount: 21 });
    expect(snap.running).toBe(true);
    expect(snap.registers).toEqual(dbg.getRegisters());
    expect(snap.flags).toEqual(dbg.getFlags());
  });

  it("reads and writes memory and reads ranges", () => {
    const mod = createMockModule();
    const dbg = createBridge(mod);

    dbg.writeMemory(0x0200, 0x42);
    expect(dbg.readMemory(0x0200)).toBe(0x42);

    dbg.writeMemory(0x0201, 0x43);
    expect(dbg.readMemoryRange(0x0200, 0x0201)).toEqual([0x42, 0x43]);
  });

  it("adds, removes, and clears breakpoints", () => {
    const mod = createMockModule();
    const dbg = createBridge(mod);

    dbg.addBreakpoint(0x0c00);
    dbg.addBreakpoint(0x0c10);
    expect(mod._state.breakpoints.has(0x0c00)).toBe(true);
    expect(mod._state.breakpoints.has(0x0c10)).toBe(true);

    dbg.removeBreakpoint(0x0c00);
    expect(mod._state.breakpoints.has(0x0c00)).toBe(false);

    dbg.clearBreakpoints();
    expect(mod._state.breakpoints.size).toBe(0);
  });

  it("reports running state and run/stop transitions", () => {
    const mod = createMockModule();
    const dbg = createBridge(mod);

    expect(dbg.isRunning()).toBe(false);
    dbg.run();
    expect(dbg.isRunning()).toBe(true);
    dbg.stop();
    expect(dbg.isRunning()).toBe(false);
  });

  it("parses disassembleAroundPC and disassembleRange via parseDisassembly", () => {
    const mod = createMockModule({
      disassembly: "3072|162|LDX|5|LDX #$05|2|2#3074|169|LDA|10|LDA #$0A|2|2",
    });
    const dbg = createBridge(mod);

    const around = dbg.disassembleAroundPC(5, 30);
    expect(around).toHaveLength(2);
    expect(around[0].mnemonic).toBe("LDX");
    expect(around[1].mnemonic).toBe("LDA");

    const range = dbg.disassembleRange(0x0c00, 0x0c10);
    expect(range).toHaveLength(2);
    expect(range[0].opcode).toBe(162);
  });

  it("loadROM writes bytes from 0x0C00, sets 0xFFFD, and sets PC to 0x0C00", () => {
    const mod = createMockModule();
    const dbg = createBridge(mod);

    dbg.loadROM([0xa2, 0x05, 0xa9, 0x0a]);

    expect(mod._state.memory[0x0c00]).toBe(0xa2);
    expect(mod._state.memory[0x0c01]).toBe(0x05);
    expect(mod._state.memory[0x0c02]).toBe(0xa9);
    expect(mod._state.memory[0x0c03]).toBe(0x0a);
    // high byte of the reset vector
    expect(mod._state.memory[0xfffd]).toBe(0x0c);
    // PC positioned at the start address
    expect(mod._state.registers.pc).toBe(0x0c00);
  });

  it("loadROM honors a custom start address", () => {
    const mod = createMockModule();
    const dbg = createBridge(mod);

    dbg.loadROM([0xea], 0x8000);

    expect(mod._state.memory[0x8000]).toBe(0xea);
    expect(mod._state.memory[0xfffd]).toBe(0x80);
    expect(mod._state.registers.pc).toBe(0x8000);
  });
});
