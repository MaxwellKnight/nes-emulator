#include <gtest/gtest.h>
#include "bus.h"
#include "test_cartridge.h"

using namespace nes;

class BusPpuTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto cart = std::make_shared<MockCartridge>();
    bus.insert_cartridge(cart);
    bus.reset();
  }

  Bus bus;
};

// One Bus::clock() must tick the PPU exactly 3 dots.
TEST_F(BusPpuTest, ClockTicksPpuThreeDots) {
  PPU& ppu = bus.get_ppu();
  u16 start_dot = ppu.dot();
  u16 start_scanline = ppu.scanline();

  bus.clock();

  // After reset the PPU starts at dot 0, scanline 0, so 3 dots forward stays
  // on the same scanline (well away from the 341 wrap).
  EXPECT_EQ(ppu.scanline(), start_scanline);
  EXPECT_EQ(ppu.dot(), static_cast<u16>(start_dot + 3));
}

// When the PPU raises an NMI (VBlank with NMI enabled), the Bus must route it
// to CPU::trigger_nmi(), sending PC to the NMI vector at $FFFA/$FFFB.
TEST_F(BusPpuTest, ClockRoutesNmiToCpu) {
  CPU& cpu = bus.get_cpu();
  PPU& ppu = bus.get_ppu();

  // Install a distinct, non-zero NMI vector ($9000). $FFFA/$FFFB are in the
  // cartridge PRG range ($8000-$FFFF), so these writes land in PRG ROM and
  // read back. The CPU only ever executes the zero-filled PRG/RAM (opcode
  // 0x00 = BRK), so it can never reach $9000 on its own -- reaching $9000 is
  // therefore an unambiguous signal that the Bus loaded the NMI vector.
  bus.cpu_write(0xFFFA, 0x00);
  bus.cpu_write(0xFFFB, 0x90);
  const u16 nmi_vector = 0x9000;

  // Enable NMI generation: PPUCTRL ($2000) bit7. $2000 is NOT claimed by the
  // cartridge mapper, so this write reaches the PPU.
  bus.cpu_write(0x2000, 0x80);
  EXPECT_EQ(ppu.reg_ctrl() & 0x80, 0x80);

  // Clock the Bus until VBlank fires and the NMI redirects PC to $9000. A full
  // frame is 341*262 = 89342 PPU dots ~= 29781 Bus clocks; clock generously.
  bool jumped = false;
  for (int i = 0; i < 40000 && !jumped; i++) {
    bus.clock();
    if (cpu.get_pc() == nmi_vector) {
      jumped = true;
    }
  }

  EXPECT_TRUE(jumped) << "CPU PC never reached the NMI vector after VBlank";
  EXPECT_GE(ppu.scanline(), static_cast<u16>(241));
}

// Writing $4014 (OAMDMA, Phase 2) must be a safe no-op, not a crash.
TEST_F(BusPpuTest, OamDmaWriteIsSafeNoOp) {
  EXPECT_NO_THROW(bus.cpu_write(0x4014, 0x02));
}
