#include <gtest/gtest.h>
#include "bus.h"
#include "cartridge.h"
#include "mapper_zero.h"

using namespace nes;

// A cartridge that exposes a programmable NMI vector at $FFFA/$FFFB and
// otherwise behaves like an empty NROM. CPU reads/writes outside RAM route
// here through the Bus.
class NmiVectorCartridge : public Cartridge {
 public:
  u8 nmi_lo = 0x00;
  u8 nmi_hi = 0x00;

  NmiVectorCartridge()
    : Cartridge("") {
    _prg_memory.resize(0x8000);
    _chr_memory.resize(0x2000);
    _mapper = std::make_shared<MapperZero>(_prg_banks, _chr_banks);
  }

  bool cpu_read(u16 address, u8& data) const override {
    if (address == 0xFFFA) {
      data = nmi_lo;
      return true;
    }
    if (address == 0xFFFB) {
      data = nmi_hi;
      return true;
    }
    if (address >= 0x8000) {
      data = 0x00;
      return true;
    }
    return false;
  }

  bool cpu_write(u16 address, u8 value) override {
    return address >= 0x8000;
  }
};

class CpuNmiTest : public ::testing::Test {
 protected:
  void SetUp() override {
    cart = std::make_shared<NmiVectorCartridge>();
    bus.insert_cartridge(cart);
    bus.reset();
  }

  Bus bus;
  std::shared_ptr<NmiVectorCartridge> cart;
  CPU& cpu = bus.get_cpu();
};

TEST_F(CpuNmiTest, JumpsToNmiVector) {
  cart->nmi_lo = 0x34;
  cart->nmi_hi = 0x12;
  cpu.set_pc(0xC000);
  cpu.set_sp(0xFD);
  cpu.set_status((u8)Flag::UNUSED);

  cpu.trigger_nmi();

  EXPECT_EQ(cpu.get_pc(), 0x1234);
}

TEST_F(CpuNmiTest, PushesPcHiThenLoThenStatus) {
  cart->nmi_lo = 0x00;
  cart->nmi_hi = 0x80;
  cpu.set_pc(0xBEEF);
  cpu.set_sp(0xFD);
  cpu.set_status((u8)Flag::UNUSED | (u8)Flag::CARRY);

  cpu.trigger_nmi();

  // Stack grows downward from $0100+SP. With SP starting at 0xFD:
  //   $01FD = PC high (0xBE)
  //   $01FC = PC low  (0xEF)
  //   $01FB = status pushed
  EXPECT_EQ(cpu.read_byte(0x01FD), 0xBE);
  EXPECT_EQ(cpu.read_byte(0x01FC), 0xEF);

  u8 pushed = cpu.read_byte(0x01FB);
  // UNUSED set, BREAK clear in the pushed status.
  EXPECT_TRUE(pushed & (u8)Flag::UNUSED);
  EXPECT_FALSE(pushed & (u8)Flag::BREAK);
  // Pre-existing CARRY preserved.
  EXPECT_TRUE(pushed & (u8)Flag::CARRY);

  // SP decremented by 3.
  EXPECT_EQ(cpu.get_sp(), (u8)0xFA);
}

TEST_F(CpuNmiTest, SetsInterruptDisableAndAccountsCycles) {
  cart->nmi_lo = 0x00;
  cart->nmi_hi = 0x90;
  cpu.set_pc(0xC123);
  cpu.set_sp(0xFD);
  cpu.set_status((u8)Flag::UNUSED);

  cpu.trigger_nmi();

  EXPECT_TRUE(cpu.get_flag(Flag::INTERRUPT_DISABLE));
  // 7 cycles consumed (matches BRK timing). After trigger the CPU should
  // report 7 remaining cycles for the interrupt sequence.
  EXPECT_EQ(cpu.get_remaining_cycles(), 7);
}
