// web/src/wasm/bridge.ts
import type {
  Registers,
  Flags,
  Stats,
  EmulatorSnapshot,
  DisassembledInstruction,
} from "./types";
import { FLAG_BITS } from "./types";
import { parseDisassembly } from "./disassembly";

export interface WasmModule {
  cwrap(
    name: string,
    ret: string | null,
    args: string[]
  ): (...a: number[]) => number | string | void;
  ccall(
    name: string,
    ret: string | null,
    args: string[],
    values: Array<number | string>
  ): number | string | void;
}

export interface Debugger {
  step(): void;
  run(): void;
  stop(): void;
  reset(): void;
  isRunning(): boolean;
  addBreakpoint(addr: number): void;
  removeBreakpoint(addr: number): void;
  clearBreakpoints(): void;
  getRegisters(): Registers;
  getFlags(): Flags;
  getStats(): Stats;
  getSnapshot(): EmulatorSnapshot;
  readMemory(addr: number): number;
  writeMemory(addr: number, value: number): void;
  readMemoryRange(start: number, end: number): number[];
  setPC(addr: number): void;
  disassembleAroundPC(
    before: number,
    after: number
  ): DisassembledInstruction[];
  disassembleRange(start: number, end: number): DisassembledInstruction[];
  loadROM(data: Uint8Array | number[], startAddr?: number): void;
}

export function createBridge(module: WasmModule): Debugger {
  const step = module.cwrap("debugger_step", null, []) as () => void;
  const run = module.cwrap("debugger_run", null, []) as () => void;
  const stop = module.cwrap("debugger_stop", null, []) as () => void;
  const reset = module.cwrap("debugger_reset", null, []) as () => void;
  const isRunningRaw = module.cwrap(
    "debugger_is_running",
    "number",
    []
  ) as () => number;

  const addBreakpointRaw = module.cwrap("debugger_add_breakpoint", null, [
    "number",
  ]) as (addr: number) => void;
  const removeBreakpointRaw = module.cwrap(
    "debugger_remove_breakpoint",
    null,
    ["number"]
  ) as (addr: number) => void;
  const clearBreakpointsRaw = module.cwrap(
    "debugger_clear_breakpoints",
    null,
    []
  ) as () => void;

  const getA = module.cwrap("debugger_get_register_a", "number", []) as () => number;
  const getX = module.cwrap("debugger_get_register_x", "number", []) as () => number;
  const getY = module.cwrap("debugger_get_register_y", "number", []) as () => number;
  const getSP = module.cwrap("debugger_get_register_sp", "number", []) as () => number;
  const getPC = module.cwrap("debugger_get_register_pc", "number", []) as () => number;
  const getStatus = module.cwrap(
    "debugger_get_register_status",
    "number",
    []
  ) as () => number;
  const getStatusFlag = module.cwrap("debugger_get_status_flag", "number", [
    "number",
  ]) as (bit: number) => number;

  const readMemoryRaw = module.cwrap("debugger_read_memory", "number", [
    "number",
  ]) as (addr: number) => number;
  const writeMemoryRaw = module.cwrap("debugger_write_memory", null, [
    "number",
    "number",
  ]) as (addr: number, value: number) => void;

  const getInstructionCount = module.cwrap(
    "debugger_get_instruction_count",
    "number",
    []
  ) as () => number;
  const getCycleCount = module.cwrap(
    "debugger_get_cycle_count",
    "number",
    []
  ) as () => number;

  // CRITICAL: the current code declared this with an empty arg list, so the
  // address was dropped and PC was never set. The correct binding is ["number"].
  const setPCRaw = module.cwrap("debugger_set_pc", null, ["number"]) as (
    addr: number
  ) => void;

  function getRegisters(): Registers {
    return {
      a: getA(),
      x: getX(),
      y: getY(),
      sp: getSP(),
      pc: getPC(),
      status: getStatus(),
    };
  }

  function getFlags(): Flags {
    return {
      n: getStatusFlag(FLAG_BITS.n) !== 0,
      v: getStatusFlag(FLAG_BITS.v) !== 0,
      u: getStatusFlag(FLAG_BITS.u) !== 0,
      b: getStatusFlag(FLAG_BITS.b) !== 0,
      d: getStatusFlag(FLAG_BITS.d) !== 0,
      i: getStatusFlag(FLAG_BITS.i) !== 0,
      z: getStatusFlag(FLAG_BITS.z) !== 0,
      c: getStatusFlag(FLAG_BITS.c) !== 0,
    };
  }

  function getStats(): Stats {
    return {
      instructionCount: getInstructionCount(),
      cycleCount: getCycleCount(),
    };
  }

  function isRunning(): boolean {
    return isRunningRaw() !== 0;
  }

  function getSnapshot(): EmulatorSnapshot {
    return {
      registers: getRegisters(),
      flags: getFlags(),
      stats: getStats(),
      running: isRunning(),
    };
  }

  function readMemory(addr: number): number {
    return readMemoryRaw(addr & 0xffff);
  }

  function writeMemory(addr: number, value: number): void {
    writeMemoryRaw(addr & 0xffff, value & 0xff);
  }

  function readMemoryRange(start: number, end: number): number[] {
    const out: number[] = [];
    for (let addr = start; addr <= end; addr += 1) {
      out.push(readMemoryRaw(addr & 0xffff));
    }
    return out;
  }

  function setPC(addr: number): void {
    setPCRaw(addr & 0xffff);
  }

  function disassembleAroundPC(
    before: number,
    after: number
  ): DisassembledInstruction[] {
    const raw = module.ccall(
      "debugger_disassemble_around_pc",
      "string",
      ["number", "number"],
      [before, after]
    ) as string;
    return parseDisassembly(raw);
  }

  function disassembleRange(
    start: number,
    end: number
  ): DisassembledInstruction[] {
    const raw = module.ccall(
      "debugger_disassemble_range",
      "string",
      ["number", "number"],
      [start, end]
    ) as string;
    return parseDisassembly(raw);
  }

  function loadROM(
    data: Uint8Array | number[],
    startAddr = 0x0c00
  ): void {
    for (let i = 0; i < data.length; i += 1) {
      writeMemory(startAddr + i, data[i]);
    }
    writeMemory(0xfffd, (startAddr >> 8) & 0xff);
    setPC(startAddr);
  }

  return {
    step,
    run,
    stop,
    reset,
    isRunning,
    addBreakpoint: (addr: number) => addBreakpointRaw(addr & 0xffff),
    removeBreakpoint: (addr: number) => removeBreakpointRaw(addr & 0xffff),
    clearBreakpoints: clearBreakpointsRaw,
    getRegisters,
    getFlags,
    getStats,
    getSnapshot,
    readMemory,
    writeMemory,
    readMemoryRange,
    setPC,
    disassembleAroundPC,
    disassembleRange,
    loadROM,
  };
}
