#include <gtest/gtest.h>
#include "ppu.h"

using namespace nes;

class PPUTestInit : public ::testing::Test {
 protected:
  PPU ppu;
  void SetUp() override { ppu.reset(); }
};

TEST_F(PPUTestInit, ResetZerosRegisters) {
  EXPECT_EQ(ppu.reg_ctrl(), 0x00);
  EXPECT_EQ(ppu.reg_mask(), 0x00);
  EXPECT_EQ(ppu.reg_status(), 0x00);
  EXPECT_EQ(ppu.vram_addr(), 0x0000);
}

TEST_F(PPUTestInit, ResetZerosTiming) {
  EXPECT_EQ(ppu.frame_count(), 0u);
  EXPECT_EQ(ppu.scanline(), 0u);
  EXPECT_EQ(ppu.dot(), 0u);
}

TEST_F(PPUTestInit, FramebufferCleared) {
  const u32* fb = ppu.framebuffer();
  ASSERT_NE(fb, nullptr);
  for (int i = 0; i < 256 * 240; i++) {
    EXPECT_EQ(fb[i], 0u) << "framebuffer[" << i << "] not cleared";
  }
}

TEST_F(PPUTestInit, NametableAndPaletteAccessible) {
  EXPECT_NE(ppu.nametable_ram(), nullptr);
  EXPECT_NE(ppu.palette_ram(), nullptr);
}
