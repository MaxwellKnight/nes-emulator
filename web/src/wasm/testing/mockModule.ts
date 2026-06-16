// web/src/wasm/testing/mockModule.ts

/** Minimal shape of the Emscripten runtime module (mirrors WasmModule in bridge.ts). */
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

export interface MockState {
  memory: Uint8Array;
  registers: {
    a: number;
    x: number;
    y: number;
    sp: number;
    pc: number;
    status: number;
  };
  breakpoints: Set<number>;
  running: boolean;
  instructionCount: number;
  cycleCount: number;
  // PPU / ROM state for the Phase 1 exports.
  romStatus: number;
  loadedRom: Uint8Array | null;
  frameCount: number;
  framebuffer: Uint8Array; // 256*240*4 RGBA
  patternTable: Uint8Array; // 128*128*4 RGBA
  nametable: Uint8Array; // 2048 bytes
  paletteRam: Uint8Array; // 32 bytes
  oam: Uint8Array; // 256 bytes (64 sprites x 4)
  ppu: { ctrl: number; mask: number; status: number; scanline: number };
  frameReason: number; // reason returned by run_frame: 0 frame, 1 breakpoint, 2 brk
}

/**
 * An in-memory test double for the Emscripten module the bridge wraps.
 * cwrap/ccall dispatch on the exported function name against MockState.
 */
export function createMockModule(opts?: {
  disassembly?: string;
  romStatus?: number;
  frameReason?: number;
}): WasmModule & { _state: MockState } {
  const FB_LEN = 256 * 240 * 4;
  const PT_LEN = 128 * 128 * 4;
  const state: MockState = {
    memory: new Uint8Array(0x10000),
    registers: { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0, status: 0 },
    breakpoints: new Set<number>(),
    running: false,
    instructionCount: 0,
    cycleCount: 0,
    romStatus: opts?.romStatus ?? 0,
    loadedRom: null,
    frameCount: 0,
    framebuffer: new Uint8Array(FB_LEN),
    patternTable: new Uint8Array(PT_LEN),
    nametable: new Uint8Array(2048),
    paletteRam: new Uint8Array(32),
    oam: new Uint8Array(256),
    ppu: { ctrl: 0, mask: 0, status: 0, scanline: 0 },
    frameReason: opts?.frameReason ?? 0,
  };

  const disassembly = opts?.disassembly ?? "";

  // A simulated WASM heap. Pointers are byte offsets into `heap`. The bridge
  // reads framebuffer/pattern bytes back through HEAPU8 at the pointers the
  // getter functions return, so those pointers must live inside this heap.
  let heap = new Uint8Array(1 << 20);
  let brk = 64; // first allocatable offset (skip the null region)
  const allocs = new Set<number>();

  // Fixed offsets for the PPU buffers the getters return pointers to.
  const FB_PTR = 8;
  // The nametable/palette views also need stable backing pointers. Place them
  // after a generous gap so malloc'd ROM buffers never collide with them.
  const NT_PTR = FB_LEN + 16;
  const PAL_PTR = NT_PTR + 2048 + 16;
  const OAM_PTR = PAL_PTR + 32 + 16;
  brk = OAM_PTR + 256 + 16;

  function syncOut(): void {
    heap.set(state.framebuffer, FB_PTR);
    heap.set(state.nametable, NT_PTR);
    heap.set(state.paletteRam, PAL_PTR);
    heap.set(state.oam, OAM_PTR);
  }
  syncOut();

  function malloc(size: number): number {
    const ptr = brk;
    brk += size + (8 - (size % 8)); // 8-byte align
    if (brk > heap.length) {
      const grown = new Uint8Array(brk + (1 << 20));
      grown.set(heap);
      heap = grown;
      module.HEAPU8 = heap;
    }
    allocs.add(ptr);
    return ptr;
  }

  function free(ptr: number): void {
    allocs.delete(ptr);
  }

  function dispatch(
    name: string,
    args: Array<number | string>
  ): number | string | void {
    const n = (i: number): number => Number(args[i]);

    switch (name) {
      case "debugger_step":
        state.instructionCount += 1;
        state.cycleCount += 2;
        state.registers.pc = (state.registers.pc + 1) & 0xffff;
        return;
      case "debugger_run":
        state.running = true;
        return;
      case "debugger_stop":
        state.running = false;
        return;
      case "debugger_reset":
        state.registers = { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0, status: 0 };
        state.instructionCount = 0;
        state.cycleCount = 0;
        state.running = false;
        return;
      case "debugger_is_running":
        return state.running ? 1 : 0;
      case "debugger_add_breakpoint":
        state.breakpoints.add(n(0) & 0xffff);
        return;
      case "debugger_remove_breakpoint":
        state.breakpoints.delete(n(0) & 0xffff);
        return;
      case "debugger_clear_breakpoints":
        state.breakpoints.clear();
        return;
      case "debugger_get_register_a":
        return state.registers.a;
      case "debugger_get_register_x":
        return state.registers.x;
      case "debugger_get_register_y":
        return state.registers.y;
      case "debugger_get_register_sp":
        return state.registers.sp;
      case "debugger_get_register_pc":
        return state.registers.pc;
      case "debugger_get_register_status":
        return state.registers.status;
      case "debugger_get_status_flag":
        return (state.registers.status >> (n(0) & 7)) & 1;
      case "debugger_read_memory":
        return state.memory[n(0) & 0xffff];
      case "debugger_write_memory":
        state.memory[n(0) & 0xffff] = n(1) & 0xff;
        return;
      case "debugger_get_instruction_count":
        return state.instructionCount;
      case "debugger_get_cycle_count":
        return state.cycleCount;
      case "debugger_set_pc":
        state.registers.pc = n(0) & 0xffff;
        return;
      case "debugger_disassemble_around_pc":
      case "debugger_disassemble_range":
        return disassembly;
      case "load_rom": {
        const ptr = n(0);
        const len = n(1);
        state.loadedRom = heap.slice(ptr, ptr + len);
        return state.romStatus;
      }
      case "get_framebuffer_ptr":
        heap.set(state.framebuffer, FB_PTR);
        return FB_PTR;
      case "get_framebuffer_len":
        return state.framebuffer.length;
      case "get_frame_count":
        return state.frameCount;
      case "run_frame":
        state.frameCount += 1;
        return state.frameReason;
      case "ppu_render_pattern_table": {
        // args: table, palette, outPtr — copy the canned pattern into the heap.
        const outPtr = n(2);
        heap.set(state.patternTable, outPtr);
        return;
      }
      case "get_nametable_ptr":
        heap.set(state.nametable, NT_PTR);
        return NT_PTR;
      case "get_palette_ram_ptr":
        heap.set(state.paletteRam, PAL_PTR);
        return PAL_PTR;
      case "get_oam_ptr":
        heap.set(state.oam, OAM_PTR);
        return OAM_PTR;
      case "set_controller":
        return;  // no-op in the mock (input is exercised via the real core)
      case "audio_available":
        return 0;  // the mock produces no audio
      case "audio_drain":
        return 0;
      case "ppu_get_ctrl":
        return state.ppu.ctrl;
      case "ppu_get_mask":
        return state.ppu.mask;
      case "ppu_get_status":
        return state.ppu.status;
      case "ppu_get_scanline":
        return state.ppu.scanline;
      default:
        throw new Error(`createMockModule: unhandled function "${name}"`);
    }
  }

  const module: WasmModule & { _state: MockState } = {
    _state: state,
    HEAPU8: heap,
    HEAPF32: new Float32Array(heap.buffer),
    _malloc: malloc,
    _free: free,
    cwrap(name, _ret, argTypes) {
      return (...callArgs: number[]) =>
        dispatch(name, callArgs.slice(0, argTypes.length));
    },
    ccall(name, _ret, _argTypes, values) {
      return dispatch(name, values);
    },
  };

  return module;
}
