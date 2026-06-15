#include <gtest/gtest.h>
#include "ppu.h"

using namespace nes;

// Friend fixture: reaches the private loopy state + helpers declared as
// `friend class PPUTestLoopy;` in ppu.h.
class PPUTestLoopy : public ::testing::Test {
 protected:
  PPU ppu;
  void SetUp() override { ppu.reset(); }

  void set_v(u16 v) { ppu._v = v; }
  void set_t(u16 t) { ppu._t = t; }
  u16 get_v() const { return ppu._v; }

  void inc_coarse_x() { ppu.inc_coarse_x(); }
  void inc_y() { ppu.inc_y(); }
  void copy_x() { ppu.copy_x(); }
  void copy_y() { ppu.copy_y(); }
};

// coarse-X simply increments when < 31.
TEST_F(PPUTestLoopy, IncCoarseXIncrements) {
  set_v(0x0000);
  inc_coarse_x();
  EXPECT_EQ(get_v(), 0x0001);
}

// coarse-X == 31 wraps to 0 and toggles horizontal nametable bit 0x0400.
TEST_F(PPUTestLoopy, IncCoarseXWrapTogglesNametableBit) {
  set_v(0x001F);  // coarse-x = 31, nametable bit 0x0400 clear
  inc_coarse_x();
  EXPECT_EQ(get_v(), 0x0400);  // coarse-x -> 0, horizontal NT toggled

  set_v(0x041F);  // coarse-x = 31, nametable bit already set
  inc_coarse_x();
  EXPECT_EQ(get_v(), 0x0000);  // wraps and toggles back
}

// inc_y bumps fine-Y when fine-Y < 7.
TEST_F(PPUTestLoopy, IncYBumpsFineY) {
  set_v(0x0000);
  inc_y();
  EXPECT_EQ(get_v(), 0x1000);  // fine-Y 0 -> 1
}

// inc_y at fine-Y == 7: resets fine-Y, coarse-Y 0 -> 1.
TEST_F(PPUTestLoopy, IncYOverflowFineYToCoarseY) {
  set_v(0x7000);  // fine-Y = 7, coarse-Y = 0
  inc_y();
  EXPECT_EQ(get_v(), 0x0020);  // fine-Y -> 0, coarse-Y -> 1 (bit5)
}

// inc_y at coarse-Y == 29 wraps coarse-Y to 0 and toggles vertical NT bit 0x0800.
TEST_F(PPUTestLoopy, IncYCoarseY29WrapTogglesVerticalNametable) {
  // fine-Y = 7, coarse-Y = 29 (29<<5 = 0x03A0).
  set_v(0x7000 | 0x03A0);
  inc_y();
  EXPECT_EQ(get_v(), 0x0800);  // fine-Y->0, coarse-Y->0, NT vbit toggled
}

// inc_y at coarse-Y == 31 wraps coarse-Y to 0 WITHOUT toggling the NT bit.
TEST_F(PPUTestLoopy, IncYCoarseY31WrapsNoToggle) {
  // fine-Y = 7, coarse-Y = 31 (31<<5 = 0x03E0).
  set_v(0x7000 | 0x03E0);
  inc_y();
  EXPECT_EQ(get_v(), 0x0000);  // fine-Y->0, coarse-Y->0, no NT toggle
}

// copy_x copies horizontal bits (mask 0x041F) from t into v.
TEST_F(PPUTestLoopy, CopyXCopiesMask041F) {
  set_v(0xFFFF);
  set_t(0x0415);  // horizontal bits set
  copy_x();
  // v keeps its non-0x041F bits; the 0x041F bits come from t.
  EXPECT_EQ(get_v() & 0x041F, 0x0415);
  EXPECT_EQ(get_v() & ~0x041F, 0xFFFF & ~0x041F);
}

// copy_y copies vertical bits (mask 0x7BE0) from t into v.
TEST_F(PPUTestLoopy, CopyYCopiesMask7BE0) {
  set_v(0x0000);
  set_t(0x7BE0);
  copy_y();
  EXPECT_EQ(get_v() & 0x7BE0, 0x7BE0);
  EXPECT_EQ(get_v() & ~0x7BE0, 0x0000);
}
