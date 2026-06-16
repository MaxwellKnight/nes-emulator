#include <gtest/gtest.h>
#include "ppu.h"

using nes::PPU;
using nes::u8;
using nes::u16;
using nes::u32;

class PPUTimingTest : public ::testing::Test {
 protected:
  void SetUp() override { ppu.reset(); }

  // Advance the PPU by n dots.
  void tick(int n) {
    for (int i = 0; i < n; i++) ppu.clock();
  }

  PPU ppu;
};

// 341 dots advances the scanline counter by exactly one and wraps the dot to 0.
TEST_F(PPUTimingTest, DotsAdvanceScanline) {
  EXPECT_EQ(ppu.scanline(), 0u);
  EXPECT_EQ(ppu.dot(), 0u);

  tick(340);
  EXPECT_EQ(ppu.scanline(), 0u);
  EXPECT_EQ(ppu.dot(), 340u);

  tick(1);  // dot 341 -> wrap
  EXPECT_EQ(ppu.scanline(), 1u);
  EXPECT_EQ(ppu.dot(), 0u);
}

// A full field (341 * 262 dots) increments the frame counter once and wraps
// scanline/dot back to the origin.
TEST_F(PPUTimingTest, FullFieldIncrementsFrameAndWraps) {
  EXPECT_EQ(ppu.frame_count(), 0u);

  tick(341 * 262);

  EXPECT_EQ(ppu.frame_count(), 1u);
  EXPECT_EQ(ppu.scanline(), 0u);
  EXPECT_EQ(ppu.dot(), 0u);
}

// VBlank (PPUSTATUS bit 7) is set at scanline 241, dot 1.
TEST_F(PPUTimingTest, VBlankSetAtScanline241Dot1) {
  // Advance to scanline 241, dot 0 (just before the set point).
  tick(341 * 241);
  EXPECT_EQ(ppu.scanline(), 241u);
  EXPECT_EQ(ppu.dot(), 0u);
  EXPECT_EQ(ppu.reg_status() & 0x80, 0x00);

  tick(1);  // dot 1
  EXPECT_EQ(ppu.dot(), 1u);
  EXPECT_EQ(ppu.reg_status() & 0x80, 0x80);
}

// With PPUCTRL bit 7 (NMI enable) set, entering VBlank latches an NMI that
// take_nmi() returns exactly once.
TEST_F(PPUTimingTest, NmiRaisedOnceWhenEnabled) {
  ppu.cpu_write(0, 0x80);  // PPUCTRL: NMI enable

  // Before VBlank there is no pending NMI.
  tick(341 * 241);
  EXPECT_FALSE(ppu.take_nmi());

  tick(1);  // scanline 241, dot 1 -> set vblank + raise NMI
  EXPECT_TRUE(ppu.take_nmi());   // consumed
  EXPECT_FALSE(ppu.take_nmi());  // cleared after consume
}

// With NMI disabled, entering VBlank still sets the flag but raises no NMI.
TEST_F(PPUTimingTest, NoNmiWhenDisabled) {
  ppu.cpu_write(0, 0x00);  // PPUCTRL: NMI disabled
  tick(341 * 241 + 1);     // scanline 241, dot 1
  EXPECT_EQ(ppu.reg_status() & 0x80, 0x80);
  EXPECT_FALSE(ppu.take_nmi());
}

// VBlank is cleared at the pre-render line (scanline 261), dot 1.
TEST_F(PPUTimingTest, VBlankClearedAtPreRenderDot1) {
  tick(341 * 241 + 1);  // set vblank at (241,1)
  EXPECT_EQ(ppu.reg_status() & 0x80, 0x80);

  // Advance to pre-render line (261) dot 1: from (241,1) that is
  // 20 full scanlines worth of dots (341 * 20) to reach (261,1).
  tick(341 * 20);  // now at (261,1)
  EXPECT_EQ(ppu.scanline(), 261u);
  EXPECT_EQ(ppu.dot(), 1u);
  EXPECT_EQ(ppu.reg_status() & 0x80, 0x00);
}
