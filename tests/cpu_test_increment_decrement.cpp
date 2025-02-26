#include "cpu_test_base.h"

class CPUIncrementDecrement : public CPUTestBase {};

TEST_F(CPUIncrementDecrement, INC_ZPG_Basic) {
  // Set up a zero page address with an initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::INC_ZPG);
  bus.write(0xFFFD, 0x10);  // Zero page address
  bus.write(0x0010, 0x42);  // Initial value at zero page address
  execute_cycles(5);        // INC_ZPG takes 5 cycles

  // Verify the value was incremented
  EXPECT_EQ(bus.read(0x0010), 0x43);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, INC_ZPG_Zero) {
  // Increment a value that will become zero
  bus.write(0xFFFC, (nes::u8)nes::Opcode::INC_ZPG);
  bus.write(0xFFFD, 0x10);  // Zero page address
  bus.write(0x0010, 0xFF);  // Initial value that will wrap to 0
  execute_cycles(5);        // INC_ZPG takes 5 cycles

  // Verify the value wrapped to zero
  EXPECT_EQ(bus.read(0x0010), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, INC_ZPG_Negative) {
  // Increment a value to become negative
  bus.write(0xFFFC, (nes::u8)nes::Opcode::INC_ZPG);
  bus.write(0xFFFD, 0x10);  // Zero page address
  bus.write(0x0010, 0x7F);  // Value that will become negative when incremented
  execute_cycles(5);        // INC_ZPG takes 5 cycles

  // Verify the value and negative flag
  EXPECT_EQ(bus.read(0x0010), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, INC_ZPX_Basic) {
  // Set X register to offset the zero page address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x05);
  execute_cycles(2);

  // Increment value using zero page X-indexed addressing
  bus.write(0xFFFE, (nes::u8)nes::Opcode::INC_ZPX);
  bus.write(0xFFFF, 0x10);  // Base zero page address
  bus.write(0x0015, 0x42);  // Value at 0x10 + X (0x05)
  execute_cycles(6);        // INC_ZPX takes 6 cycles

  // Verify the value was incremented
  EXPECT_EQ(bus.read(0x0015), 0x43);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, INC_ABS_Basic) {
  // Increment value using absolute addressing
  bus.write(0xFFFC, (nes::u8)nes::Opcode::INC_ABS);
  bus.write(0xFFFD, 0x34);  // Low byte of address
  bus.write(0xFFFE, 0x12);  // High byte of address
  bus.write(0x1234, 0x42);  // Value at absolute address
  execute_cycles(6);        // INC_ABS takes 6 cycles

  // Verify the value was incremented
  EXPECT_EQ(bus.read(0x1234), 0x43);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, INC_ABX_Basic) {
  // Set X register to offset the absolute address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x05);
  execute_cycles(2);

  // Increment value using absolute X-indexed addressing
  bus.write(0xFFFE, (nes::u8)nes::Opcode::INC_ABX);
  bus.write(0xFFFF, 0x30);  // Low byte of base address
  bus.write(0x0000, 0x12);  // High byte of base address
  bus.write(0x1235, 0x42);  // Value at 0x1230 + X (0x05)
  execute_cycles(7);        // INC_ABX takes 7 cycles

  // Verify the value was incremented
  EXPECT_EQ(bus.read(0x1235), 0x43);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}
