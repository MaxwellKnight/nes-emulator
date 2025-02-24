#include "../include/bus.h"
#include "../include/cpu.h"
#include "types.h"
#include <gtest/gtest.h>

class CPUTransferTest : public ::testing::Test {
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

TEST_F(CPUTransferTest, tax) {
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

TEST_F(CPUTransferTest, tay) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);
  execute_cycles(2); // LDA_IM takes 2 cycles

  bus.write(0xFFFE, (nes::u8)(nes::Opcode::TAY));
  execute_cycles(2); // TAX takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0x42);
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTransferTest, tay_zero_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x00);
  execute_cycles(2); // LDA_IM takes 2 cycles

  bus.write(0xFFFE, (nes::u8)(nes::Opcode::TAY));
  execute_cycles(2); // TAX takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0);
  EXPECT_EQ(cpu.get_accumulator(), 0);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTransferTest, tay_negative_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0xFF);
  execute_cycles(2); // LDA_IM takes 2 cycles

  bus.write(0xFFFE, (nes::u8)(nes::Opcode::TAY));
  execute_cycles(2); // TAX takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0xFF);
  EXPECT_EQ(cpu.get_accumulator(), 0xFF);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTransferTest, txa) {
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

TEST_F(CPUTransferTest, tsx) {
  // Set up a specific stack pointer value
  cpu.reset(); // This sets SP to 0xFF

  // Execute TSX
  bus.write(0xFFFC, (nes::u8)nes::Opcode::TSX);
  execute_cycles(2); // TSX takes 2 cycles

  EXPECT_EQ(cpu.get_x(), 0xFF); // X should now equal SP
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTransferTest, tsx_zero_value) {
  // Set stack pointer directly to 0x00
  cpu.set_sp(0x00);

  // Transfer stack pointer to X
  bus.write(0xFFFC, (nes::u8)nes::Opcode::TSX);
  execute_cycles(2); // TSX takes 2 cycles

  // Verify results
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTransferTest, tsx_negative_value) {
  // Set stack pointer to a negative value (bit 7 set)
  cpu.set_sp(0x80);

  // Transfer stack pointer to X
  bus.write(0xFFFC, (nes::u8)nes::Opcode::TSX);
  execute_cycles(2); // TSX takes 2 cycles

  // Verify results
  EXPECT_EQ(cpu.get_x(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTransferTest, txa_zero_value) {
  // Load 0 into X
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::TXA);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTransferTest, txa_negative_value) {
  // Load negative value into X
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x80); // Negative value (bit 7 set)
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::TXA);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x80);
  EXPECT_EQ(cpu.get_x(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// TXS Tests
TEST_F(CPUTransferTest, txs) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  // Then transfer X to SP
  bus.write(0xFFFE, (nes::u8)nes::Opcode::TXS);
  execute_cycles(2); // TXS takes 2 cycles

  EXPECT_EQ(cpu.get_sp(), 0x42);
  EXPECT_EQ(cpu.get_x(), 0x42); // X should remain unchanged
  // TXS doesn't affect any flags
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTransferTest, txs_zero_value) {
  // Load 0 into X
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::TXS);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_sp(), 0x00);
  EXPECT_EQ(cpu.get_x(), 0x00);
  // TXS doesn't affect any flags
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// TYA
TEST_F(CPUTransferTest, tya) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  // Then transfer Y to A
  bus.write(0xFFFE, (nes::u8)nes::Opcode::TYA);
  execute_cycles(2); // TYA takes 2 cycles

  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_y(), 0x42); // Y should remain unchanged
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTransferTest, tya_zero_value) {
  // Load 0 into Y
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::TYA);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTransferTest, tya_negative_value) {
  // Load negative value into Y
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0x80); // Negative value (bit 7 set)
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::TYA);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x80);
  EXPECT_EQ(cpu.get_y(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}
