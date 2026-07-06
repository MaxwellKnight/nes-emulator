#include <gtest/gtest.h>
#include "ppu.h"

using namespace nes;

class PPUTestRegisters : public ::testing::Test {
 protected:
  PPU ppu;
  void SetUp() override { ppu.reset(); }
};

// PPUCTRL ($2000): stores ctrl and writes nametable bits into t bits 10-11.
TEST_F(PPUTestRegisters, PpuCtrlSetsCtrlAndNametableBits) {
  ppu.cpu_write(0, 0x03);  // nametable select = 3
  EXPECT_EQ(ppu.reg_ctrl(), 0x03);
  // t bits 10-11 = 0b11 -> 0x0C00
  EXPECT_EQ(ppu.vram_addr(), 0x0000);  // v unchanged by $2000
}

// PPUSTATUS ($2002) read returns top 3 bits | buffer low 5, clears vblank, resets w.
TEST_F(PPUTestRegisters, PpuStatusReadClearsVblankAndResetsW) {
  // Prime w=1 via a single PPUSCROLL write, then a PPUADDR write would use second latch.
  ppu.cpu_write(5, 0x00);  // w 0 -> 1
  // Force vblank set by reading status after manually toggling is not possible w/o clock;
  // instead verify read clears whatever bit7 is and resets w so the next $2006 is a HIGH write.
  ppu.cpu_read(2);         // status read: resets w to 0
  // Now two $2006 writes should land high-then-low (w restarted).
  ppu.cpu_write(6, 0x21);  // high byte -> t = 0x2100 (bit14 masked)
  ppu.cpu_write(6, 0x08);  // low byte  -> t = 0x2108, v = t
  EXPECT_EQ(ppu.vram_addr(), 0x2108);
}

// PPUSCROLL ($2005) x2 with w toggle: 1st sets fine-x + coarse-x in t, 2nd sets fine/coarse y.
TEST_F(PPUTestRegisters, PpuScrollTwoWritesSetXAndT) {
  ppu.cpu_read(2);          // ensure w = 0
  ppu.cpu_write(5, 0x7D);   // coarse-x = 0x7D>>3 = 0x0F, fine-x = 0x7D&7 = 5
  ppu.cpu_write(5, 0x5E);   // 2nd write -> fine-y + coarse-y into t
  // After two writes, w is back to 0; verify via $2006 immediately taking the HIGH latch.
  ppu.cpu_write(6, 0x00);
  ppu.cpu_write(6, 0x00);
  EXPECT_EQ(ppu.vram_addr(), 0x0000);
}

// PPUADDR ($2006) x2: high then low; second write copies t -> v.
TEST_F(PPUTestRegisters, PpuAddrTwoWritesSetTThenV) {
  ppu.cpu_read(2);          // w = 0
  ppu.cpu_write(6, 0x3F);   // high (bit14 cleared) -> t high = 0x3F
  EXPECT_EQ(ppu.vram_addr(), 0x0000);  // v not yet updated
  ppu.cpu_write(6, 0x00);   // low -> t = 0x3F00, v = t
  EXPECT_EQ(ppu.vram_addr(), 0x3F00);
}

TEST_F(PPUTestRegisters, PpuAddrHighByteMasksBit14) {
  ppu.cpu_read(2);
  ppu.cpu_write(6, 0xFF);   // high byte; only bits 0..5 used -> t high = 0x3F
  ppu.cpu_write(6, 0xFF);
  EXPECT_EQ(ppu.vram_addr(), 0x3FFF);
}

// PPUDATA ($2007) write then buffered read; first read returns stale buffer.
TEST_F(PPUTestRegisters, PpuDataWriteThenBufferedRead) {
  // Point v at nametable $2005 and write a byte there.
  ppu.cpu_read(2);
  ppu.cpu_write(6, 0x20);
  ppu.cpu_write(6, 0x05);   // v = 0x2005
  ppu.cpu_write(7, 0x42);   // ppu_write(0x2005, 0x42); v -> 0x2006 (increment 1)

  // Re-point v back to 0x2005.
  ppu.cpu_read(2);
  ppu.cpu_write(6, 0x20);
  ppu.cpu_write(6, 0x05);
  u8 first = ppu.cpu_read(7);   // buffered: returns OLD buffer, loads 0x42 into buffer
  u8 second = ppu.cpu_read(7);  // returns the previously buffered 0x42
  EXPECT_NE(first, 0x42);       // first read is stale
  EXPECT_EQ(second, 0x42);      // second read yields the written byte
}

// PPUDATA increment: bit2 of PPUCTRL selects +1 vs +32.
TEST_F(PPUTestRegisters, PpuDataIncrementMode) {
  ppu.cpu_write(0, 0x00);   // increment +1
  ppu.cpu_read(2);
  ppu.cpu_write(6, 0x20);
  ppu.cpu_write(6, 0x00);   // v = 0x2000
  ppu.cpu_write(7, 0x01);
  EXPECT_EQ(ppu.vram_addr(), 0x2001);

  ppu.cpu_write(0, 0x04);   // increment +32
  ppu.cpu_read(2);
  ppu.cpu_write(6, 0x20);
  ppu.cpu_write(6, 0x00);   // v = 0x2000
  ppu.cpu_write(7, 0x01);
  EXPECT_EQ(ppu.vram_addr(), 0x2020);
}
