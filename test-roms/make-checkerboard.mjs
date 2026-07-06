// Generates a minimal NROM (mapper-0) .nes ROM that renders a vertical
// stripe / checkerboard background — no external assets, no copyright.
//
//   - 16-byte iNES header (1x16KB PRG, 1x8KB CHR, mapper 0, horizontal mirror)
//   - PRG reset routine: enable BG, write a 4-entry palette, fill nametable 0
//     with alternating tile 0 (blank) / tile 1 (solid), then spin.
//   - CHR: tile 0 = blank, tile 1 = solid color-3.
//
// Run: node test-roms/make-checkerboard.mjs  ->  writes checkerboard.nes
import { writeFileSync } from "node:fs";

// ---- PRG code (loaded at $8000; reset vector points here) -----------------
const code = [];
let fillLabel = 0; // resolved below

// SEI; CLD; LDX #$FF; TXS
code.push(0x78, 0xd8, 0xa2, 0xff, 0x9a);
// LDA #$00; STA $2000   (PPUCTRL: nametable 0, BG pattern table 0)
code.push(0xa9, 0x00, 0x8d, 0x00, 0x20);
// LDA $2002             (reset the $2006 write latch)
code.push(0xad, 0x02, 0x20);
// PPUADDR = $3F00 (palette)
code.push(0xa9, 0x3f, 0x8d, 0x06, 0x20, 0xa9, 0x00, 0x8d, 0x06, 0x20);
// palette[0..3] = $0F (black), $16 (red), $2A (green), $30 (white)
for (const v of [0x0f, 0x16, 0x2a, 0x30]) code.push(0xa9, v, 0x8d, 0x07, 0x20);
// PPUADDR = $2000 (nametable 0)
code.push(0xa9, 0x20, 0x8d, 0x06, 0x20, 0xa9, 0x00, 0x8d, 0x06, 0x20);
// LDX #$04 ; LDY #$00   (4 pages of 256 = 1024 bytes covers the 1KB nametable)
code.push(0xa2, 0x04, 0xa0, 0x00);

// fill: TYA; AND #$01; STA $2007; INY; BNE fill; DEX; BNE fill
// Fill the nametable while rendering is still OFF, so $2007 auto-increments
// the loopy address cleanly (enabling BG first would let the renderer clobber
// the shared $2006/$2007 address mid-write).
fillLabel = code.length;
code.push(0x98); // TYA
code.push(0x29, 0x01); // AND #$01
code.push(0x8d, 0x07, 0x20); // STA $2007
code.push(0xc8); // INY
// BNE fill
code.push(0xd0);
code.push((fillLabel - (code.length + 1)) & 0xff); // rel8 back to fill
code.push(0xca); // DEX
// BNE fill
code.push(0xd0);
code.push((fillLabel - (code.length + 1)) & 0xff);

// Enable background LAST, once the nametable is fully populated.
// LDA #$08; STA $2001   (PPUMASK: show background)
code.push(0xa9, 0x08, 0x8d, 0x01, 0x20);

// spin: JMP spin
const spin = 0x8000 + code.length;
code.push(0x4c, spin & 0xff, (spin >> 8) & 0xff);

// ---- assemble 16KB PRG with vectors ---------------------------------------
const PRG = new Uint8Array(0x4000);
PRG.set(code, 0);
// vectors at $FFFA/$FFFC/$FFFE -> PRG offsets $3FFA/$3FFC/$3FFE
const setVec = (off, addr) => {
  PRG[off] = addr & 0xff;
  PRG[off + 1] = (addr >> 8) & 0xff;
};
setVec(0x3ffa, 0x8000); // NMI (unused)
setVec(0x3ffc, 0x8000); // RESET
setVec(0x3ffe, 0x8000); // IRQ (unused)

// ---- CHR: tile 0 blank, tile 1 solid color-3 ------------------------------
const CHR = new Uint8Array(0x2000);
for (let row = 0; row < 8; row++) {
  CHR[16 + row] = 0xff; // tile 1, low plane
  CHR[16 + 8 + row] = 0xff; // tile 1, high plane  -> pixel value 3
}

// ---- iNES header ----------------------------------------------------------
const header = new Uint8Array(16);
header.set([0x4e, 0x45, 0x53, 0x1a], 0); // "NES\x1a"
header[4] = 1; // 1 x 16KB PRG
header[5] = 1; // 1 x 8KB CHR
header[6] = 0x00; // mapper low nibble 0, horizontal mirroring
header[7] = 0x00; // mapper high nibble 0

const rom = new Uint8Array(16 + PRG.length + CHR.length);
rom.set(header, 0);
rom.set(PRG, 16);
rom.set(CHR, 16 + PRG.length);

const out = new URL("./checkerboard.nes", import.meta.url);
writeFileSync(out, rom);
console.log(`wrote ${out.pathname} (${rom.length} bytes)`);
console.log(`code length: ${code.length} bytes, spin @ $${spin.toString(16)}`);
