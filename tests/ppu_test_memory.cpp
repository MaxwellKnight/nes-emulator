#include <gtest/gtest.h>
#include <memory>
#include "cartridge.h"
#include "ppu.h"

using namespace nes;

namespace {
// Build a minimal NROM iNES image in memory: 1x16KB PRG, 1x8KB CHR.
// flags6 bit0 selects mirroring (0 = horizontal, 1 = vertical).
std::vector<u8> make_rom(bool vertical) {
  std::vector<u8> bytes(16, 0);
  bytes[0] = 'N';
  bytes[1] = 'E';
  bytes[2] = 'S';
  bytes[3] = 0x1A;
  bytes[4] = 1;                      // 1 x 16KB PRG
  bytes[5] = 1;                      // 1 x 8KB CHR
  bytes[6] = vertical ? 0x01 : 0x00; // mirroring select; mapper low nibble 0
  bytes[7] = 0x00;                   // mapper high nibble 0
  bytes.resize(16 + 16384 + 8192, 0);
  return bytes;
}

std::shared_ptr<Cartridge> make_cart(bool vertical) {
  int status = -1;
  auto cart = Cartridge::from_ines(make_rom(vertical), status);
  EXPECT_EQ(status, 0);
  return cart;
}
}  // namespace

class PPUTestMemory : public ::testing::Test {
 protected:
  PPU ppu;
  void SetUp() override { ppu.reset(); }
};

// Pattern table reads/writes route to cartridge CHR (CHR-RAM writable).
TEST_F(PPUTestMemory, PatternTableRoutesToChr) {
  auto cart = make_cart(false);
  ppu.insert_cartridge(cart);
  // Seed CHR directly through the cartridge so ppu_read picks it up.
  cart->_chr_memory[0x0000] = 0xAB;
  cart->_chr_memory[0x1FFF] = 0xCD;
  EXPECT_EQ(ppu.ppu_read(0x0000), 0xAB);
  EXPECT_EQ(ppu.ppu_read(0x1FFF), 0xCD);
}

TEST_F(PPUTestMemory, PpuReadNoCartridgeReturnsZero) {
  EXPECT_EQ(ppu.ppu_read(0x0000), 0x00);
  EXPECT_EQ(ppu.ppu_read(0x1234), 0x00);
}

// Horizontal mirroring: table = (addr>>11)&1.
// $2000 and $2400 share one physical table; $2800 and $2C00 share the other.
TEST_F(PPUTestMemory, HorizontalMirroring) {
  auto cart = make_cart(false);  // horizontal
  ppu.insert_cartridge(cart);
  ppu.ppu_write(0x2000, 0x11);
  ppu.ppu_write(0x2800, 0x22);
  EXPECT_EQ(ppu.ppu_read(0x2400), 0x11);  // $2400 mirrors $2000
  EXPECT_EQ(ppu.ppu_read(0x2C00), 0x22);  // $2C00 mirrors $2800
  EXPECT_EQ(ppu.ppu_read(0x2000), 0x11);
  EXPECT_EQ(ppu.ppu_read(0x2800), 0x22);
}

// Vertical mirroring: table = (addr>>10)&1.
// $2000 and $2800 share; $2400 and $2C00 share.
TEST_F(PPUTestMemory, VerticalMirroring) {
  auto cart = make_cart(true);  // vertical
  ppu.insert_cartridge(cart);
  ppu.ppu_write(0x2000, 0x33);
  ppu.ppu_write(0x2400, 0x44);
  EXPECT_EQ(ppu.ppu_read(0x2800), 0x33);  // $2800 mirrors $2000
  EXPECT_EQ(ppu.ppu_read(0x2C00), 0x44);  // $2C00 mirrors $2400
}

// $3000-$3EFF mirrors $2000-$2EFF.
TEST_F(PPUTestMemory, NametableThreeKMirror) {
  auto cart = make_cart(true);
  ppu.insert_cartridge(cart);
  ppu.ppu_write(0x2123, 0x55);
  EXPECT_EQ(ppu.ppu_read(0x3123), 0x55);
}

// Palette mirroring: $3F10/$14/$18/$1C mirror $3F00/$04/$08/$0C.
TEST_F(PPUTestMemory, PaletteMirroring) {
  ppu.ppu_write(0x3F00, 0x12);
  EXPECT_EQ(ppu.ppu_read(0x3F10), 0x12);
  ppu.ppu_write(0x3F14, 0x34);
  EXPECT_EQ(ppu.ppu_read(0x3F04), 0x34);
  ppu.ppu_write(0x3F18, 0x07);
  EXPECT_EQ(ppu.ppu_read(0x3F08), 0x07);
  ppu.ppu_write(0x3F1C, 0x09);
  EXPECT_EQ(ppu.ppu_read(0x3F0C), 0x09);
}

// Greyscale (PPUMASK bit0) masks palette reads to &0x30.
TEST_F(PPUTestMemory, PaletteGreyscaleMask) {
  ppu.ppu_write(0x3F05, 0x3F);
  EXPECT_EQ(ppu.ppu_read(0x3F05), 0x3F);  // greyscale off
  ppu.cpu_write(1, 0x01);                 // PPUMASK greyscale on
  EXPECT_EQ(ppu.ppu_read(0x3F05), 0x30);  // 0x3F & 0x30
}
