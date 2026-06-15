// web/src/wasm/testing/mockModule.ts
import type { WasmModule } from "../bridge";

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
}

/**
 * An in-memory test double for the Emscripten module the bridge wraps.
 * cwrap/ccall dispatch on the exported function name against MockState.
 */
export function createMockModule(opts?: {
  disassembly?: string;
}): WasmModule & { _state: MockState } {
  const state: MockState = {
    memory: new Uint8Array(0x10000),
    registers: { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0, status: 0 },
    breakpoints: new Set<number>(),
    running: false,
    instructionCount: 0,
    cycleCount: 0,
  };

  const disassembly = opts?.disassembly ?? "";

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
      default:
        throw new Error(`createMockModule: unhandled function "${name}"`);
    }
  }

  const module: WasmModule & { _state: MockState } = {
    _state: state,
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
