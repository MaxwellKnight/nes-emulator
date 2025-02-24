#include "cpu_test_base.h"

class CPUShiftRotateTest : public CPUTestBase {};

// ASL Accumulator (0x0A)
TEST_F(CPUShiftRotateTest, asl_accumulator) {
  // Load value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFD, 0x45); // binary: 01000101
  execute_cycles(2);

  // Execute ASL on accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_ACC);
  execute_cycles(2); // ASL Accumulator takes 2 cycles

  EXPECT_EQ(cpu.get_accumulator(), 0x8A); // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, asl_accumulator_carry) {
  // Load value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFD, 0x80); // binary: 10000000
  execute_cycles(2);

  // Execute ASL on accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_ACC);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x00); // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// ASL Absolute (0x0E)
TEST_F(CPUShiftRotateTest, asl_absolute) {
  // Set up test value in memory (using lower address range 0x0000-0x1FFF)
  bus.write(0x0042, 0x45); // binary: 01000101

  // Execute ASL on absolute address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::ASL_ABS);
  bus.write(0xFFFD, 0x42);
  bus.write(0xFFFE, 0x00); // Address 0x0042
  execute_cycles(6);       // ASL Absolute takes 6 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0042), 0x8A); // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, asl_absolute_carry_and_zero) {
  // Set up test value in memory (using lower address range)
  bus.write(0x0042, 0x80); // binary: 10000000

  // Execute ASL on absolute address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::ASL_ABS);
  bus.write(0xFFFD, 0x42);
  bus.write(0xFFFE, 0x00); // Address 0x0042
  execute_cycles(6);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0042), 0x00); // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// ASL Absolute X-Indexed (0x1E)
TEST_F(CPUShiftRotateTest, asl_absolute_x) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x10); // X = 0x10
  execute_cycles(2);

  // Set up test value in memory (using lower address range)
  bus.write(0x0052, 0x45); // binary: 01000101 (at address 0x0042 + 0x10)

  // Execute ASL on absolute X-indexed address
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_XABS);
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x00); // Base address 0x0042, X = 0x10, final = 0x0052
  execute_cycles(7);       // ASL Absolute X takes 7 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0052), 0x8A); // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, asl_absolute_x_carry_and_zero) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x10); // X = 0x10
  execute_cycles(2);

  // Set up test value in memory (using lower address range)
  bus.write(0x0052, 0x80); // binary: 10000000 (at address 0x0042 + 0x10)

  // Execute ASL on absolute X-indexed address
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_XABS);
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x00); // Base address 0x0042, X = 0x10, final = 0x0052
  execute_cycles(7);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0052), 0x00); // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// ASL Zero Page (0x06)
TEST_F(CPUShiftRotateTest, asl_zero_page) {
  // Set up test value in zero page
  bus.write(0x42, 0x45); // binary: 01000101

  // Execute ASL on zero page address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::ASL_ZP);
  bus.write(0xFFFD, 0x42); // Zero page address 0x42
  execute_cycles(5);       // ASL Zero Page takes 5 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x42), 0x8A); // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, asl_zero_page_carry_and_zero) {
  // Set up test value in zero page
  bus.write(0x42, 0x80); // binary: 10000000

  // Execute ASL on zero page address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::ASL_ZP);
  bus.write(0xFFFD, 0x42); // Zero page address 0x42
  execute_cycles(5);

  EXPECT_EQ(bus.read(0x42), 0x00); // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// ASL Zero Page X-Indexed (0x16)
TEST_F(CPUShiftRotateTest, asl_zero_page_x) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x10); // X = 0x10
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x52, 0x45); // binary: 01000101 (at zero page address 0x42 + 0x10)

  // Execute ASL on zero page X-indexed address
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_XZP);
  bus.write(0xFFFF,
            0x42);   // Base zero page address 0x42, X = 0x10, final = 0x52
  execute_cycles(6); // ASL Zero Page X takes 6 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x52), 0x8A); // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, asl_zero_page_x_wrapping) {
  // Set X register to cause wrap-around
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0xFF); // X = 0xFF
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x41, 0x45); // binary: 01000101 (at wrapped address (0x42 + 0xFF) &
                         // 0xFF = 0x41)

  // Execute ASL on zero page X-indexed address with wrap
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_XZP);
  bus.write(0xFFFF, 0x42);
  execute_cycles(6);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x41), 0x8A); // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}
