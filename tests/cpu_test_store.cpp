#include "cpu_test_base.h"

class CPUStoreTest : public CPUTestBase {};

TEST_F(CPUStoreTest, sta_absolute) {
  // First load value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2);  // LDA_IMM takes 2 cycles

  // Then store it using STA absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STA_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03);  // Target address: 0x0341
  execute_cycles(4);        // STA Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x37);
  EXPECT_EQ(cpu.get_accumulator(), 0x37);  // Accumulator should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sta_absolute_x) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x02);
  execute_cycles(2);  // LDX_IM takes 2 cycles

  // Load value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x37);
  execute_cycles(2);  // LDA_IMM takes 2 cycles

  // Store using STA absolute X
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_ABX);
  bus.write(0x0001, 0x42);
  bus.write(0x0002, 0x02);  // Base address: 0x0242, X = 0x02, final: 0x0244
  execute_cycles(5);        // STA Absolute X takes 5 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0244), 0x37);  // 0x0242 + 0x02
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sta_absolute_y) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x02);
  execute_cycles(2);  // LDY_IM takes 2 cycles

  // Load value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x37);
  execute_cycles(2);  // LDA_IMM takes 2 cycles

  // Store using STA absolute Y
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_ABY);
  bus.write(0x0001, 0x42);
  bus.write(0x0002, 0x02);  // Base address: 0x0242, Y = 0x02, final: 0x0244
  execute_cycles(5);        // STA Absolute Y takes 5 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0244), 0x37);  // 0x0242 + 0x02
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sta_zero_page_) {
  // Load value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2);  // LDA_IMM takes 2 cycles

  // Store using STA zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STA_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address: 0x42
  execute_cycles(3);        // STA Zero Page takes 3 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x42), 0x37);
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sta_zero_page_x) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x02);
  execute_cycles(2);  // LDX_IM takes 2 cycles

  // Load value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x37);
  execute_cycles(2);  // LDA_IMM takes 2 cycles

  // Store using STA zero page X
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_ZPX);
  bus.write(0x0001, 0x42);  // Base address: 0x42, X = 0x02, final: 0x44
  execute_cycles(4);        // STA Zero Page X takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x44), 0x37);  // 0x42 + 0x02
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sta_indirect_zero_page_x) {
  // First set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x02);  // X = 0x02
  execute_cycles(2);        // LDX_IM takes 2 cycles

  // Load value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x37);  // A = 0x37
  execute_cycles(2);        // LDA_IMM takes 2 cycles

  // Set up the indirect address
  bus.write(0x24, 0x80);  // Zero Page address (0x22 + X) contains 0x80
  bus.write(0x25, 0x04);  // Zero Page address (0x23 + X) contains 0x04
                          // This makes the final address 0x0480

  // Store using STA indexed indirect
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_IZX);
  bus.write(0x0001, 0x22);  // Zero Page address before X indexing
  execute_cycles(6);        // STA (Indirect,X) takes 6 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0480), 0x37);
  EXPECT_EQ(cpu.get_accumulator(), 0x37);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sta_indirect_zero_page_y) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x02);  // Y = 0x02
  execute_cycles(2);        // LDY_IM takes 2 cycles

  // Load a value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x37);  // A = 0x37
  execute_cycles(2);        // LDA_IMM takes 2 cycles

  // Set up the indirect address
  bus.write(0x22, 0x80);  // Zero Page address contains low byte
  bus.write(0x23, 0x04);  // Zero Page address + 1 contains high byte
                          // This makes the base address 0x0480
                          // Final address will be 0x0482 (after adding Y)

  // Write the STA instruction
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_IZY);
  bus.write(0x0001, 0x22);  // Zero Page address
  execute_cycles(6);        // STA ($nn),Y takes 6 cycles

  // Verify the value was stored at the correct location
  EXPECT_EQ(bus.read(0x0482), 0x37);
  EXPECT_EQ(cpu.get_accumulator(), 0x37);  // Accumulator should remain unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sta_indirect_zero_page_y_page_cross) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0xFF);  // Y = 0xFF to force page crossing
  execute_cycles(2);        // LDY_IM takes 2 cycles

  // Load a value into accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x37);  // A = 0x37
  execute_cycles(2);        // LDA_IMM takes 2 cycles

  // Set up the indirect address
  bus.write(0x22, 0x80);  // Zero Page address contains low byte
  bus.write(0x23, 0x04);  // Zero Page address + 1 contains high byte
                          // This makes the base address 0x0480
                          // Final address will be 0x057F (after adding Y)

  // Write the STA instruction
  bus.write(0x0000, (nes::u8)nes::Opcode::STA_IZY);
  bus.write(0x0001, 0x22);  // Zero Page address
  execute_cycles(6);        // STA ($nn),Y takes 6 cycles (no extra cycle for page
                            // cross on writes)

  // Verify the value was stored at the correct location
  EXPECT_EQ(bus.read(0x057F), 0x37);
  EXPECT_EQ(cpu.get_accumulator(), 0x37);  // Accumulator should remain unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, stx_absolute) {
  // First load value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2);  // LDX_IM takes 2 cycles

  // Then store it using STX absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STX_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03);  // Target address: 0x0341
  execute_cycles(4);        // STX Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x37);
  EXPECT_EQ(cpu.get_x(), 0x37);  // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, stx_zero_page) {
  // Load value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2);  // LDX_IM takes 2 cycles

  // Store using STX zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STX_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address: 0x42
  execute_cycles(3);        // STX Zero Page takes 3 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x42), 0x37);
  EXPECT_EQ(cpu.get_x(), 0x37);  // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, stx_zero_page_y) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x02);
  execute_cycles(2);  // LDY_IM takes 2 cycles

  // Load value into X register
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFF, 0x37);
  execute_cycles(2);  // LDX_IM takes 2 cycles

  // Store using STX zero page Y
  bus.write(0x0000, (nes::u8)nes::Opcode::STX_ZPY);
  bus.write(0x0001, 0x42);  // Base address: 0x42, Y = 0x02, final: 0x44
  execute_cycles(4);        // STX Zero Page Y takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x44), 0x37);  // 0x42 + 0x02
  EXPECT_EQ(cpu.get_x(), 0x37);     // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test storing zero value
TEST_F(CPUStoreTest, stx_zero_value) {
  // Load zero into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2);  // LDX_IM takes 2 cycles

  // Store using STX absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STX_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03);  // Target address: 0x0341
  execute_cycles(4);        // STX Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x00);
  EXPECT_EQ(cpu.get_x(), 0x00);  // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test storing negative value (high bit set)
TEST_F(CPUStoreTest, stx_negative_value) {
  // Load negative value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x80);  // Negative value (high bit set)
  execute_cycles(2);        // LDX_IM takes 2 cycles

  // Store using STX absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STX_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03);  // Target address: 0x0341
  execute_cycles(4);        // STX Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x80);
  EXPECT_EQ(cpu.get_x(), 0x80);  // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test zero page wrap-around with Y-indexed addressing
TEST_F(CPUStoreTest, stx_zero_page_y_wrap) {
  // First set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0xFF);  // Y = 0xFF
  execute_cycles(2);        // LDY_IM takes 2 cycles

  // Load value into X register
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFF, 0x37);
  execute_cycles(2);  // LDX_IM takes 2 cycles

  // Store using STX zero page Y
  bus.write(0x0000, (nes::u8)nes::Opcode::STX_ZPY);
  bus.write(0x0001, 0x84);  // Base address: 0x84
                            // When Y (0xFF) is added: 0x84 + 0xFF = 0x183
                            // After zero page wrap: 0x83
  execute_cycles(4);        // STX Zero Page Y takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x83), 0x37);  // (0x84 + 0xFF) & 0xFF = 0x83
  EXPECT_EQ(cpu.get_x(), 0x37);     // X register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sty_absolute) {
  // First load value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2);  // LDY_IM takes 2 cycles

  // Then store it using STY absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STY_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03);  // Target address: 0x0341
  execute_cycles(4);        // STY Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x37);
  EXPECT_EQ(cpu.get_y(), 0x37);  // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sty_zero_page) {
  // Load value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x37);
  execute_cycles(2);  // LDY_IM takes 2 cycles

  // Store using STY zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STY_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address: 0x42
  execute_cycles(3);        // STY Zero Page takes 3 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x42), 0x37);
  EXPECT_EQ(cpu.get_y(), 0x37);  // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sty_zero_page_x) {
  // First set X register for indexing
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x02);  // X = 0x02
  execute_cycles(2);        // LDX_IM takes 2 cycles

  // Load value into Y register
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFF, 0x37);  // Y = 0x37
  execute_cycles(2);        // LDY_IM takes 2 cycles

  // Store using STY zero page X
  bus.write(0x0000, (nes::u8)nes::Opcode::STY_ZPX);
  bus.write(0x0001, 0x42);  // Base address: 0x42, X = 0x02, final: 0x44
  execute_cycles(4);        // STY Zero Page X takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x44), 0x37);  // 0x42 + 0x02
  EXPECT_EQ(cpu.get_y(), 0x37);     // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test storing zero value
TEST_F(CPUStoreTest, sty_zero_value) {
  // Load zero into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2);  // LDX_IM takes 2 cycles

  // Store using STY absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STY_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03);  // Target address: 0x0341
  execute_cycles(4);        // STY Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x00);
  EXPECT_EQ(cpu.get_y(), 0x00);  // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test storing negative value (high bit set)
TEST_F(CPUStoreTest, sty_negative_value) {
  // Load negative value into X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x80);  // Negative value (high bit set)
  execute_cycles(2);        // LDY_IM takes 2 cycles

  // Store using STY absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::STY_ABS);
  bus.write(0xFFFF, 0x41);
  bus.write(0x0000, 0x03);  // Target address: 0x0341
  execute_cycles(4);        // STY Absolute takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x0341), 0x80);
  EXPECT_EQ(cpu.get_y(), 0x80);  // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test zero page wrap-around with Y-indexed addressing
TEST_F(CPUStoreTest, sty_zero_page_x_wrap) {
  // First set X register for indexing
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0xFF);  // X = 0xFF to force wrap-around
  execute_cycles(2);        // LDX_IM takes 2 cycles

  // Load value into Y register
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFF, 0x37);  // Y = 0x37
  execute_cycles(2);        // LDY_IM takes 2 cycles

  // Store using STY zero page X
  bus.write(0x0000, (nes::u8)nes::Opcode::STY_ZPX);
  bus.write(0x0001, 0x84);  // Base address: 0x84
                            // When X (0xFF) is added: 0x84 + 0xFF = 0x183
                            // After zero page wrap: 0x83
  execute_cycles(4);        // STY Zero Page X takes 4 cycles

  // Verify value was stored correctly
  EXPECT_EQ(bus.read(0x83), 0x37);  // (0x84 + 0xFF) & 0xFF = 0x83
  EXPECT_EQ(cpu.get_y(), 0x37);     // Y register should be unchanged
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUStoreTest, sta_zero_page) {
  bus.write(0xFFFC, (nes::u8)(nes::Opcode::LDA_IMM));
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);  // LDA_IMM takes 2 cycles

  bus.write(0xFFFE, (nes::u8)(nes::Opcode::STA_ZPG));
  bus.write(0xFFFF, 0x20);
  execute_cycles(3);  // STA Zero Page takes 3 cycles
  EXPECT_EQ(bus.read(0x20), 0x42);
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
}
