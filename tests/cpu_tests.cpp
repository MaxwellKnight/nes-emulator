#include "../include/bus.h"
#include "../include/cpu.h"
#include <gtest/gtest.h>

class CPUTest : public ::testing::Test {
protected:
  void SetUp() override { cpu.reset(); }

  void execute_cycles(int cycles) {
    for (int i = 0; i < cycles; i++) {
      cpu.clock();
    }
    EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  }

  nes::Bus bus;
  nes::CPU cpu{bus};
};

TEST_F(CPUTest, Initialization) {
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_EQ(cpu.get_sp(), 0xFF);
  EXPECT_EQ(cpu.get_pc(), 0xFFFC);
  EXPECT_EQ(cpu.get_status() & 0x30, 0x30); // UNUSED and BREAK flags
  EXPECT_EQ(cpu.get_remaining_cycles(), 0); // Should start with 0 cycles
}

TEST_F(CPUTest, LDAImmediate) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDA Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDAImmediateZeroFlag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x00);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDA Immediate takes 2 cycles
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDAImmediateNegativeFlag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x80);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDA Immediate takes 2 cycles
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, TAX) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);
  execute_cycles(2); // LDA_IM takes 2 cycles

  bus.write(0xFFFE, (nes::u8)(nes::Opcode::TAX));
  execute_cycles(2); // TAX takes 2 cycles
  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDAZeroPage) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_ZP));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(3); // LDA Zero Page takes 3 cycles
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, STAZeroPage) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);
  execute_cycles(2); // LDA_IM takes 2 cycles

  bus.write(0xFFFE, (nes::u8)(nes::Opcode::STA_ZP));
  bus.write(0xFFFF, 0x20);
  execute_cycles(3); // STA Zero Page takes 3 cycles
  EXPECT_EQ(bus.read(0x20), 0x42);
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
}

TEST_F(CPUTest, TXA) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);
  execute_cycles(2); // LDA_IM takes 2 cycles

  bus.write(0xFFFE, (nes::u8)(nes::Opcode::TAX));
  execute_cycles(2); // TAX takes 2 cycles

  bus.write(0xFFFF, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0x0000, 0x00);
  execute_cycles(2); // LDA_IM takes 2 cycles

  bus.write(0x0001, (nes::u8)(nes::Opcode::TXA));
  execute_cycles(2); // TXA takes 2 cycles
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDXImmediate) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDX Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDXImmediateZeroFlag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x00);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDX Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDXImmediateNegativeFlag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x80);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDX Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_x(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDXZeroPage) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_ZP));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(3); // LDX Zero Page takes 3 cycles
  EXPECT_EQ(cpu.get_x(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYImmediate) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_IM));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDY Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_flag(nes::Flag::CARRY), false);
  EXPECT_EQ(cpu.get_flag(nes::Flag::OVERFLOW_), false);
}

TEST_F(CPUTest, LDYImmediateZeroFlag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_IM));
  bus.write(0xFFFD, 0x00);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDY Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYImmediateNegativeFlag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_IM));
  bus.write(0xFFFD, 0x80);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDY Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYZeroPage) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_ZP));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(3); // LDY Zero Page takes 3 cycles
  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYZeroPageX) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x02);
  execute_cycles(2); // LDX_IM takes 2 cycles

  // Then test LDY zero page X
  bus.write(0x44, 0x37); // Target address: 0x42 + 0x02 = 0x44
  bus.write(0xFFFE, (nes::u8)(nes::Opcode::LDY_XZP));
  bus.write(0xFFFF, 0x42);
  execute_cycles(4); // LDY Zero Page X takes 4 cycles
  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYAbsolute) {
  bus.write(0x0242, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_ABS));
  bus.write(0xFFFD, 0x42);
  bus.write(0xFFFE, 0x02); // Changed high byte to 0x02

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(4); // LDY Absolute takes 4 cycles
  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYAbsoluteX) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x02);
  execute_cycles(2); // LDX_IM takes 2 cycles

  // Then test LDY absolute X
  bus.write(0x0244, 0x37);
  bus.write(0xFFFE, (nes::u8)(nes::Opcode::LDY_XABS));
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x02);
  execute_cycles(4); // LDY Absolute X takes 4 cycles (no page cross)

  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYAbsoluteXPageCross) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0xFF); // X = 0xFF to force page cross
  execute_cycles(2);       // LDX_IM takes 2 cycles

  // Then test LDY absolute X with page cross
  bus.write(0x0341, 0x37);
  bus.write(0xFFFE, (nes::u8)(nes::Opcode::LDY_XABS));
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x02);
  execute_cycles(5); // LDY Absolute X takes 5 cycles with page cross

  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}
