#include <gtest/gtest.h>
#include <set>
#include <vector>
#include "bus.h"
#include "cartridge.h"
#include "debugger.h"
#include "palette.h"

using namespace nes;

namespace {

// Build a minimal NROM (mapper-0) iNES image whose reset vector points at
// $8000 and whose PRG program: sets a 4-entry background palette, fills
// nametable 0 with alternating tile 0 (blank) / tile 1 (solid color-3) while
// rendering is OFF, then enables background and spins. CHR tile 1 is solid.
// Mirrors test-roms/make-checkerboard.mjs.
std::vector<u8> make_checkerboard_rom() {
  std::vector<u8> code;
  auto emit = [&](std::initializer_list<u8> bytes) {
    for (u8 b : bytes) code.push_back(b);
  };

  emit({0x78, 0xD8, 0xA2, 0xFF, 0x9A});        // SEI; CLD; LDX #$FF; TXS
  emit({0xA9, 0x00, 0x8D, 0x00, 0x20});        // LDA #$00; STA $2000 (PPUCTRL)
  emit({0xAD, 0x02, 0x20});                    // LDA $2002 (reset $2006 latch)
  emit({0xA9, 0x3F, 0x8D, 0x06, 0x20});        // PPUADDR = $3F..
  emit({0xA9, 0x00, 0x8D, 0x06, 0x20});        // PPUADDR = $..00  -> $3F00
  for (u8 v : {0x0F, 0x16, 0x2A, 0x30}) {      // palette[0..3]
    emit({0xA9, v, 0x8D, 0x07, 0x20});
  }
  emit({0xA9, 0x20, 0x8D, 0x06, 0x20});        // PPUADDR = $20..
  emit({0xA9, 0x00, 0x8D, 0x06, 0x20});        // PPUADDR = $..00  -> $2000

  emit({0xA2, 0x04, 0xA0, 0x00});              // LDX #$04; LDY #$00
  const std::size_t fill = code.size();        // fill:
  emit({0x98});                                //   TYA
  emit({0x29, 0x01});                          //   AND #$01
  emit({0x8D, 0x07, 0x20});                    //   STA $2007
  emit({0xC8});                                //   INY
  emit({0xD0, 0x00});                          //   BNE fill (offset patched)
  code.back() = static_cast<u8>(fill - (code.size()));
  emit({0xCA});                                //   DEX
  emit({0xD0, 0x00});                          //   BNE fill (offset patched)
  code.back() = static_cast<u8>(fill - (code.size()));

  emit({0xA9, 0x08, 0x8D, 0x01, 0x20});        // LDA #$08; STA $2001 (BG on)

  const u16 spin = static_cast<u16>(0x8000 + code.size());
  emit({0x4C, static_cast<u8>(spin & 0xFF),
        static_cast<u8>((spin >> 8) & 0xFF)});  // JMP spin

  // Assemble 16KB PRG with vectors at $FFFA/$FFFC/$FFFE (offsets $3FFA..).
  std::vector<u8> prg(0x4000, 0x00);
  for (std::size_t i = 0; i < code.size(); ++i) prg[i] = code[i];
  prg[0x3FFC] = 0x00;  // RESET low
  prg[0x3FFD] = 0x80;  // RESET high -> $8000

  // 8KB CHR: tile 1 = solid color 3 (both bitplanes set for all 8 rows).
  std::vector<u8> chr(0x2000, 0x00);
  for (int row = 0; row < 8; ++row) {
    chr[16 + row] = 0xFF;      // low plane
    chr[16 + 8 + row] = 0xFF;  // high plane
  }

  std::vector<u8> rom = {'N', 'E', 'S', 0x1A, 1, 1, 0x00, 0x00,
                         0,   0,   0,   0,    0, 0, 0,    0};
  rom.insert(rom.end(), prg.begin(), prg.end());
  rom.insert(rom.end(), chr.begin(), chr.end());
  return rom;
}

}  // namespace

// reset_to_vector() loads PC from $FFFC/$FFFD instead of leaving it at $FFFC.
TEST(RomBootTest, ResetLoadsPcFromResetVector) {
  Bus bus;
  Debugger dbg{bus.get_cpu(), bus};

  int status = -1;
  auto cart = Cartridge::from_ines(make_checkerboard_rom(), status);
  ASSERT_EQ(status, 0);
  ASSERT_NE(cart, nullptr);

  bus.insert_cartridge(cart);
  bus.reset();
  dbg.reset_to_vector();

  EXPECT_EQ(dbg.get_register_pc(), 0x8000);
}

// Booting and running a few frames renders a non-blank background: the
// alternating tiles must produce at least the backdrop + the solid color.
TEST(RomBootTest, RendersBackgroundAfterBoot) {
  Bus bus;
  Debugger dbg{bus.get_cpu(), bus};

  int status = -1;
  auto cart = Cartridge::from_ines(make_checkerboard_rom(), status);
  ASSERT_EQ(status, 0);

  bus.insert_cartridge(cart);
  bus.reset();
  dbg.reset_to_vector();

  // Frame 1 runs the setup (fill nametable, enable BG); later frames render the
  // now-stable nametable from the top. Run a few so the screen settles.
  for (int i = 0; i < 3; ++i) dbg.run_frame();

  const u32* fb = bus.get_ppu().framebuffer();
  std::set<u32> colors;
  for (int i = 0; i < 256 * 240; ++i) colors.insert(fb[i]);

  // The solid tile (color index 3 -> palette $30) must appear somewhere.
  const u32 white = palette_rgba(0x30);
  bool has_white = false;
  for (int i = 0; i < 256 * 240; ++i) {
    if (fb[i] == white) {
      has_white = true;
      break;
    }
  }

  EXPECT_GE(colors.size(), 2u) << "framebuffer is a single flat color";
  EXPECT_TRUE(has_white) << "solid tile color did not render";
}
