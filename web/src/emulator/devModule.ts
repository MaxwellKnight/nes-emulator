// web/src/emulator/devModule.ts
import type { WasmModule } from "../wasm/bridge";
import { createMockModule } from "../wasm/testing/mockModule";
import { loadEmulatorModule } from "../wasm/loader";

/**
 * A realistic `#`-delimited disassembly string for a small counter demo,
 * matching the C++ debugger's wire format:
 *   address|opcode|mnemonic|operand|formatted|bytes|cycles
 * Program (loaded at $0C00):
 *   LDX #$0A / STX $0200 / LDA #$00 / CLC / ADC $0201 / DEY / BNE / JMP
 */
const DEMO_DISASM = [
  "3072|162|LDX|10|#$0A|2|2",
  "3074|142|STX|512|$0200|3|4",
  "3077|169|LDA|0|#$00|2|2",
  "3079|24|CLC|0||1|2",
  "3080|109|ADC|513|$0201|3|4",
  "3083|136|DEY|0||1|2",
  "3084|208|BNE|250|$0C7A|2|3",
  "3086|76|JMP|3072|$0C00|3|3",
  "3089|234|NOP|0||1|2",
  "3090|169|LDA|1|#$01|2|2",
].join("#");

/**
 * Build a seeded mock module so the bento UI is fully populated and interactive
 * for design review when the compiled WASM artifact isn't available.
 */
export function createDevModule(): WasmModule {
  const mock = createMockModule({ disassembly: DEMO_DISASM });
  const s = mock._state;

  // Non-zero registers so the CPU State tile looks alive. PC sits on STX $0200.
  s.registers = { a: 0x0a, x: 0x0a, y: 0x03, sp: 0xfd, pc: 0x0c00, status: 0 };
  // N + I + Z lit (status bits 7, 2, 1).
  s.registers.status = 0b1000_0110;

  // A few bytes of program memory at $0C00 + some stack bytes around SP.
  const program = [
    0xa2, 0x0a, 0x8e, 0x00, 0x02, 0xa9, 0x00, 0x18, 0x6d, 0x01, 0x02, 0x88,
    0xd0, 0xfa, 0x4c, 0x00, 0xc0, 0xea, 0xa9, 0x01,
  ];
  program.forEach((b, i) => {
    s.memory[0x0c00 + i] = b;
  });
  // Some zero-page values so the default Memory view isn't empty.
  [0x0a, 0x03, 0x00, 0x18, 0x6d, 0x01, 0x00, 0x88].forEach((b, i) => {
    s.memory[i] = b;
  });
  // Stack contents around SP ($01FD).
  [0xfd, 0xc0, 0x03, 0xa9, 0x00, 0x18].forEach((b, i) => {
    s.memory[0x01fd + i] = b;
  });

  // A couple of breakpoints so the Breakpoints tile is populated.
  s.breakpoints.add(0x0c02);
  s.breakpoints.add(0x0c0c);

  // Some accumulated stats for the toolbar gauges.
  s.instructionCount = 12004;
  s.cycleCount = 48213;

  return mock;
}

/**
 * Load the real emulator module; in DEV, fall back to a seeded mock so the UI
 * renders without the compiled WASM artifact. Strictly gated to DEV so
 * production behaviour and all tests are unaffected.
 */
export async function loadModuleWithDevFallback(): Promise<WasmModule> {
  try {
    return await loadEmulatorModule();
  } catch (error) {
    if (import.meta.env.DEV) {
      // eslint-disable-next-line no-console
      console.warn(
        "[dev] Real WASM module failed to load — using seeded dev module.",
        error,
      );
      return createDevModule();
    }
    throw error;
  }
}
