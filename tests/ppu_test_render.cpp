#include <gtest/gtest.h>
#include <array>
#include <memory>
#include "palette.h"
#include "ppu.h"
#include "test_cartridge.h"

using nes::PPU;
using nes::u8;
using nes::u16;
using nes::u32;

class PPURenderTest : public ::testing::Test {
 protected:
  void SetUp() override {
    cart = std::make_shared<nes::MockCartridge>();
    ppu.insert_cartridge(cart);
    ppu.reset();
  }

  // Write a byte into PPU-internal memory (nametable / palette) via the
  // address latch ($2006) + data ($2007). NOTE: this only works for
  // $2000-$3FFF; CHR ($0000-$1FFF) on the mapper-0 MockCartridge is ROM and
  // cannot be written through $2007, so CHR is seeded via seed_chr() below.
  void ppu_poke(u16 addr, u8 value) {
    ppu.cpu_write(6, (addr >> 8) & 0x3F);  // PPUADDR high
    ppu.cpu_write(6, addr & 0xFF);         // PPUADDR low
    ppu.cpu_write(7, value);               // PPUDATA write (auto-increment)
  }

  // Seed a byte of CHR directly into the cartridge's CHR memory (the mapper-0
  // CHR window maps $0000-$1FFF straight through, so chr_addr == _chr_memory
  // index).
  void seed_chr(u16 chr_addr, u8 value) { cart->_chr_memory[chr_addr] = value; }

  // Build an 8x8 tile in CHR (pattern table 0) whose every pixel is color 1.
  // Plane 0 (low) all-ones, plane 1 (high) all-zeros => 2-bit pixel value 1.
  void seed_solid_tile_color1(u8 tile_id) {
    u16 base = tile_id * 16;
    for (int row = 0; row < 8; row++) {
      seed_chr(base + row, 0xFF);      // low plane
      seed_chr(base + 8 + row, 0x00);  // high plane
    }
  }

  PPU ppu;
  std::shared_ptr<nes::MockCartridge> cart;
};

// Backdrop: with BG rendering disabled, render_scanline fills the line with the
// backdrop color (_palette[0] mapped through the master palette).
TEST_F(PPURenderTest, BackdropFillWhenBgDisabled) {
  // Set backdrop palette entry $3F00 = 0x21 (light blue) while BG is enabled,
  // then disable BG so the renderer takes the backdrop path.
  ppu.cpu_write(1, 0x08);  // PPUMASK: show-bg ON (so $2007 writes do not glitch)
  ppu_poke(0x3F00, 0x21);
  ppu.cpu_write(1, 0x00);  // PPUMASK: BG OFF

  // Render scanline 0 by clocking the visible line through dot 257.
  for (int d = 0; d <= 257; d++) ppu.clock();

  const u32* fb = ppu.framebuffer();
  u32 expected = nes::palette_rgba(0x21);
  for (int x = 0; x < 256; x++) {
    EXPECT_EQ(fb[0 * 256 + x], expected) << "x=" << x;
  }
}

// A solid color-1 tile placed at nametable (0,0) with a palette renders the
// expected exact RGBA across the first 8 pixels of scanline 0.
TEST_F(PPURenderTest, SolidTileRendersExactPixels) {
  // Palette: backdrop $3F00 = 0x0F (black); palette 0 color 1 ($3F01) = 0x30 (white).
  ppu.cpu_write(1, 0x08);  // PPUMASK: show-bg ON for VRAM writes
  ppu_poke(0x3F00, 0x0F);
  ppu_poke(0x3F01, 0x30);

  // CHR tile 1 = solid color 1.
  seed_solid_tile_color1(1);

  // Nametable (0,0) top-left tile -> tile id 1.
  ppu_poke(0x2000, 0x01);
  // Attribute byte for top-left 32x32 region = 0 -> palette 0 for that quadrant.
  ppu_poke(0x23C0, 0x00);

  // PPUCTRL: BG pattern base $0000 (bit4=0). Scroll = 0.
  ppu.cpu_write(0, 0x00);
  // PPUSCROLL x=0, y=0
  ppu.cpu_write(5, 0x00);
  ppu.cpu_write(5, 0x00);
  // PPUADDR sets v=t after the second write; reset v/t to 0 via $2006.
  ppu.cpu_write(6, 0x00);
  ppu.cpu_write(6, 0x00);

  // BG enabled.
  ppu.cpu_write(1, 0x08);

  // Render scanline 0.
  for (int d = 0; d <= 257; d++) ppu.clock();

  const u32* fb = ppu.framebuffer();
  u32 white = nes::palette_rgba(0x30);
  // First 8 pixels (the solid tile, pixel value 1 -> palette index 1 -> white).
  for (int x = 0; x < 8; x++) {
    EXPECT_EQ(fb[0 * 256 + x], white) << "x=" << x;
  }
}

// Pixel value 0 always maps to the backdrop ($3F00), regardless of the tile's
// palette high bits.
TEST_F(PPURenderTest, PixelZeroIsBackdrop) {
  ppu.cpu_write(1, 0x08);
  ppu_poke(0x3F00, 0x16);  // backdrop = red
  ppu_poke(0x3F01, 0x30);  // palette 0 color 1 = white (unused here)

  // Tile 2: both planes all-zero -> every pixel value 0.
  u16 base = 2 * 16;
  for (int row = 0; row < 8; row++) {
    seed_chr(base + row, 0x00);
    seed_chr(base + 8 + row, 0x00);
  }
  ppu_poke(0x2000, 0x02);
  ppu_poke(0x23C0, 0x00);

  ppu.cpu_write(0, 0x00);
  ppu.cpu_write(6, 0x00);
  ppu.cpu_write(6, 0x00);
  ppu.cpu_write(1, 0x08);

  for (int d = 0; d <= 257; d++) ppu.clock();

  const u32* fb = ppu.framebuffer();
  u32 red = nes::palette_rgba(0x16);
  for (int x = 0; x < 8; x++) {
    EXPECT_EQ(fb[0 * 256 + x], red) << "x=" << x;
  }
}

// render_pattern_table renders a known CHR tile into the 128x128 RGBA buffer.
// Tile (0,0) in table 0 is solid color 1; using palette p, color 1 maps to
// _palette[(p<<2)|1].
TEST_F(PPURenderTest, RenderPatternTableSolidTile) {
  // Seed CHR tile 0 = solid color 1.
  for (int row = 0; row < 8; row++) {
    seed_chr(0 + row, 0xFF);      // low plane
    seed_chr(0 + 8 + row, 0x00);  // high plane
  }

  // Palette 1 (index 1<<2 = 4): color 1 -> $3F05 = 0x30 (white).
  ppu.cpu_write(1, 0x08);
  ppu_poke(0x3F05, 0x30);
  ppu.cpu_write(1, 0x00);

  std::array<u32, 128 * 128> out{};
  ppu.render_pattern_table(0, /*palette=*/1, out.data());

  u32 white = nes::palette_rgba(0x30);
  // The top-left 8x8 block (tile 0) should all be white.
  for (int y = 0; y < 8; y++) {
    for (int x = 0; x < 8; x++) {
      EXPECT_EQ(out[y * 128 + x], white) << "x=" << x << " y=" << y;
    }
  }
}
