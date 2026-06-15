// web/src/wasm/testing/mockModule.test.ts
import { describe, it, expect } from "vitest";
import { createMockModule } from "./mockModule";

describe("createMockModule", () => {
  it("round-trips set_pc then get_register_pc via cwrap", () => {
    const mod = createMockModule();
    const setPc = mod.cwrap("debugger_set_pc", null, ["number"]) as (
      a: number
    ) => void;
    const getPc = mod.cwrap("debugger_get_register_pc", "number", []) as () => number;

    setPc(0x0c00);
    expect(getPc()).toBe(0x0c00);
    expect(mod._state.registers.pc).toBe(0x0c00);
  });

  it("round-trips write_memory then read_memory via cwrap", () => {
    const mod = createMockModule();
    const write = mod.cwrap("debugger_write_memory", null, [
      "number",
      "number",
    ]) as (addr: number, value: number) => void;
    const read = mod.cwrap("debugger_read_memory", "number", ["number"]) as (
      addr: number
    ) => number;

    write(0x0200, 0x42);
    expect(read(0x0200)).toBe(0x42);
    expect(mod._state.memory[0x0200]).toBe(0x42);
  });

  it("returns the configured disassembly string from disassemble_* via ccall", () => {
    const mod = createMockModule({ disassembly: "3072|162|LDX|5|LDX #$05|2|2" });
    const around = mod.ccall(
      "debugger_disassemble_around_pc",
      "string",
      ["number", "number"],
      [5, 30]
    );
    expect(around).toBe("3072|162|LDX|5|LDX #$05|2|2");
  });

  it("seeds the stack pointer to 0xFD", () => {
    const mod = createMockModule();
    expect(mod._state.registers.sp).toBe(0xfd);
  });
});
