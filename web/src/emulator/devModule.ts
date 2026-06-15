// web/src/emulator/devModule.ts
import { createBridge, type WasmModule } from "../wasm/bridge";
import { createMockModule } from "../wasm/testing/mockModule";
import { loadEmulatorModule } from "../wasm/loader";

// The counter demo program (loaded at $0C00) + a sprinkle of zero-page bytes,
// shared between the seeded mock and the real-module dev seeding below.
const DEMO_PROGRAM = [
  0xa2, 0x0a, 0x8e, 0x00, 0x02, 0xa0, 0x03, 0x8c, 0x01, 0x02, 0xac, 0x00, 0x02,
  0xa9, 0x00, 0x18, 0x6d, 0x01, 0x02, 0x88, 0xd0, 0xfa, 0x8d, 0x02, 0x02, 0x4c,
  0x0c, 0xc0,
];
const ZP_SEED = [
  0x0a, 0x03, 0x00, 0x18, 0x6d, 0x01, 0x00, 0x88, 0xd0, 0xfa, 0x8d, 0x02,
];

/**
 * A realistic `#`-delimited disassembly string for a small counter demo,
 * matching the C++ debugger's wire format:
 *   address|opcode|mnemonic|operand|formatted|bytes|cycles
 *
 * Program (loaded at $0C00) — the counter from the brief plus filler so the
 * Disassembly panel has enough rows to fill a tall window:
 *   LDX #$0A / STX $0200 / LDY #$03 / STY $0201 / LDY $0200 / LDA #$00 / CLC
 *   ADC $0201 / DEY / BNE / STA $0202 / JMP / ... NOPs / vectors
 */
const DEMO_DISASM = [
  "3072|162|LDX|10|#$0A|2|2",
  "3074|142|STX|512|$0200|3|4",
  "3077|160|LDY|3|#$03|2|2",
  "3079|140|STY|513|$0201|3|4",
  "3082|172|LDY|512|$0200|3|4",
  "3085|169|LDA|0|#$00|2|2",
  "3087|24|CLC|0||1|2",
  "3088|109|ADC|513|$0201|3|4",
  "3091|136|DEY|0||1|2",
  "3092|208|BNE|250|$0C56|2|3",
  "3094|141|STA|514|$0202|3|4",
  "3097|76|JMP|3084|$0C0C|3|3",
  "3100|234|NOP|0||1|2",
  "3101|234|NOP|0||1|2",
  "3102|169|LDA|1|#$01|2|2",
  "3104|133|STA|16|$10|2|3",
  "3106|230|INC|16|$10|2|5",
  "3108|76|JMP|3106|$0C22|3|3",
  "3111|162|LDX|255|#$FF|2|2",
  "3113|154|TXS|0||1|2",
  "3114|160|LDY|0|#$00|2|2",
  "3116|185|LDA|512|$0200,Y|3|4",
  "3119|153|STA|768|$0300,Y|3|5",
  "3122|200|INY|0||1|2",
  "3123|208|BNE|247|$0C53|2|3",
  "3125|96|RTS|0||1|6",
  "3126|234|NOP|0||1|2",
  "3127|234|NOP|0||1|2",
  "3128|234|NOP|0||1|2",
  "3129|234|NOP|0||1|2",
].join("#");

// Pack an RGBA pixel into 4 consecutive bytes (R,G,B,A) at `buf[off..]`.
function putPixel(
  buf: Uint8Array | Uint8ClampedArray,
  off: number,
  r: number,
  g: number,
  b: number,
): void {
  buf[off] = r;
  buf[off + 1] = g;
  buf[off + 2] = b;
  buf[off + 3] = 0xff;
}

/**
 * Paint a recognizable 256×240 RGBA demo image: an NES-blue backdrop with a
 * lighter 8×8 checkerboard, so the Screen canvas is obviously non-blank before
 * any ROM is loaded. DEV/demo-only — never used in production or by the core.
 */
export function fillDemoFramebuffer(
  buf: Uint8Array | Uint8ClampedArray,
): void {
  const W = 256;
  const H = 240;
  for (let y = 0; y < H; y += 1) {
    for (let x = 0; x < W; x += 1) {
      const off = (y * W + x) * 4;
      const checker = ((x >> 3) + (y >> 3)) & 1;
      if (checker) {
        putPixel(buf, off, 0x3c, 0x3c, 0xa0); // light NES-blue
      } else {
        putPixel(buf, off, 0x14, 0x14, 0x60); // dark NES-blue backdrop
      }
    }
  }
}

// A tiny demo nametable + palette so the PPU debug viewer shows content too.
function fillDemoNametable(nt: Uint8Array): void {
  for (let i = 0; i < nt.length; i += 1) {
    nt[i] = (i & 0x0f) === 0 ? 0x24 : i & 0x3f; // sprinkle non-zero tile ids
  }
}

function fillDemoPalette(pal: Uint8Array): void {
  // A plausible background palette set (indices into the master palette).
  const demo = [
    0x0f, 0x30, 0x21, 0x12, 0x0f, 0x27, 0x17, 0x07, 0x0f, 0x29, 0x1a, 0x0c,
    0x0f, 0x2b, 0x1b, 0x0d, 0x0f, 0x30, 0x21, 0x12, 0x0f, 0x27, 0x17, 0x07,
    0x0f, 0x29, 0x1a, 0x0c, 0x0f, 0x2b, 0x1b, 0x0d,
  ];
  for (let i = 0; i < 32; i += 1) pal[i] = demo[i];
}

/**
 * Build a seeded mock module so the cockpit UI is fully populated and
 * interactive for design review when the compiled WASM artifact isn't
 * available.
 */
export function createDevModule(): WasmModule {
  const mock = createMockModule({ disassembly: DEMO_DISASM });
  const s = mock._state;

  // Non-zero registers so the readout strip looks alive. PC sits on the loop.
  s.registers = { a: 0x0a, x: 0x0a, y: 0x03, sp: 0xfd, pc: 0x0c0c, status: 0 };
  // N + I + Z lit (status bits 7, 2, 1).
  s.registers.status = 0b1000_0110;

  // The counter demo program at $0C00 (matches the brief's byte sequence).
  DEMO_PROGRAM.forEach((b, i) => {
    s.memory[0x0c00 + i] = b;
  });
  // Some zero-page values so the default Memory view isn't empty.
  ZP_SEED.forEach((b, i) => {
    s.memory[i] = b;
  });
  // Stack contents around SP ($01FD).
  [0xfd, 0xc0, 0x03, 0xa9, 0x00, 0x18, 0x6d, 0x01, 0x02].forEach((b, i) => {
    s.memory[0x01fd + i] = b;
  });

  // A couple of breakpoints (on real instruction boundaries) so the
  // Breakpoints module + the disassembly dots are populated.
  s.breakpoints.add(0x0c0c); // LDY $0200 (current PC)
  s.breakpoints.add(0x0c14); // BNE

  // Some accumulated stats for the toolbar gauges.
  s.instructionCount = 12004;
  s.cycleCount = 48213;

  // Seed a non-blank demo screen + PPU debug buffers so the cockpit shows
  // content immediately (the mock's getters read these back through HEAPU8).
  fillDemoFramebuffer(s.framebuffer);
  fillDemoNametable(s.nametable);
  fillDemoPalette(s.paletteRam);

  return mock;
}

/**
 * Load the real emulator module; in DEV, fall back to a seeded mock so the UI
 * renders without the compiled WASM artifact. Strictly gated to DEV so
 * production behaviour and all tests are unaffected.
 */
export async function loadModuleWithDevFallback(): Promise<WasmModule> {
  try {
    const module = await loadEmulatorModule();
    // DEV-only: seed the real module with the demo program so the cockpit boots
    // alive (filled disassembly, a non-reset PC, populated memory) instead of a
    // sea of $00 / BRK. No-op in production builds; tests inject their own module.
    if (import.meta.env.DEV) {
      try {
        const bridge = createBridge(module);
        bridge.loadROM(Uint8Array.from(DEMO_PROGRAM));
        ZP_SEED.forEach((b, i) => bridge.writeMemory(i, b));
        // Advance a few instructions so the registers/flags read as live.
        for (let i = 0; i < 6; i++) bridge.step();
        // Paint a demo image straight into the framebuffer heap view so the
        // first blit is non-blank even before a ROM is loaded (DEV-only).
        try {
          const fb = bridge.getFramebuffer();
          fillDemoFramebuffer(fb);
        } catch {
          /* framebuffer not available yet — harmless in DEV */
        }
      } catch {
        /* ignore — leave the empty initial state */
      }
    }
    return module;
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
