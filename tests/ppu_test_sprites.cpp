#include <gtest/gtest.h>
#include <array>
#include <memory>
#include "bus.h"
#include "palette.h"
#include "ppu.h"
#include "test_cartridge.h"

using nes::PPU;
using nes::u16;
using nes::u32;
using nes::u8;

class PPUSpriteTest : public ::testing::Test {
 protected:
  void SetUp() override {
    cart = std::make_shared<nes::MockCartridge>();
    ppu.insert_cartridge(cart);
    ppu.reset();
  }

  // Write a byte into PPU-internal memory (nametable / palette) via $2006/$2007.
  void ppu_poke(u16 addr, u8 value) {
    ppu.cpu_write(6, (addr >> 8) & 0x3F);
    ppu.cpu_write(6, addr & 0xFF);
    ppu.cpu_write(7, value);
  }

  void seed_chr(u16 chr_addr, u8 value) { cart->_chr_memory[chr_addr] = value; }

  // Build a solid color-N (1..3) tile in CHR table 0 at the given tile id.
  void seed_solid_tile(u8 tile_id, u8 color) {
    u16 base = tile_id * 16;
    for (int row = 0; row < 8; row++) {
      seed_chr(base + row, (color & 1) ? 0xFF : 0x00);       // low plane
      seed_chr(base + 8 + row, (color & 2) ? 0xFF : 0x00);   // high plane
    }
  }

  // Set one OAM entry (Y, tile, attr, X) via OAMADDR/OAMDATA.
  void set_sprite(int index, u8 y, u8 tile, u8 attr, u8 x) {
    ppu.cpu_write(3, index * 4);  // OAMADDR
    ppu.cpu_write(4, y);
    ppu.cpu_write(4, tile);
    ppu.cpu_write(4, attr);
    ppu.cpu_write(4, x);
  }

  // Clock the PPU through a given visible scanline up to dot 257 (render point).
  void render_line(int line) {
    while (!(ppu.scanline() == line && ppu.dot() >= 257)) ppu.clock();
  }

  u32 px(int line, int x) { return ppu.framebuffer()[line * 256 + x]; }

  PPU ppu;
  std::shared_ptr<nes::MockCartridge> cart;
};

// $2004 writes advance OAMADDR; $2004 reads return the current entry.
TEST_F(PPUSpriteTest, OamReadWriteThroughRegisters) {
  ppu.cpu_write(3, 0x00);
  ppu.cpu_write(4, 0xAB);
  ppu.cpu_write(4, 0xCD);
  ppu.cpu_write(3, 0x00);
  EXPECT_EQ(ppu.cpu_read(4), 0xAB);
  ppu.cpu_write(3, 0x01);
  EXPECT_EQ(ppu.cpu_read(4), 0xCD);
}

// A solid 8x8 sprite renders its palette color at the expected pixels. Sprites
// are delayed one scanline, so OAM Y = line - 1.
TEST_F(PPUSpriteTest, SpriteRendersWithDelay) {
  seed_solid_tile(1, /*color=*/1);
  ppu_poke(0x3F11, 0x30);  // sprite palette 0, color 1 -> white
  set_sprite(0, /*y=*/49, /*tile=*/1, /*attr=*/0x00, /*x=*/20);
  ppu.cpu_write(0, 0x00);  // 8x8, sprite table 0
  ppu.cpu_write(1, 0x14);  // show sprites (d4) + show sprites in left 8 (d2)

  render_line(50);  // sprite top scanline = Y + 1 = 50

  u32 white = nes::palette_rgba(0x30);
  for (int x = 20; x < 28; x++) EXPECT_EQ(px(50, x), white) << "x=" << x;
  // The pixel just left of the sprite is backdrop, not sprite color.
  EXPECT_NE(px(50, 19), white);
}

// Horizontal flip mirrors the sprite's columns.
TEST_F(PPUSpriteTest, HorizontalFlip) {
  // Tile 2: left half color 1, right half transparent (0b11110000 low plane).
  for (int row = 0; row < 8; row++) {
    seed_chr(2 * 16 + row, 0xF0);
    seed_chr(2 * 16 + 8 + row, 0x00);
  }
  ppu_poke(0x3F11, 0x30);
  set_sprite(0, 49, 2, 0x40 /*h-flip*/, 0);  // x=0 needs left-8 enabled
  ppu.cpu_write(1, 0x14);
  render_line(50);
  u32 white = nes::palette_rgba(0x30);
  // With h-flip, the opaque half moves to the right (x=4..7).
  EXPECT_EQ(px(50, 7), white);
  EXPECT_NE(px(50, 0), white);
}

// Sprite-0 hit: opaque sprite-0 pixel over opaque background sets PPUSTATUS d6.
TEST_F(PPUSpriteTest, SpriteZeroHit) {
  // Background: solid color-1 tile across the screen.
  seed_solid_tile(1, 1);
  ppu_poke(0x3F00, 0x0F);
  ppu_poke(0x3F01, 0x21);
  ppu_poke(0x3F11, 0x30);
  for (int i = 0; i < 32; i++) ppu_poke(0x2000 + i, 1);  // nametable row of tile 1
  ppu_poke(0x23C0, 0x00);
  // Sprite 0 solid, overlapping the background. Y=0 -> renders on line 1, where
  // coarse-Y is still 0 so the BG reads the seeded nametable row 0.
  seed_solid_tile(3, 1);  // tile 3 solid color 1
  set_sprite(0, 0, 3, 0x00, 40);
  ppu.cpu_write(0, 0x00);
  ppu.cpu_write(6, 0x00);  // reset v/t to 0 so BG reads nametable row 0
  ppu.cpu_write(6, 0x00);
  ppu.cpu_write(1, 0x1E);  // show bg + sprites, both incl. left 8

  EXPECT_EQ(ppu.reg_status() & 0x40, 0x00);  // not set yet
  render_line(1);
  EXPECT_EQ(ppu.reg_status() & 0x40, 0x40) << "sprite-0 hit should be set";
}

// Behind-background priority: a "behind" sprite is hidden where BG is opaque.
TEST_F(PPUSpriteTest, BehindBackgroundPriority) {
  seed_solid_tile(1, 1);          // bg tile, color 1
  seed_solid_tile(3, 1);          // sprite tile, color 1
  ppu_poke(0x3F00, 0x0F);
  ppu_poke(0x3F01, 0x21);         // bg color -> light blue
  ppu_poke(0x3F11, 0x30);         // sprite color -> white
  for (int i = 0; i < 32; i++) ppu_poke(0x2000 + i, 1);
  ppu_poke(0x23C0, 0x00);
  set_sprite(0, 0, 3, 0x20 /*behind*/, 40);  // Y=0 -> line 1 (coarse-Y 0)
  ppu.cpu_write(0, 0x00);
  ppu.cpu_write(6, 0x00);  // reset v/t to 0 so BG reads nametable row 0
  ppu.cpu_write(6, 0x00);
  ppu.cpu_write(1, 0x1E);
  render_line(1);
  // BG is opaque here, so the behind-sprite must NOT overwrite it.
  u32 bg = nes::palette_rgba(0x21);
  EXPECT_EQ(px(1, 40), bg);
}

// 8x16 sprites: tile bit0 selects the pattern table; top half uses tile&0xFE.
TEST_F(PPUSpriteTest, EightBySixteen) {
  // Put a solid color-1 tile at index 0 of pattern table 1 ($1000) top half.
  for (int row = 0; row < 8; row++) {
    seed_chr(0x1000 + 0 * 16 + row, 0xFF);  // tile 0 of table 1, low plane
    seed_chr(0x1000 + 0 * 16 + 8 + row, 0x00);
  }
  ppu_poke(0x3F11, 0x30);
  // tile=1 -> table 1 ($1000), base tile 0. Sprite top scanline 50.
  set_sprite(0, 49, 0x01, 0x00, 30);
  ppu.cpu_write(0, 0x20);  // 8x16 mode
  ppu.cpu_write(1, 0x14);
  render_line(50);
  u32 white = nes::palette_rgba(0x30);
  EXPECT_EQ(px(50, 30), white);
}

// A 9th in-range sprite on a scanline sets the overflow flag (PPUSTATUS d5).
TEST_F(PPUSpriteTest, OverflowFlag) {
  seed_solid_tile(1, 1);
  ppu_poke(0x3F11, 0x30);
  for (int s = 0; s < 9; s++) set_sprite(s, 49, 1, 0x00, s * 8);
  ppu.cpu_write(1, 0x14);
  render_line(50);
  EXPECT_EQ(ppu.reg_status() & 0x20, 0x20) << "overflow should be set with 9 sprites";
}

// OAM DMA ($4014) copies a CPU page into OAM and stalls the CPU.
TEST_F(PPUSpriteTest, OamDmaCopiesPage) {
  nes::Bus bus;
  auto c = std::make_shared<nes::MockCartridge>();
  bus.insert_cartridge(c);
  bus.reset();
  // Fill CPU RAM page $02 with a recognizable pattern.
  for (int i = 0; i < 256; i++) bus.cpu_write(0x0200 + i, static_cast<u8>(i ^ 0x5A));
  bus.cpu_write(0x2003, 0x00);  // OAMADDR = 0
  bus.cpu_write(0x4014, 0x02);  // trigger DMA from page $0200
  const u8* oam = bus.get_ppu().oam_data();
  for (int i = 0; i < 256; i++)
    EXPECT_EQ(oam[i], static_cast<u8>(i ^ 0x5A)) << "oam[" << i << "]";
}
