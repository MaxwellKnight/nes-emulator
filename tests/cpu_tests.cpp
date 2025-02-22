#include "../include/bus.h"
#include "../include/cpu.h"
#include <gtest/gtest.h>

class CPUTest : public ::testing::Test {
protected:
  void SetUp() override { cpu.reset(); }

  void execute_instruction() {
    do {
      cpu.clock();
    } while (cpu.get_remaining_cycles() > 0);
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
}

TEST_F(CPUTest, LDAImmediate) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);

  execute_instruction();

  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDAImmediateZeroFlag) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x00);

  execute_instruction();

  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDAImmediateNegativeFlag) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x80);

  execute_instruction();

  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, TAX) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);
  execute_instruction();

  bus.write(0xFFFE, static_cast<nes::u8>(nes::Opcode::TAX));
  execute_instruction();

  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDAZeroPage) {
  bus.write(0x42, 0x37);

  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_ZP));
  bus.write(0xFFFD, 0x42);

  execute_instruction();

  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, STAZeroPage) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);
  execute_instruction();

  bus.write(0xFFFE, static_cast<nes::u8>(nes::Opcode::STA_ZP));
  bus.write(0xFFFF, 0x20);
  execute_instruction();

  EXPECT_EQ(bus.read(0x20), 0x42);
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
}

TEST_F(CPUTest, TXA) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);
  execute_instruction();

  bus.write(0xFFFE, static_cast<nes::u8>(nes::Opcode::TAX));
  execute_instruction();

  bus.write(0xFFFF, static_cast<nes::u8>(nes::Opcode::LDA_IM));
  bus.write(0x0000, 0x00);
  execute_instruction();

  bus.write(0x0001, static_cast<nes::u8>(nes::Opcode::TXA));
  execute_instruction();

  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDXImmediate) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x42);

  execute_instruction();

  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDXImmediateZeroFlag) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x00);
  execute_instruction();

  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDXImmediateNegativeFlag) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x80);
  execute_instruction();

  EXPECT_EQ(cpu.get_x(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDXZeroPage) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDX_ZP));
  bus.write(0xFFFD, 0x42);
  execute_instruction();

  EXPECT_EQ(cpu.get_x(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYImmediate) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDY_IM));
  bus.write(0xFFFD, 0x42);

  execute_instruction();

  EXPECT_EQ(cpu.get_y(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  // Verify C and V flags are unaffected
  EXPECT_EQ(cpu.get_flag(nes::Flag::CARRY), false);
  EXPECT_EQ(cpu.get_flag(nes::Flag::OVERFLOW_), false);
}

TEST_F(CPUTest, LDYImmediateZeroFlag) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDY_IM));
  bus.write(0xFFFD, 0x00);
  execute_instruction();

  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYImmediateNegativeFlag) {
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDY_IM));
  bus.write(0xFFFD, 0x80);
  execute_instruction();

  EXPECT_EQ(cpu.get_y(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYZeroPage) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDY_ZP));
  bus.write(0xFFFD, 0x42);
  execute_instruction();

  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYZeroPageX) {
  // First set X register
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x02);
  execute_instruction();

  // Then test LDY zero page X
  bus.write(0x44, 0x37); // Target address: 0x42 + 0x02 = 0x44
  bus.write(0xFFFE, static_cast<nes::u8>(nes::Opcode::LDY_XZP));
  bus.write(0xFFFF, 0x42);
  execute_instruction();

  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYAbsolute) {
  bus.write(0x4242, 0x37);
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDY_ABS));
  bus.write(0xFFFD, 0x42);
  bus.write(0xFFFE, 0x42);
  execute_instruction();

  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYAbsoluteX) {
  // First set X register
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x02);
  execute_instruction();

  // Then test LDY absolute X
  bus.write(0x4244, 0x37); // Target address: 0x4242 + 0x02 = 0x4244
  bus.write(0xFFFE, static_cast<nes::u8>(nes::Opcode::LDY_XABS));
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x42);
  execute_instruction();

  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, LDYAbsoluteXPageCross) {
  // First set X register
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0xFF); // X = 0xFF to force page cross
  execute_instruction();

  // Then test LDY absolute X with page cross
  bus.write(0x4341, 0x37); // Write to 0x4242 + 0xFF = 0x4341
  bus.write(0xFFFE, static_cast<nes::u8>(nes::Opcode::LDY_XABS));
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x42);
  execute_instruction();

  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}
