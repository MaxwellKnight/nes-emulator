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
  HEAPU8: Uint8Array;
  HEAPF32: Float32Array;
  _malloc(size: number): number;
  _free(ptr: number): void;
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
  loadRom(bytes: Uint8Array): number;
  getFramebuffer(): Uint8ClampedArray;
  runFrame(): number;
  frameCount(): number;
  renderPatternTable(table: number, palette: number): Uint8ClampedArray;
  getNametable(): Uint8Array;
  getPaletteRam(): Uint8Array;
  getOam(): Uint8Array;
  setController(state: number, port?: number): void;
  audioAvailable(): number;
  audioDrain(max: number): Float32Array;
  ppuState(): { ctrl: number; mask: number; status: number; scanline: number };
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

  const loadRomRaw = module.cwrap("load_rom", "number", [
    "number",
    "number",
  ]) as (ptr: number, len: number) => number;
  const getFramebufferPtr = module.cwrap(
    "get_framebuffer_ptr",
    "number",
    []
  ) as () => number;
  const getFramebufferLen = module.cwrap(
    "get_framebuffer_len",
    "number",
    []
  ) as () => number;
  const getFrameCount = module.cwrap(
    "get_frame_count",
    "number",
    []
  ) as () => number;
  const runFrameRaw = module.cwrap("run_frame", "number", []) as () => number;
  const renderPatternTableRaw = module.cwrap("ppu_render_pattern_table", null, [
    "number",
    "number",
    "number",
  ]) as (table: number, palette: number, out: number) => void;
  const getNametablePtr = module.cwrap(
    "get_nametable_ptr",
    "number",
    []
  ) as () => number;
  const getPaletteRamPtr = module.cwrap(
    "get_palette_ram_ptr",
    "number",
    []
  ) as () => number;
  const getOamPtr = module.cwrap("get_oam_ptr", "number", []) as () => number;
  const setControllerRaw = module.cwrap("set_controller", null, [
    "number",
    "number",
  ]) as (port: number, buttons: number) => void;
  const audioAvailableRaw = module.cwrap(
    "audio_available",
    "number",
    [],
  ) as () => number;
  const audioDrainRaw = module.cwrap("audio_drain", "number", [
    "number",
    "number",
  ]) as (ptr: number, max: number) => number;
  // Reusable heap buffer for draining audio samples (one frame is ~735 samples).
  const AUDIO_BUF = 4096;
  const audioBufPtr = module._malloc(AUDIO_BUF * 4);
  const ppuGetCtrl = module.cwrap("ppu_get_ctrl", "number", []) as () => number;
  const ppuGetMask = module.cwrap("ppu_get_mask", "number", []) as () => number;
  const ppuGetStatus = module.cwrap(
    "ppu_get_status",
    "number",
    []
  ) as () => number;
  const ppuGetScanline = module.cwrap(
    "ppu_get_scanline",
    "number",
    []
  ) as () => number;

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
    // Reset vector is little-endian: low byte at $FFFC, high byte at $FFFD.
    writeMemory(0xfffc, startAddr & 0xff);
    writeMemory(0xfffd, (startAddr >> 8) & 0xff);
    setPC(startAddr);
  }

  function loadRom(bytes: Uint8Array): number {
    const ptr = module._malloc(bytes.length);
    try {
      module.HEAPU8.set(bytes, ptr);
      return loadRomRaw(ptr, bytes.length) as number;
    } finally {
      module._free(ptr);
    }
  }

  function getFramebuffer(): Uint8ClampedArray {
    const ptr = getFramebufferPtr();
    const len = getFramebufferLen();
    return new Uint8ClampedArray(module.HEAPU8.buffer, ptr, len);
  }

  function runFrame(): number {
    return runFrameRaw() as number;
  }

  function frameCount(): number {
    return getFrameCount() as number;
  }

  function renderPatternTable(
    table: number,
    palette: number
  ): Uint8ClampedArray {
    const len = 128 * 128 * 4;
    const ptr = module._malloc(len);
    try {
      renderPatternTableRaw(table, palette, ptr);
      // Copy out of the heap so the caller owns a stable buffer after free.
      return new Uint8ClampedArray(
        module.HEAPU8.buffer.slice(ptr, ptr + len)
      );
    } finally {
      module._free(ptr);
    }
  }

  function getNametable(): Uint8Array {
    const ptr = getNametablePtr();
    return new Uint8Array(module.HEAPU8.buffer, ptr, 2048);
  }

  function getPaletteRam(): Uint8Array {
    const ptr = getPaletteRamPtr();
    return new Uint8Array(module.HEAPU8.buffer, ptr, 32);
  }

  function getOam(): Uint8Array {
    const ptr = getOamPtr();
    return new Uint8Array(module.HEAPU8.buffer, ptr, 256);
  }

  function setController(state: number, port = 0): void {
    setControllerRaw(port, state & 0xff);
  }

  function audioAvailable(): number {
    return audioAvailableRaw();
  }

  function audioDrain(max: number): Float32Array {
    const n = audioDrainRaw(audioBufPtr, Math.min(max, AUDIO_BUF));
    // Copy out of the heap view so the caller owns a stable buffer.
    return module.HEAPF32.subarray(
      audioBufPtr / 4,
      audioBufPtr / 4 + n,
    ).slice();
  }

  function ppuState(): {
    ctrl: number;
    mask: number;
    status: number;
    scanline: number;
  } {
    return {
      ctrl: ppuGetCtrl(),
      mask: ppuGetMask(),
      status: ppuGetStatus(),
      scanline: ppuGetScanline(),
    };
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
    loadRom,
    getFramebuffer,
    runFrame,
    frameCount,
    renderPatternTable,
    getNametable,
    getPaletteRam,
    getOam,
    setController,
    audioAvailable,
    audioDrain,
    ppuState,
  };
}
