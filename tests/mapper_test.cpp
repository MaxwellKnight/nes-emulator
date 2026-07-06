#include <gtest/gtest.h>
#include <vector>
#include "cartridge.h"

using namespace nes;

namespace {
// Build an iNES image. PRG bytes are filled with their 8KB-bank index and CHR
// bytes with their 1KB-bank index, so a read reveals which bank is mapped.
std::vector<u8> make_rom(u8 prg16k, u8 chr8k, u8 mapper, u8 flags6_extra = 0) {
  std::vector<u8> b(16, 0);
  b[0] = 'N'; b[1] = 'E'; b[2] = 'S'; b[3] = 0x1A;
  b[4] = prg16k;
  b[5] = chr8k;
  b[6] = ((mapper & 0x0F) << 4) | flags6_extra;
  b[7] = mapper & 0xF0;
  for (size_t i = 0; i < (size_t)prg16k * 16384; i++) b.push_back((i / 0x2000) & 0xFF);
  for (size_t i = 0; i < (size_t)chr8k * 8192; i++) b.push_back((i / 0x0400) & 0xFF);
  return b;
}

std::shared_ptr<Cartridge> load(const std::vector<u8>& rom) {
  int st = -1;
  auto c = Cartridge::from_ines(rom, st);
  EXPECT_EQ(st, 0);
  return c;
}

u8 cpu(Cartridge& c, u16 a) { u8 d = 0; c.cpu_read(a, d); return d; }
u8 ppu(Cartridge& c, u16 a) { u8 d = 0; c.ppu_read(a, d); return d; }

// MMC1 loads a register via five serial writes (LSB first).
void mmc1_load(Cartridge& c, u16 addr, u8 value) {
  for (int i = 0; i < 5; i++) c.cpu_write(addr, (value >> i) & 1);
}
}  // namespace

TEST(MapperTest, UxROMPrgBanking) {
  auto c = load(make_rom(/*prg16k*/ 8, /*chr8k*/ 0, /*mapper*/ 2));
  // Default bank 0 at $8000; last 16KB bank fixed at $C000 (8KB index 14).
  EXPECT_EQ(cpu(*c, 0x8000), 0);
  EXPECT_EQ(cpu(*c, 0xC000), 14);
  c->cpu_write(0x8000, 3);  // select 16KB bank 3 -> 8KB index 6
  EXPECT_EQ(cpu(*c, 0x8000), 6);
  EXPECT_EQ(cpu(*c, 0xC000), 14);  // last bank unchanged
}

TEST(MapperTest, CNROMChrBanking) {
  auto c = load(make_rom(/*prg16k*/ 2, /*chr8k*/ 4, /*mapper*/ 3));
  EXPECT_EQ(ppu(*c, 0x0000), 0);   // CHR bank 0 -> 1KB index 0
  c->cpu_write(0x8000, 2);          // select 8KB CHR bank 2
  EXPECT_EQ(ppu(*c, 0x0000), 16);  // bank 2 -> 1KB index 16
}

TEST(MapperTest, MMC1PrgBankingAndMirror) {
  auto c = load(make_rom(/*prg16k*/ 4, /*chr8k*/ 1, /*mapper*/ 1));
  // Power-on PRG mode 3: $8000 switchable (bank 0), $C000 fixed last (bank 3 -> idx 6).
  EXPECT_EQ(cpu(*c, 0x8000), 0);
  EXPECT_EQ(cpu(*c, 0xC000), 6);
  mmc1_load(*c, 0xE000, 2);  // PRG reg = 16KB bank 2 -> 8KB index 4
  EXPECT_EQ(cpu(*c, 0x8000), 4);
  EXPECT_EQ(cpu(*c, 0xC000), 6);

  // Control reg mirroring: value 2 -> vertical, 3 -> horizontal.
  mmc1_load(*c, 0x8000, 0x02);
  EXPECT_EQ(c->mirror_mode(), Cartridge::MirrorMode::VERTICAL);
  mmc1_load(*c, 0x8000, 0x03);
  EXPECT_EQ(c->mirror_mode(), Cartridge::MirrorMode::HORIZONTAL);
}

TEST(MapperTest, MMC3BankingAndMirror) {
  auto c = load(make_rom(/*prg16k*/ 4, /*chr8k*/ 2, /*mapper*/ 4));  // 8x8KB PRG
  // PRG mode 0: $8000=R6, $A000=R7, $C000=second-last(6), $E000=last(7).
  c->cpu_write(0x8000, 6); c->cpu_write(0x8001, 2);  // R6 = 8KB bank 2
  c->cpu_write(0x8000, 7); c->cpu_write(0x8001, 3);  // R7 = 8KB bank 3
  EXPECT_EQ(cpu(*c, 0x8000), 2);
  EXPECT_EQ(cpu(*c, 0xA000), 3);
  EXPECT_EQ(cpu(*c, 0xC000), 6);
  EXPECT_EQ(cpu(*c, 0xE000), 7);

  // CHR mode 0: R2..R5 are 1KB banks at $1000-$1FFF.
  c->cpu_write(0x8000, 2); c->cpu_write(0x8001, 5);  // R2 = 1KB bank 5
  EXPECT_EQ(ppu(*c, 0x1000), 5);

  // Mirroring register: 0 -> vertical, 1 -> horizontal.
  c->cpu_write(0xA000, 0);
  EXPECT_EQ(c->mirror_mode(), Cartridge::MirrorMode::VERTICAL);
  c->cpu_write(0xA000, 1);
  EXPECT_EQ(c->mirror_mode(), Cartridge::MirrorMode::HORIZONTAL);
}

TEST(MapperTest, MMC3ScanlineIrq) {
  auto c = load(make_rom(/*prg16k*/ 4, /*chr8k*/ 2, /*mapper*/ 4));
  c->cpu_write(0xC000, 2);  // IRQ latch = 2
  c->cpu_write(0xC001, 0);  // request reload
  c->cpu_write(0xE001, 0);  // enable IRQ

  EXPECT_FALSE(c->irq_pending());
  c->signal_scanline();  // reload -> counter = 2
  EXPECT_FALSE(c->irq_pending());
  c->signal_scanline();  // counter -> 1
  EXPECT_FALSE(c->irq_pending());
  c->signal_scanline();  // counter -> 0 -> IRQ
  EXPECT_TRUE(c->irq_pending());

  c->irq_clear();
  EXPECT_FALSE(c->irq_pending());

  // Disabling acknowledges and prevents further IRQs.
  c->cpu_write(0xE000, 0);  // disable
  for (int i = 0; i < 10; i++) c->signal_scanline();
  EXPECT_FALSE(c->irq_pending());
}

TEST(MapperTest, UnsupportedMapperRejected) {
  int st = -1;
  auto c = Cartridge::from_ines(make_rom(2, 1, /*mapper*/ 5), st);  // MMC5
  EXPECT_EQ(st, 2);
  EXPECT_EQ(c, nullptr);
}
