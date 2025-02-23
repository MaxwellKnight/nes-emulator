#include "../include/bus.h"
#include "../include/cpu.h"
#include "types.h"
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

TEST_F(CPUTest, initialization) {
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_EQ(cpu.get_sp(), 0xFF);
  EXPECT_EQ(cpu.get_pc(), 0xFFFC);
  EXPECT_EQ(cpu.get_status() & 0x30, 0x30); // UNUSED and BREAK flags
  EXPECT_EQ(cpu.get_remaining_cycles(), 0); // Should start with 0 cycles
}

TEST_F(CPUTest, lda_immediate) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDA Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, lda_immediate_zero_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x00);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDA Immediate takes 2 cycles
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, lda_immediate_negative_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x80);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDA Immediate takes 2 cycles
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, tax) {
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

TEST_F(CPUTest, tay) {
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

TEST_F(CPUTest, tay_zero_flag) {
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

TEST_F(CPUTest, tay_negative_flag) {
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

TEST_F(CPUTest, lda_zero_page) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_ZP));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(3); // LDA Zero Page takes 3 cycles
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, sta_zero_page) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IM));
  bus.write(0xFFFD, 0x42);
  execute_cycles(2); // LDA_IM takes 2 cycles

  bus.write(0xFFFE, (nes::u8)(nes::Opcode::STA_ZP));
  bus.write(0xFFFF, 0x20);
  execute_cycles(3); // STA Zero Page takes 3 cycles
  EXPECT_EQ(bus.read(0x20), 0x42);
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
}

TEST_F(CPUTest, txa) {
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

TEST_F(CPUTest, ldx_immediate) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDX Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_EQ(cpu.get_pc(), 0xFFFE);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, ldx_immediate_zero_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x00);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDX Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, ldx_immediate_negative_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_IM));
  bus.write(0xFFFD, 0x80);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDX Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_x(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, ldx_zero_page) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDX_ZP));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(3); // LDX Zero Page takes 3 cycles
  EXPECT_EQ(cpu.get_x(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, ldy_immediate) {
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

TEST_F(CPUTest, ldy_immediate_zero_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_IM));
  bus.write(0xFFFD, 0x00);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDY Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, ldy_immediate_negtive_flag) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_IM));
  bus.write(0xFFFD, 0x80);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(2); // LDY Immediate takes 2 cycles
  EXPECT_EQ(cpu.get_y(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, ldy_zero_page) {
  bus.write(0x42, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_ZP));
  bus.write(0xFFFD, 0x42);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(3); // LDY Zero Page takes 3 cycles
  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, ldy_zero_page_x) {
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

TEST_F(CPUTest, ldy_absolute) {
  bus.write(0x0242, 0x37);
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDY_ABS));
  bus.write(0xFFFD, 0x42);
  bus.write(0xFFFE, 0x02);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  execute_cycles(4); // LDY Absolute takes 4 cycles
  EXPECT_EQ(cpu.get_y(), 0x37);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUTest, ldy_absolute_x) {
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

TEST_F(CPUTest, ldy_absolute_x_page_cross) {
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

// STA_ABS = 0x8D,  // STA Absolute
// STA_XABS = 0x9D, // STA X-Index Absolute
// STA_YABS = 0x99, // STA Y-Index Absolute
// STA_ZP = 0x85,   // STA Zero Page
// STA_XZP = 0x95,  // STA Absolute
// STA_YZP = 0x81,  // STA Absolute
// STA_YZPI = 0x91, // STA Absolute
TEST_F(CPUTest, sta_absolute) {
  // First load value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2); // LDA_IM takes 2 cycles

  // Then store it using STA absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STA_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03); // Target address: 0x0341
  execute_cycles(4);       // STA Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x37);
  EXPECT_EQ(cpu.get_accumulator(), 0x37); // Accumulator should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, sta_absolute_x) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x02);
  execute_cycles(2); // LDX_IM takes 2 cycles

  // Load value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFF, 0x37);
  execute_cycles(2); // LDA_IM takes 2 cycles

  // Store using STA absolute X
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_XABS);
  bus.write(0x0001, 0x42);
  bus.write(0x0002, 0x02); // Base address: 0x0242, X = 0x02, final: 0x0244
  execute_cycles(5);       // STA Absolute X takes 5 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0244), 0x37); // 0x0242 + 0x02
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, sta_absolute_y) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0x02);
  execute_cycles(2); // LDY_IM takes 2 cycles

  // Load value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFF, 0x37);
  execute_cycles(2); // LDA_IM takes 2 cycles

  // Store using STA absolute Y
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_YABS);
  bus.write(0x0001, 0x42);
  bus.write(0x0002, 0x02); // Base address: 0x0242, Y = 0x02, final: 0x0244
  execute_cycles(5);       // STA Absolute Y takes 5 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0244), 0x37); // 0x0242 + 0x02
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, sta_zero_page_) {
  // Load value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2); // LDA_IM takes 2 cycles

  // Store using STA zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STA_ZP);
  bus.write(0xFFFF, 0x42); // Zero page address: 0x42
  execute_cycles(3);       // STA Zero Page takes 3 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x42), 0x37);
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, sta_zero_page_x) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x02);
  execute_cycles(2); // LDX_IM takes 2 cycles

  // Load value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFF, 0x37);
  execute_cycles(2); // LDA_IM takes 2 cycles

  // Store using STA zero page X
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_XZP);
  bus.write(0x0001, 0x42); // Base address: 0x42, X = 0x02, final: 0x44
  execute_cycles(4);       // STA Zero Page X takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x44), 0x37); // 0x42 + 0x02
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, sta_indirect_zero_page_x) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x02); // X = 0x02
  execute_cycles(2);       // LDX_IM takes 2 cycles

  // Load value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFF, 0x37); // A = 0x37
  execute_cycles(2);       // LDA_IM takes 2 cycles

  // Set up the indirect address
  bus.write(0x24, 0x80); // Zero Page address (0x22 + X) contains 0x80
  bus.write(0x25, 0x04); // Zero Page address (0x23 + X) contains 0x04
                         // This makes the final address 0x0480

  // Store using STA indexed indirect
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_XZPI);
  bus.write(0x0001, 0x22); // Zero Page address before X indexing
  execute_cycles(6);       // STA (Indirect,X) takes 6 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0480), 0x37);
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, sta_indirect_zero_page_y) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0x02); // Y = 0x02
  execute_cycles(2);       // LDY_IM takes 2 cycles

  // Load a value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFF, 0x37); // A = 0x37
  execute_cycles(2);       // LDA_IM takes 2 cycles

  // Set up the indirect address
  bus.write(0x22, 0x80); // Zero Page address contains low byte
  bus.write(0x23, 0x04); // Zero Page address + 1 contains high byte
                         // This makes the base address 0x0480
                         // Final address will be 0x0482 (after adding Y)

  // Write the STA instruction
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_YZPI);
  bus.write(0x0001, 0x22); // Zero Page address
  execute_cycles(6);       // STA ($nn),Y takes 6 cycles

  // Verify the value was stored at the correct location
  EXPECT_EQ(bus.read(0x0482), 0x37);
  EXPECT_EQ(cpu.get_accumulator(), 0x37); // Accumulator should remain unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, sta_indirect_zero_page_y_page_cross) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0xFF); // Y = 0xFF to force page crossing
  execute_cycles(2);       // LDY_IM takes 2 cycles

  // Load a value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFF, 0x37); // A = 0x37
  execute_cycles(2);       // LDA_IM takes 2 cycles

  // Set up the indirect address
  bus.write(0x22, 0x80); // Zero Page address contains low byte
  bus.write(0x23, 0x04); // Zero Page address + 1 contains high byte
                         // This makes the base address 0x0480
                         // Final address will be 0x057F (after adding Y)

  // Write the STA instruction
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_YZPI);
  bus.write(0x0001, 0x22); // Zero Page address
  execute_cycles(6); // STA ($nn),Y takes 6 cycles (no extra cycle for page
                     // cross on writes)

  // Verify the value was stored at the correct location
  EXPECT_EQ(bus.read(0x057F), 0x37);
  EXPECT_EQ(cpu.get_accumulator(), 0x37); // Accumulator should remain unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, stx_absolute) {
  // First load value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2); // LDX_IM takes 2 cycles

  // Then store it using STX absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STX_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03); // Target address: 0x0341
  execute_cycles(4);       // STX Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x37);
  EXPECT_EQ(cpu.get_x(), 0x37); // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, stx_zero_page) {
  // Load value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2); // LDX_IM takes 2 cycles

  // Store using STX zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STX_ZP);
  bus.write(0xFFFF, 0x42); // Zero page address: 0x42
  execute_cycles(3);       // STX Zero Page takes 3 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x42), 0x37);
  EXPECT_EQ(cpu.get_x(), 0x37); // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, stx_zero_page_y) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0x02);
  execute_cycles(2); // LDY_IM takes 2 cycles

  // Load value into X register
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFF, 0x37);
  execute_cycles(2); // LDX_IM takes 2 cycles

  // Store using STX zero page Y
  bus.write(0x0000, (nes::u8)nes::Opcode::STX_YZP);
  bus.write(0x0001, 0x42); // Base address: 0x42, Y = 0x02, final: 0x44
  execute_cycles(4);       // STX Zero Page Y takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x44), 0x37); // 0x42 + 0x02
  EXPECT_EQ(cpu.get_x(), 0x37);    // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test storing zero value
TEST_F(CPUTest, stx_zero_value) {
  // Load zero into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2); // LDX_IM takes 2 cycles

  // Store using STX absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STX_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03); // Target address: 0x0341
  execute_cycles(4);       // STX Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x00);
  EXPECT_EQ(cpu.get_x(), 0x00); // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test storing negative value (high bit set)
TEST_F(CPUTest, stx_negative_value) {
  // Load negative value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x80); // Negative value (high bit set)
  execute_cycles(2);       // LDX_IM takes 2 cycles

  // Store using STX absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STX_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03); // Target address: 0x0341
  execute_cycles(4);       // STX Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x80);
  EXPECT_EQ(cpu.get_x(), 0x80); // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test zero page wrap-around with Y-indexed addressing
TEST_F(CPUTest, stx_zero_page_y_wrap) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0xFF); // Y = 0xFF
  execute_cycles(2);       // LDY_IM takes 2 cycles

  // Load value into X register
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFF, 0x37);
  execute_cycles(2); // LDX_IM takes 2 cycles

  // Store using STX zero page Y
  bus.write(0x0000, (nes::u8)nes::Opcode::STX_YZP);
  bus.write(0x0001, 0x84); // Base address: 0x84
                           // When Y (0xFF) is added: 0x84 + 0xFF = 0x183
                           // After zero page wrap: 0x83
  execute_cycles(4);       // STX Zero Page Y takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x83), 0x37); // (0x84 + 0xFF) & 0xFF = 0x83
  EXPECT_EQ(cpu.get_x(), 0x37);    // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, sty_absolute) {
  // First load value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2); // LDY_IM takes 2 cycles

  // Then store it using STY absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STY_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03); // Target address: 0x0341
  execute_cycles(4);       // STY Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x37);
  EXPECT_EQ(cpu.get_y(), 0x37); // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, sty_zero_page) {
  // Load value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2); // LDY_IM takes 2 cycles

  // Store using STY zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STY_ZP);
  bus.write(0xFFFF, 0x42); // Zero page address: 0x42
  execute_cycles(3);       // STY Zero Page takes 3 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x42), 0x37);
  EXPECT_EQ(cpu.get_y(), 0x37); // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, sty_zero_page_x) {
  // First set X register for indexing
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0x02); // X = 0x02
  execute_cycles(2);       // LDX_IM takes 2 cycles

  // Load value into Y register
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFF, 0x37); // Y = 0x37
  execute_cycles(2);       // LDY_IM takes 2 cycles

  // Store using STY zero page X
  bus.write(0x0000, (nes::u8)nes::Opcode::STY_XZP);
  bus.write(0x0001, 0x42); // Base address: 0x42, X = 0x02, final: 0x44
  execute_cycles(4);       // STY Zero Page X takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x44), 0x37); // 0x42 + 0x02
  EXPECT_EQ(cpu.get_y(), 0x37);    // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test storing zero value
TEST_F(CPUTest, sty_zero_value) {
  // Load zero into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2); // LDX_IM takes 2 cycles

  // Store using STY absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STY_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03); // Target address: 0x0341
  execute_cycles(4);       // STY Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x00);
  EXPECT_EQ(cpu.get_y(), 0x00); // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test storing negative value (high bit set)
TEST_F(CPUTest, sty_negative_value) {
  // Load negative value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFD, 0x80); // Negative value (high bit set)
  execute_cycles(2);       // LDY_IM takes 2 cycles

  // Store using STY absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STY_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03); // Target address: 0x0341
  execute_cycles(4);       // STY Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x80);
  EXPECT_EQ(cpu.get_y(), 0x80); // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test zero page wrap-around with Y-indexed addressing
TEST_F(CPUTest, sty_zero_page_x_wrap) {
  // First set X register for indexing
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IM);
  bus.write(0xFFFD, 0xFF); // X = 0xFF to force wrap-around
  execute_cycles(2);       // LDX_IM takes 2 cycles

  // Load value into Y register
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDY_IM);
  bus.write(0xFFFF, 0x37); // Y = 0x37
  execute_cycles(2);       // LDY_IM takes 2 cycles

  // Store using STY zero page X
  bus.write(0x0000, (nes::u8)nes::Opcode::STY_XZP);
  bus.write(0x0001, 0x84); // Base address: 0x84
                           // When X (0xFF) is added: 0x84 + 0xFF = 0x183
                           // After zero page wrap: 0x83
  execute_cycles(4);       // STY Zero Page X takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x83), 0x37); // (0x84 + 0xFF) & 0xFF = 0x83
  EXPECT_EQ(cpu.get_y(), 0x37);    // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// TSX
TEST_F(CPUTest, tsx) {
  // Set up a specific stack pointer value
  cpu.reset(); // This sets SP to 0xFF

  // Execute TSX
  bus.write(0xFFFC, (nes::u8)nes::Opcode::TSX);
  execute_cycles(2); // TSX takes 2 cycles

  EXPECT_EQ(cpu.get_x(), 0xFF); // X should now equal SP
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUTest, tsx_zero_value) {
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

TEST_F(CPUTest, tsx_negative_value) {
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

TEST_F(CPUTest, txa_zero_value) {
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

TEST_F(CPUTest, txa_negative_value) {
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
TEST_F(CPUTest, txs) {
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

TEST_F(CPUTest, txs_zero_value) {
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
TEST_F(CPUTest, tya) {
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

TEST_F(CPUTest, tya_zero_value) {
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

TEST_F(CPUTest, tya_negative_value) {
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
