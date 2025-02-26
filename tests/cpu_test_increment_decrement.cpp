#include "cpu_test_base.h"

class CPUIncrementDecrement : public CPUTestBase {};

TEST_F(CPUIncrementDecrement, INX_Basic) {
  // Load initial value into X
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x41);
  execute_cycles(2);

  // Increment X
  bus.write(0xFFFE, (nes::u8)nes::Opcode::INX_IMP);
  execute_cycles(2);  // INX takes 2 cycles

  // Verify X was incremented
  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, INX_Zero) {
  // Load value that will become zero when incremented
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0xFF);
  execute_cycles(2);

  // Increment X
  bus.write(0xFFFE, (nes::u8)nes::Opcode::INX_IMP);
  execute_cycles(2);  // INX takes 2 cycles

  // Verify X wrapped to zero
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, INX_Negative) {
  // Load value that will become negative when incremented
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x7F);
  execute_cycles(2);

  // Increment X
  bus.write(0xFFFE, (nes::u8)nes::Opcode::INX_IMP);
  execute_cycles(2);  // INX takes 2 cycles

  // Verify X became negative
  EXPECT_EQ(cpu.get_x(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, INY_Basic) {
  // Load initial value into Y
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x41);
  execute_cycles(2);

  // Increment Y
  bus.write(0xFFFE, (nes::u8)nes::Opcode::INY_IMP);
  execute_cycles(2);  // INY takes 2 cycles

  // Verify Y was incremented
  EXPECT_EQ(cpu.get_y(), 0x42);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, INY_Zero) {
  // Load value that will become zero when incremented
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0xFF);
  execute_cycles(2);

  // Increment Y
  bus.write(0xFFFE, (nes::u8)nes::Opcode::INY_IMP);
  execute_cycles(2);  // INY takes 2 cycles

  // Verify Y wrapped to zero
  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, INY_Negative) {
  // Load value that will become negative when incremented
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x7F);
  execute_cycles(2);

  // Increment Y
  bus.write(0xFFFE, (nes::u8)nes::Opcode::INY_IMP);
  execute_cycles(2);  // INY takes 2 cycles

  // Verify Y became negative
  EXPECT_EQ(cpu.get_y(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

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

TEST_F(CPUIncrementDecrement, DEC_ZPG_Basic) {
  // Set up a zero page address with an initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::DEC_ZPG);
  bus.write(0xFFFD, 0x10);  // Zero page address
  bus.write(0x0010, 0x42);  // Initial value at zero page address
  execute_cycles(5);        // DEC_ZPG takes 5 cycles

  // Verify the value was decremented
  EXPECT_EQ(bus.read(0x0010), 0x41);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEC_ZPG_Zero) {
  // Decrement a value that will become zero
  bus.write(0xFFFC, (nes::u8)nes::Opcode::DEC_ZPG);
  bus.write(0xFFFD, 0x10);  // Zero page address
  bus.write(0x0010, 0x01);  // Initial value that will become 0
  execute_cycles(5);        // DEC_ZPG takes 5 cycles

  // Verify the value became zero
  EXPECT_EQ(bus.read(0x0010), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEC_ZPG_Negative) {
  // Decrement a value that will become negative
  bus.write(0xFFFC, (nes::u8)nes::Opcode::DEC_ZPG);
  bus.write(0xFFFD, 0x10);  // Zero page address
  bus.write(0x0010, 0x00);  // Initial value that will become 0xFF (negative)
  execute_cycles(5);        // DEC_ZPG takes 5 cycles

  // Verify the value became 0xFF
  EXPECT_EQ(bus.read(0x0010), 0xFF);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEC_ZPX_Basic) {
  // Set X register to offset the zero page address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x05);
  execute_cycles(2);

  // Decrement value using zero page X-indexed addressing
  bus.write(0xFFFE, (nes::u8)nes::Opcode::DEC_ZPX);
  bus.write(0xFFFF, 0x10);  // Base zero page address
  bus.write(0x0015, 0x42);  // Value at 0x10 + X (0x05)
  execute_cycles(6);        // DEC_ZPX takes 6 cycles

  // Verify the value was decremented
  EXPECT_EQ(bus.read(0x0015), 0x41);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEC_ABS_Basic) {
  // Decrement value using absolute addressing
  bus.write(0xFFFC, (nes::u8)nes::Opcode::DEC_ABS);
  bus.write(0xFFFD, 0x34);  // Low byte of address
  bus.write(0xFFFE, 0x12);  // High byte of address
  bus.write(0x1234, 0x42);  // Value at absolute address
  execute_cycles(6);        // DEC_ABS takes 6 cycles

  // Verify the value was decremented
  EXPECT_EQ(bus.read(0x1234), 0x41);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEC_ABX_Basic) {
  // Set X register to offset the absolute address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x05);
  execute_cycles(2);

  // Decrement value using absolute X-indexed addressing
  bus.write(0xFFFE, (nes::u8)nes::Opcode::DEC_ABX);
  bus.write(0xFFFF, 0x30);  // Low byte of base address
  bus.write(0x0000, 0x12);  // High byte of base address
  bus.write(0x1235, 0x42);  // Value at 0x1230 + X (0x05)
  execute_cycles(7);        // DEC_ABX takes 7 cycles

  // Verify the value was decremented
  EXPECT_EQ(bus.read(0x1235), 0x41);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEX_Basic) {
  // Load initial value into X
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  // Decrement X
  bus.write(0xFFFE, (nes::u8)nes::Opcode::DEX_IMP);
  execute_cycles(2);  // DEX takes 2 cycles

  // Verify X was decremented
  EXPECT_EQ(cpu.get_x(), 0x41);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEX_Zero) {
  // Load value that will become zero when decremented
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x01);
  execute_cycles(2);

  // Decrement X
  bus.write(0xFFFE, (nes::u8)nes::Opcode::DEX_IMP);
  execute_cycles(2);  // DEX takes 2 cycles

  // Verify X wrapped to zero
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEX_Negative) {
  // Load value that will become negative when decremented
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2);

  // Decrement X
  bus.write(0xFFFE, (nes::u8)nes::Opcode::DEX_IMP);
  execute_cycles(2);  // DEX takes 2 cycles

  // Verify X became 0xFF (negative)
  EXPECT_EQ(cpu.get_x(), 0xFF);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEY_Basic) {
  // Load initial value into Y
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  // Decrement Y
  bus.write(0xFFFE, (nes::u8)nes::Opcode::DEY_IMP);
  execute_cycles(2);  // DEY takes 2 cycles

  // Verify Y was decremented
  EXPECT_EQ(cpu.get_y(), 0x41);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEY_Zero) {
  // Load value that will become zero when decremented
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x01);
  execute_cycles(2);

  // Decrement Y
  bus.write(0xFFFE, (nes::u8)nes::Opcode::DEY_IMP);
  execute_cycles(2);  // DEY takes 2 cycles

  // Verify Y wrapped to zero
  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUIncrementDecrement, DEY_Negative) {
  // Load value that will become negative when decremented
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2);

  // Decrement Y
  bus.write(0xFFFE, (nes::u8)nes::Opcode::DEY_IMP);
  execute_cycles(2);  // DEY takes 2 cycles

  // Verify Y became 0xFF (negative)
  EXPECT_EQ(cpu.get_y(), 0xFF);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}
