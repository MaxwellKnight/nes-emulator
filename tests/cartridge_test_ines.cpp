#include <gtest/gtest.h>
#include <vector>
#include "cartridge.h"

using namespace nes;

// Builds a minimal iNES image: 16-byte header + prg_banks*16384 PRG +
// chr_banks*8192 CHR. flags6/flags7 are caller-controlled.
static std::vector<u8> make_ines(u8 prg_banks, u8 chr_banks, u8 flags6,
                                  u8 flags7, bool good_magic = true) {
  std::vector<u8> b(16, 0);
  if (good_magic) {
    b[0] = 'N';
    b[1] = 'E';
    b[2] = 'S';
    b[3] = 0x1A;
  } else {
    b[0] = 'X';
    b[1] = 'X';
    b[2] = 'X';
    b[3] = 0x00;
  }
  b[4] = prg_banks;
  b[5] = chr_banks;
  b[6] = flags6;
  b[7] = flags7;
  // Append PRG then CHR payloads (fill with recognizable bytes).
  for (int i = 0; i < prg_banks * 16384; i++) b.push_back(0xA5);
  for (int i = 0; i < chr_banks * 8192; i++) b.push_back(0x5A);
  return b;
}

TEST(CartridgeInesTest, ParsesValidHorizontalRom) {
  auto bytes = make_ines(/*prg*/ 1, /*chr*/ 1, /*flags6*/ 0x00, /*flags7*/ 0x00);
  int status = -1;
  auto cart = Cartridge::from_ines(bytes, status);
  ASSERT_EQ(status, 0);
  ASSERT_NE(cart, nullptr);
  EXPECT_EQ(cart->_prg_memory.size(), (size_t)16384);
  EXPECT_EQ(cart->_chr_memory.size(), (size_t)8192);
  EXPECT_EQ(cart->mirror_mode(), Cartridge::MirrorMode::HORIZONTAL);
  EXPECT_EQ(cart->_prg_memory[0], 0xA5);
  EXPECT_EQ(cart->_chr_memory[0], 0x5A);
}

TEST(CartridgeInesTest, ParsesVerticalMirroring) {
  auto bytes = make_ines(2, 1, /*flags6 bit0=1 -> vertical*/ 0x01, 0x00);
  int status = -1;
  auto cart = Cartridge::from_ines(bytes, status);
  ASSERT_EQ(status, 0);
  EXPECT_EQ(cart->_prg_memory.size(), (size_t)32768);
  EXPECT_EQ(cart->mirror_mode(), Cartridge::MirrorMode::VERTICAL);
}

TEST(CartridgeInesTest, BadMagicReturnsStatus1) {
  auto bytes = make_ines(1, 1, 0x00, 0x00, /*good_magic*/ false);
  int status = -1;
  auto cart = Cartridge::from_ines(bytes, status);
  EXPECT_EQ(status, 1);
  EXPECT_EQ(cart, nullptr);
}

TEST(CartridgeInesTest, NonZeroMapperReturnsStatus2) {
  // mapper = (flags7 & 0xF0) | (flags6 >> 4). Set flags7 high nibble = 1.
  auto bytes = make_ines(1, 1, /*flags6*/ 0x00, /*flags7*/ 0x10);
  int status = -1;
  auto cart = Cartridge::from_ines(bytes, status);
  EXPECT_EQ(status, 2);
  EXPECT_EQ(cart, nullptr);
}

TEST(CartridgeInesTest, FourScreenReturnsStatus3) {
  // flags6 bit3 = four-screen.
  auto bytes = make_ines(1, 1, /*flags6*/ 0x08, /*flags7*/ 0x00);
  int status = -1;
  auto cart = Cartridge::from_ines(bytes, status);
  EXPECT_EQ(status, 3);
  EXPECT_EQ(cart, nullptr);
}

TEST(CartridgeInesTest, SkipsTrainerBeforePrg) {
  // flags6 bit2 = trainer present (512 bytes before PRG).
  std::vector<u8> b(16, 0);
  b[0] = 'N'; b[1] = 'E'; b[2] = 'S'; b[3] = 0x1A;
  b[4] = 1;            // 1 PRG bank
  b[5] = 1;            // 1 CHR bank
  b[6] = 0x04;         // trainer present
  b[7] = 0x00;
  for (int i = 0; i < 512; i++) b.push_back(0xEE);     // trainer
  for (int i = 0; i < 16384; i++) b.push_back(0xA5);   // PRG
  for (int i = 0; i < 8192; i++) b.push_back(0x5A);    // CHR

  int status = -1;
  auto cart = Cartridge::from_ines(b, status);
  ASSERT_EQ(status, 0);
  // PRG must start AFTER the trainer (so first byte is 0xA5, not 0xEE).
  EXPECT_EQ(cart->_prg_memory[0], 0xA5);
  EXPECT_EQ(cart->_chr_memory[0], 0x5A);
}

TEST(CartridgeInesTest, ChrRamAllocatedAndWritable) {
  // chr_banks == 0 -> allocate 8192 bytes of writable CHR-RAM.
  auto bytes = make_ines(1, /*chr*/ 0, 0x00, 0x00);
  int status = -1;
  auto cart = Cartridge::from_ines(bytes, status);
  ASSERT_EQ(status, 0);
  EXPECT_EQ(cart->_chr_memory.size(), (size_t)8192);

  // ppu_write into the pattern-table region must succeed and be readable.
  EXPECT_TRUE(cart->ppu_write(0x0001, 0x3C));
  u8 data = 0x00;
  EXPECT_TRUE(cart->ppu_read(0x0001, data));
  EXPECT_EQ(data, 0x3C);
}
