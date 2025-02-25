#include "cpu_test_base.h"

class CPULoadTest : public CPUTestBase {};

TEST_F(CPULoadTest, lda_immediate) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IMM));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2);  // LDA Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, lda_immediate_zero_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IMM));
  bus.write(0xFFFD, 0x00);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2);  // LDA Immediate takes 2 cycles
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, lda_immediate_negative_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IMM));
  bus.write(0xFFFD, 0x80);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2);  // LDA Immediate takes 2 cycles
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, lda_zero_page) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_ZPG));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(3);  // LDA Zero Page takes 3 cycles
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldx_immediate) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IMM));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2);  // LDX Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldx_immediate_zero_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IMM));
  bus.write(0xFFFD, 0x00);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2);  // LDX Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldx_immediate_negative_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IMM));
  bus.write(0xFFFD, 0x80);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2);  // LDX Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_x(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldx_zero_page) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_ZPG));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(3);  // LDX Zero Page takes 3 cycles
  EXPECT_EQ(cpu.get_x(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldy_immediate) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_IMM));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2);  // LDY Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_flag(nes::Flag::CARRY), false);
  EXPECT_EQ(cpu.get_flag(nes::Flag::OVERFLOW_), false);
}

TEST_F(CPULoadTest, ldy_immediate_zero_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_IMM));
  bus.write(0xFFFD, 0x00);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2);  // LDY Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldy_immediate_negtive_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_IMM));
  bus.write(0xFFFD, 0x80);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2);  // LDY Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldy_zero_page) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_ZPG));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(3);  // LDY Zero Page takes 3 cycles
  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldy_zero_page_x) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IMM));
  bus.write(0xFFFD, 0x02);
  execute_cycles(2);  // LDX_IMM takes 2 cycles

  // Then test LDY zero page X
  bus.write(0x44, 0x37);  // Target address: 0x42 + 0x02 = 0x44
  bus.write(0xFFFE, (nes::u8)(nes::Opcode::LDY_ZPX));
  bus.write(0xFFFF, 0x42);
  execute_cycles(4);  // LDY Zero Page X takes 4 cycles
  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldy_absolute) {
  bus.write(0x0242, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_ABS));
  bus.write(0xFFFD, 0x42);
  bus.write(0xFFFE, 0x02);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(4);  // LDY Absolute takes 4 cycles
  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldy_absolute_x) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IMM));
  bus.write(0xFFFD, 0x02);
  execute_cycles(2);  // LDX_IMM takes 2 cycles

  // Then test LDY absolute X
  bus.write(0x0244, 0x37);
  bus.write(0xFFFE, (nes::u8)(nes::Opcode::LDY_ABX));
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x02);
  execute_cycles(4);  // LDY Absolute X takes 4 cycles (no page cross)

  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPULoadTest, ldy_absolute_x_page_cross) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IMM));
  bus.write(0xFFFD, 0xFF);  // X = 0xFF to force page cross
  execute_cycles(2);        // LDX_IMM takes 2 cycles

  // Then test LDY absolute X with page cross
  bus.write(0x0341, 0x37);
  bus.write(0xFFFE, (nes::u8)(nes::Opcode::LDY_ABX));
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x02);
  execute_cycles(5);  // LDY Absolute X takes 5 cycles with page cross

  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}
