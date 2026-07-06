#include <gtest/gtest.h>
#include "palette.h"
#include "types.h"

using namespace nes;

// Helpers to unpack the little-endian RGBA u32 (R is the low byte in memory).
static u8 R(u32 p) { return (u8)(p & 0xFF); }
static u8 G(u32 p) { return (u8)((p >> 8) & 0xFF); }
static u8 B(u32 p) { return (u8)((p >> 16) & 0xFF); }
static u8 A(u32 p) { return (u8)((p >> 24) & 0xFF); }

TEST(PaletteTest, AlphaIsAlwaysOpaque) {
  for (int i = 0; i < 64; i++) {
    EXPECT_EQ(A(NES_PALETTE[i]), 0xFF) << "entry " << i;
  }
}

TEST(PaletteTest, ByteOrderIsRgbaLittleEndian) {
  // Entry 0x21 is a bright blue (0x0078F8) in the standard 2C02 palette:
  // R=0x00, G=0x78, B=0xF8. The low byte of the packed u32 must be R.
  u32 p = NES_PALETTE[0x21];
  EXPECT_EQ(R(p), 0x00);
  EXPECT_EQ(G(p), 0x78);
  EXPECT_EQ(B(p), 0xF8);
  // Low byte (first byte in RGBA memory) is R.
  EXPECT_EQ((u8)(p & 0xFF), R(p));
}

TEST(PaletteTest, KnownEntries) {
  // 0x0F is black (the canonical NES "blacker than black").
  EXPECT_EQ(R(NES_PALETTE[0x0F]), 0x00);
  EXPECT_EQ(G(NES_PALETTE[0x0F]), 0x00);
  EXPECT_EQ(B(NES_PALETTE[0x0F]), 0x00);

  // 0x30 is white.
  EXPECT_EQ(R(NES_PALETTE[0x30]), 0xFF);
  EXPECT_EQ(G(NES_PALETTE[0x30]), 0xFF);
  EXPECT_EQ(B(NES_PALETTE[0x30]), 0xFF);

  // 0x20 is a light grey (0xFCFCFC) in the standard palette.
  EXPECT_EQ(R(NES_PALETTE[0x20]), 0xFC);
  EXPECT_EQ(G(NES_PALETTE[0x20]), 0xFC);
  EXPECT_EQ(B(NES_PALETTE[0x20]), 0xFC);
}

TEST(PaletteTest, PaletteRgbaMasksIndex) {
  // palette_rgba masks to 6 bits, so index and index|0x40 must match.
  EXPECT_EQ(palette_rgba(0x05), NES_PALETTE[0x05]);
  EXPECT_EQ(palette_rgba(0x45), NES_PALETTE[0x05]);
  EXPECT_EQ(palette_rgba(0xC0), NES_PALETTE[0x00]);
  EXPECT_EQ(palette_rgba(0x3F), NES_PALETTE[0x3F]);
}
