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
  // Setup
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);

  // Execute
  execute_instruction();

  // Verify
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
  // Setup: Load value into A first
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
