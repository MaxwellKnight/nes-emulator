#include "cpu_test_base.h"

class CPUArithmeticTest : public CPUTestBase {};

// ADC - Basic Addition
TEST_F(CPUArithmeticTest, adc_basic_addition) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x20);  // Load 0x20 (32) into A
  execute_cycles(2);

  // Clear carry
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Execute ADC immediate
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x15);  // Add 0x15 (21) to accumulator
  execute_cycles(2);

  // Result: 0x20 + 0x15 = 0x35 (53)
  EXPECT_EQ(cpu.get_accumulator(), 0x35);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// ADC - Addition with Carry In
TEST_F(CPUArithmeticTest, adc_addition_with_carry) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x20);  // Load 0x20 (32) into A
  execute_cycles(2);

  // Set carry
  bus.write(0xFFFE, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Execute ADC immediate
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x15);  // Add 0x15 (21) to accumulator
  execute_cycles(2);

  // Result: 0x20 + 0x15 + 0x01 (carry) = 0x36 (54)
  EXPECT_EQ(cpu.get_accumulator(), 0x36);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// ADC - Addition with Carry Out
TEST_F(CPUArithmeticTest, adc_addition_with_carry_out) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xF0);  // Load 0xF0 (240) into A
  execute_cycles(2);

  // Clear carry
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Execute ADC immediate
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x10);  // Add 0x10 (16) to accumulator
  execute_cycles(2);

  // Result: 0xF0 + 0x10 = 0x100 (256), which truncates to 0x00 with carry
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// ADC - Addition with Carry In and Out
TEST_F(CPUArithmeticTest, adc_addition_with_carry_in_and_out) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xFF);  // Load 0xFF (255) into A
  execute_cycles(2);

  // Set carry
  bus.write(0xFFFE, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Execute ADC immediate
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x01);  // Add 0x01 (1) to accumulator
  execute_cycles(2);

  // Result: 0xFF + 0x01 + 0x01 (carry) = 0x101 (257), which truncates to 0x01 with carry
  EXPECT_EQ(cpu.get_accumulator(), 0x01);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// ADC - Overflow Flag Set (positive to negative)
TEST_F(CPUArithmeticTest, adc_overflow_positive_to_negative) {
  // Load accumulator with positive value near max positive (0x7F = 127)
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x7F);  // Load 0x7F (127, max positive 8-bit signed) into A
  execute_cycles(2);

  // Clear carry
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Execute ADC immediate
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x01);  // Add 0x01 (1) to accumulator
  execute_cycles(2);

  // Result: 0x7F + 0x01 = 0x80 (128, which is -128 in signed 8-bit)
  // This flips from positive to negative, setting overflow
  EXPECT_EQ(cpu.get_accumulator(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));   // Bit 7 is now set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::OVERFLOW_));  // Signed overflow occurred
}

// ADC - Overflow Flag Set (negative to positive)
TEST_F(CPUArithmeticTest, adc_overflow_negative_to_positive) {
  // Load accumulator with negative value (0x80 = -128)
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x80);  // Load 0x80 (-128 in signed 8-bit) into A
  execute_cycles(2);

  // Clear carry
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Execute ADC immediate
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x01);  // Add 0x01 (1) to accumulator
  execute_cycles(2);

  // Result: 0x80 + 0x01 = 0x81 (-127 in signed 8-bit)
  // No overflow as we're still in negative range
  EXPECT_EQ(cpu.get_accumulator(), 0x81);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));    // Bit 7 is still set
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));  // No signed overflow
}

// ADC - Test with negative operand
TEST_F(CPUArithmeticTest, adc_with_negative_operand) {
  // Load accumulator with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x20);  // Load 0x20 (32) into A
  execute_cycles(2);

  // Clear carry
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Execute ADC immediate with negative operand
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0xF0);  // Add 0xF0 (-16 in signed 8-bit) to accumulator
  execute_cycles(2);

  // Result: 0x20 + 0xF0 = 0x110, which truncates to 0x10 with carry
  EXPECT_EQ(cpu.get_accumulator(), 0x10);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// ADC - Addition resulting in zero
TEST_F(CPUArithmeticTest, adc_result_zero) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xFF);  // Load 0xFF (-1 in signed 8-bit) into A
  execute_cycles(2);

  // Clear carry
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Execute ADC immediate
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x01);  // Add 0x01 (1) to accumulator
  execute_cycles(2);

  // Result: 0xFF + 0x01 = 0x100, which truncates to 0x00 with carry
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// Test different addressing modes

// ADC - Immediate Addressing Mode
TEST_F(CPUArithmeticTest, adc_immediate) {
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x22);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x64);  // 0x42 + 0x22 = 0x64
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// ADC - Zero Page Addressing Mode
TEST_F(CPUArithmeticTest, adc_zero_page) {
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set up value in zero page
  bus.write(0x42, 0x22);

  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_ZPG);
  bus.write(0x0000, 0x42);  // Zero page address
  execute_cycles(3);

  EXPECT_EQ(cpu.get_accumulator(), 0x64);  // 0x42 + 0x22 = 0x64
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// ADC - Absolute Addressing Mode
TEST_F(CPUArithmeticTest, adc_absolute) {
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x1234, 0x22);

  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_ABS);
  bus.write(0x0000, 0x34);  // Low byte of address
  bus.write(0x0001, 0x12);  // High byte of address
  execute_cycles(4);

  EXPECT_EQ(cpu.get_accumulator(), 0x64);  // 0x42 + 0x22 = 0x64
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// ADC - Absolute X-Indexed Addressing Mode
TEST_F(CPUArithmeticTest, adc_absolute_x) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x10);
  execute_cycles(2);

  // Load accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x42);
  execute_cycles(2);

  // Clear carry
  bus.write(0x0000, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x1244, 0x22);  // 0x1234 + 0x10 = 0x1244

  bus.write(0x0001, (nes::u8)nes::Opcode::ADC_ABX);
  bus.write(0x0002, 0x34);  // Low byte of base address
  bus.write(0x0003, 0x12);  // High byte of base address
  execute_cycles(4);        // No page crossing

  EXPECT_EQ(cpu.get_accumulator(), 0x64);  // 0x42 + 0x22 = 0x64
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// ADC - Absolute X-Indexed with Page Crossing
TEST_F(CPUArithmeticTest, adc_absolute_x_page_crossing) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0xFF);  // Will cause page crossing
  execute_cycles(2);

  // Load accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x42);
  execute_cycles(2);

  // Clear carry
  bus.write(0x0000, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x12FE, 0x22);  // 0x1200 + 0xFF = 0x12FF (crosses page)

  bus.write(0x0001, (nes::u8)nes::Opcode::ADC_ABX);
  bus.write(0x0002, 0xFF);  // Low byte of base address
  bus.write(0x0003, 0x11);  // High byte of base address
  execute_cycles(5);        // Page crossing adds 1 cycle

  EXPECT_EQ(cpu.get_accumulator(), 0x64);  // 0x42 + 0x22 = 0x64
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// ADC - Multiple operations sequence
TEST_F(CPUArithmeticTest, adc_sequence) {
  // Start with 0 in accumulator, clear carry
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // First addition: 0 + 10 = 10
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x0A);  // Add 10
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x0A);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Second addition: 10 + 20 = 30
  bus.write(0x0001, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0002, 0x14);  // Add 20
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x1E);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Third addition: 30 + 200 = 230
  bus.write(0x0003, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0004, 0xC8);  // Add 200
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0xE6);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Fourth addition: 230 + 50 = 280, which wraps to 24 with carry
  bus.write(0x0005, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0006, 0x32);  // Add 50
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x18);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// CMP - Equal comparison
TEST_F(CPUArithmeticTest, cmp_equal) {
  // Load accumulator with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into A
  execute_cycles(2);

  // Execute CMP immediate with equal value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CMP_IMM);
  bus.write(0xFFFF, 0x42);  // Compare with 0x42
  execute_cycles(2);

  // Equal comparison sets Z flag and C flag, clears N flag
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Accumulator should remain unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// CMP - Accumulator greater than memory
TEST_F(CPUArithmeticTest, cmp_greater) {
  // Load accumulator with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into A
  execute_cycles(2);

  // Execute CMP immediate with smaller value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CMP_IMM);
  bus.write(0xFFFF, 0x30);  // Compare with 0x30
  execute_cycles(2);

  // A > M sets C flag, clears Z flag
  // N flag depends on bit 7 of result (0x42 - 0x30 = 0x12, so N should be clear)
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Accumulator should remain unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
}

// CMP - Accumulator less than memory
TEST_F(CPUArithmeticTest, cmp_less) {
  // Load accumulator with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into A
  execute_cycles(2);

  // Execute CMP immediate with larger value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CMP_IMM);
  bus.write(0xFFFF, 0x50);  // Compare with 0x50
  execute_cycles(2);

  // A < M clears C flag and Z flag
  // N flag depends on bit 7 of result (0x42 - 0x50 = 0xF2, so N should be set)
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Accumulator should remain unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
}

// CMP - With negative number
TEST_F(CPUArithmeticTest, cmp_negative) {
  // Load accumulator with negative value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x80);  // Load 0x80 (-128 in two's complement) into A
  execute_cycles(2);

  // Execute CMP immediate with positive value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CMP_IMM);
  bus.write(0xFFFF, 0x01);  // Compare with 0x01
  execute_cycles(2);

  // 0x80 - 0x01 = 0x7F, sets C flag, clears Z flag and N flag
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Accumulator should remain unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0x80);
}

// CMP - Compare with zero
TEST_F(CPUArithmeticTest, cmp_with_zero) {
  // Load accumulator with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into A
  execute_cycles(2);

  // Execute CMP immediate with zero
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CMP_IMM);
  bus.write(0xFFFF, 0x00);  // Compare with 0x00
  execute_cycles(2);

  // A > 0 sets C flag, clears Z flag
  // 0x42 - 0x00 = 0x42, so N should be clear
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Accumulator should remain unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
}

// CMP - Accumulator is zero
TEST_F(CPUArithmeticTest, cmp_accumulator_zero) {
  // Load accumulator with zero
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x00);  // Load 0x00 into A
  execute_cycles(2);

  // Execute CMP immediate with positive value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CMP_IMM);
  bus.write(0xFFFF, 0x01);  // Compare with 0x01
  execute_cycles(2);

  // 0x00 - 0x01 = 0xFF, clears C flag and Z flag, sets N flag
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Accumulator should remain unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
}

// Testing different addressing modes

// CMP - Zero Page addressing mode
TEST_F(CPUArithmeticTest, cmp_zero_page) {
  // Load accumulator with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into A
  execute_cycles(2);

  // Set up value in zero page
  bus.write(0x42, 0x42);  // Same value as accumulator

  // Execute CMP zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CMP_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // Equal comparison sets Z flag and C flag, clears N flag
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// CMP - Absolute addressing mode
TEST_F(CPUArithmeticTest, cmp_absolute) {
  // Load accumulator with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into A
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x1234, 0x30);  // Less than accumulator

  // Execute CMP absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CMP_ABS);
  bus.write(0xFFFF, 0x34);  // Low byte of address
  bus.write(0x0000, 0x12);  // High byte of address
  execute_cycles(4);

  // A > M sets C flag, clears Z flag and N flag
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// CMP - Absolute X-indexed addressing mode
TEST_F(CPUArithmeticTest, cmp_absolute_x) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x10);  // X = 0x10
  execute_cycles(2);

  // Load accumulator with value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x42);  // Load 0x42 into A
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x1244, 0x50);  // 0x1234 + 0x10 = 0x1244, value greater than accumulator

  // Execute CMP absolute X-indexed
  bus.write(0x0000, (nes::u8)nes::Opcode::CMP_ABX);
  bus.write(0x0001, 0x34);  // Low byte of base address
  bus.write(0x0002, 0x12);  // High byte of base address
  execute_cycles(4);        // No page crossing

  // A < M clears C flag and Z flag, sets N flag
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// CMP - Absolute X-indexed with page crossing
TEST_F(CPUArithmeticTest, cmp_absolute_x_page_crossing) {
  // Set X register to cause page crossing
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0xFF);  // X = 0xFF
  execute_cycles(2);

  // Load accumulator with value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x42);  // Load 0x42 into A
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x12FE, 0x42);  // 0x1200 + 0xFF = 0x12FF (page crossing), equal to accumulator

  // Execute CMP absolute X-indexed with page crossing
  bus.write(0x0000, (nes::u8)nes::Opcode::CMP_ABX);
  bus.write(0x0001, 0xFF);  // Low byte of base address
  bus.write(0x0002, 0x11);  // High byte of base address
  execute_cycles(5);        // Page crossing adds 1 cycle

  // Equal comparison sets Z flag and C flag, clears N flag
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// CMP - Absolute Y-indexed addressing mode
TEST_F(CPUArithmeticTest, cmp_absolute_y) {
  // Set Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x10);  // Y = 0x10
  execute_cycles(2);

  // Load accumulator with value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0xFF);  // Load 0xFF into A
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x1244, 0x01);  // 0x1234 + 0x10 = 0x1244, value less than accumulator

  // Execute CMP absolute Y-indexed
  bus.write(0x0000, (nes::u8)nes::Opcode::CMP_ABY);
  bus.write(0x0001, 0x34);  // Low byte of base address
  bus.write(0x0002, 0x12);  // High byte of base address
  execute_cycles(4);        // No page crossing

  // A > M sets C flag, clears Z flag, might set or clear N flag
  // 0xFF - 0x01 = 0xFE, which has bit 7 set
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// CMP - Zero Page X-indexed addressing mode
TEST_F(CPUArithmeticTest, cmp_zero_page_x) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x10);  // X = 0x10
  execute_cycles(2);

  // Load accumulator with value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x42);  // Load 0x42 into A
  execute_cycles(2);

  // Set up value in zero page
  bus.write(0x52, 0x42);  // 0x42 + 0x10 = 0x52, same as accumulator

  // Execute CMP zero page X-indexed
  bus.write(0x0000, (nes::u8)nes::Opcode::CMP_ZPX);
  bus.write(0x0001, 0x42);  // Zero page address
  execute_cycles(4);

  // Equal comparison sets Z flag and C flag, clears N flag
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// CPX - Equal comparison
TEST_F(CPUArithmeticTest, cpx_equal) {
  // Load X register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into X
  execute_cycles(2);

  // Execute CPX immediate with equal value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPX_IMM);
  bus.write(0xFFFF, 0x42);  // Compare with 0x42
  execute_cycles(2);

  // Equal comparison sets Z flag and C flag, clears N flag
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // X register should remain unchanged
  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// CPX - X register greater than memory
TEST_F(CPUArithmeticTest, cpx_greater) {
  // Load X register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into X
  execute_cycles(2);

  // Execute CPX immediate with smaller value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPX_IMM);
  bus.write(0xFFFF, 0x30);  // Compare with 0x30
  execute_cycles(2);

  // X > M sets C flag, clears Z flag
  // N flag depends on bit 7 of result (0x42 - 0x30 = 0x12, so N should be clear)
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // X register should remain unchanged
  EXPECT_EQ(cpu.get_x(), 0x42);
}

// CPX - X register less than memory
TEST_F(CPUArithmeticTest, cpx_less) {
  // Load X register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into X
  execute_cycles(2);

  // Execute CPX immediate with larger value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPX_IMM);
  bus.write(0xFFFF, 0x50);  // Compare with 0x50
  execute_cycles(2);

  // X < M clears C flag and Z flag
  // N flag depends on bit 7 of result (0x42 - 0x50 = 0xF2, so N should be set)
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // X register should remain unchanged
  EXPECT_EQ(cpu.get_x(), 0x42);
}

// CPX - With negative number in X
TEST_F(CPUArithmeticTest, cpx_negative_x) {
  // Load X register with negative value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x80);  // Load 0x80 (-128 in two's complement) into X
  execute_cycles(2);

  // Execute CPX immediate with positive value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPX_IMM);
  bus.write(0xFFFF, 0x01);  // Compare with 0x01
  execute_cycles(2);

  // 0x80 - 0x01 = 0x7F, sets C flag, clears Z flag and N flag
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // X register should remain unchanged
  EXPECT_EQ(cpu.get_x(), 0x80);
}

// CPX - Compare with zero
TEST_F(CPUArithmeticTest, cpx_with_zero) {
  // Load X register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into X
  execute_cycles(2);

  // Execute CPX immediate with zero
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPX_IMM);
  bus.write(0xFFFF, 0x00);  // Compare with 0x00
  execute_cycles(2);

  // X > 0 sets C flag, clears Z flag
  // 0x42 - 0x00 = 0x42, so N should be clear
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // X register should remain unchanged
  EXPECT_EQ(cpu.get_x(), 0x42);
}

// CPX - X register is zero
TEST_F(CPUArithmeticTest, cpx_x_zero) {
  // Load X register with zero
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x00);  // Load 0x00 into X
  execute_cycles(2);

  // Execute CPX immediate with positive value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPX_IMM);
  bus.write(0xFFFF, 0x01);  // Compare with 0x01
  execute_cycles(2);

  // 0x00 - 0x01 = 0xFF, clears C flag and Z flag, sets N flag
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // X register should remain unchanged
  EXPECT_EQ(cpu.get_x(), 0x00);
}

// Testing different addressing modes

// CPX - Zero Page addressing mode
TEST_F(CPUArithmeticTest, cpx_zero_page) {
  // Load X register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into X
  execute_cycles(2);

  // Set up value in zero page
  bus.write(0x42, 0x42);  // Same value as X register

  // Execute CPX zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPX_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // Equal comparison sets Z flag and C flag, clears N flag
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // X register should remain unchanged
  EXPECT_EQ(cpu.get_x(), 0x42);
}

// CPX - Absolute addressing mode
TEST_F(CPUArithmeticTest, cpx_absolute) {
  // Load X register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into X
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x1234, 0x30);  // Less than X register

  // Execute CPX absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPX_ABS);
  bus.write(0xFFFF, 0x34);  // Low byte of address
  bus.write(0x0000, 0x12);  // High byte of address
  execute_cycles(4);

  // X > M sets C flag, clears Z flag and N flag
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // X register should remain unchanged
  EXPECT_EQ(cpu.get_x(), 0x42);
}

// CPX - Verify overflow flag is not affected
TEST_F(CPUArithmeticTest, cpx_overflow_not_affected) {
  // Load X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  // Get current overflow state (should be true if we could set it)
  bool overflow_before = cpu.get_flag(nes::Flag::OVERFLOW_);

  // Execute CPX
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPX_IMM);
  bus.write(0xFFFF, 0x30);
  execute_cycles(2);

  // Overflow flag should remain unchanged
  EXPECT_EQ(cpu.get_flag(nes::Flag::OVERFLOW_), overflow_before);
}

// CPX - Ensure no other registers are affected
TEST_F(CPUArithmeticTest, cpx_other_registers_unchanged) {
  // Set up A and Y registers
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xAA);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFF, 0xBB);
  execute_cycles(2);

  // Load X register
  bus.write(0x0000, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0x0001, 0x42);
  execute_cycles(2);

  // Execute CPX
  bus.write(0x0002, (nes::u8)nes::Opcode::CPX_IMM);
  bus.write(0x0003, 0x30);
  execute_cycles(2);

  // A and Y should be unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0xAA);
  EXPECT_EQ(cpu.get_y(), 0xBB);
  EXPECT_EQ(cpu.get_x(), 0x42);
}

// CPY - Equal comparison
TEST_F(CPUArithmeticTest, cpy_equal) {
  // Load Y register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into Y
  execute_cycles(2);

  // Execute CPY immediate with equal value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPY_IMM);
  bus.write(0xFFFF, 0x42);  // Compare with 0x42
  execute_cycles(2);

  // Equal comparison sets Z flag and C flag, clears N flag
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Y register should remain unchanged
  EXPECT_EQ(cpu.get_y(), 0x42);
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// CPY - Y register greater than memory
TEST_F(CPUArithmeticTest, cpy_greater) {
  // Load Y register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into Y
  execute_cycles(2);

  // Execute CPY immediate with smaller value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPY_IMM);
  bus.write(0xFFFF, 0x30);  // Compare with 0x30
  execute_cycles(2);

  // Y > M sets C flag, clears Z flag
  // N flag depends on bit 7 of result (0x42 - 0x30 = 0x12, so N should be clear)
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Y register should remain unchanged
  EXPECT_EQ(cpu.get_y(), 0x42);
}

// CPY - Y register less than memory
TEST_F(CPUArithmeticTest, cpy_less) {
  // Load Y register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into Y
  execute_cycles(2);

  // Execute CPY immediate with larger value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPY_IMM);
  bus.write(0xFFFF, 0x50);  // Compare with 0x50
  execute_cycles(2);

  // Y < M clears C flag and Z flag
  // N flag depends on bit 7 of result (0x42 - 0x50 = 0xF2, so N should be set)
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Y register should remain unchanged
  EXPECT_EQ(cpu.get_y(), 0x42);
}

// CPY - With negative number in Y
TEST_F(CPUArithmeticTest, cpy_negative_y) {
  // Load Y register with negative value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x80);  // Load 0x80 (-128 in two's complement) into Y
  execute_cycles(2);

  // Execute CPY immediate with positive value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPY_IMM);
  bus.write(0xFFFF, 0x01);  // Compare with 0x01
  execute_cycles(2);

  // 0x80 - 0x01 = 0x7F, sets C flag, clears Z flag and N flag
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Y register should remain unchanged
  EXPECT_EQ(cpu.get_y(), 0x80);
}

// CPY - Compare with zero
TEST_F(CPUArithmeticTest, cpy_with_zero) {
  // Load Y register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into Y
  execute_cycles(2);

  // Execute CPY immediate with zero
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPY_IMM);
  bus.write(0xFFFF, 0x00);  // Compare with 0x00
  execute_cycles(2);

  // Y > 0 sets C flag, clears Z flag
  // 0x42 - 0x00 = 0x42, so N should be clear
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Y register should remain unchanged
  EXPECT_EQ(cpu.get_y(), 0x42);
}

// CPY - Y register is zero
TEST_F(CPUArithmeticTest, cpy_y_zero) {
  // Load Y register with zero
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x00);  // Load 0x00 into Y
  execute_cycles(2);

  // Execute CPY immediate with positive value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPY_IMM);
  bus.write(0xFFFF, 0x01);  // Compare with 0x01
  execute_cycles(2);

  // 0x00 - 0x01 = 0xFF, clears C flag and Z flag, sets N flag
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Y register should remain unchanged
  EXPECT_EQ(cpu.get_y(), 0x00);
}

// Testing different addressing modes

// CPY - Zero Page addressing mode
TEST_F(CPUArithmeticTest, cpy_zero_page) {
  // Load Y register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into Y
  execute_cycles(2);

  // Set up value in zero page
  bus.write(0x42, 0x42);  // Same value as Y register

  // Execute CPY zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPY_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // Equal comparison sets Z flag and C flag, clears N flag
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Y register should remain unchanged
  EXPECT_EQ(cpu.get_y(), 0x42);
}

// CPY - Absolute addressing mode
TEST_F(CPUArithmeticTest, cpy_absolute) {
  // Load Y register with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x42);  // Load 0x42 into Y
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x1234, 0x30);  // Less than Y register

  // Execute CPY absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPY_ABS);
  bus.write(0xFFFF, 0x34);  // Low byte of address
  bus.write(0x0000, 0x12);  // High byte of address
  execute_cycles(4);

  // Y > M sets C flag, clears Z flag and N flag
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Y register should remain unchanged
  EXPECT_EQ(cpu.get_y(), 0x42);
}

// CPY - Verify overflow flag is not affected
TEST_F(CPUArithmeticTest, cpy_overflow_not_affected) {
  // Load Y register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  // Get current overflow state (should be true if we could set it)
  bool overflow_before = cpu.get_flag(nes::Flag::OVERFLOW_);

  // Execute CPY
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CPY_IMM);
  bus.write(0xFFFF, 0x30);
  execute_cycles(2);

  // Overflow flag should remain unchanged
  EXPECT_EQ(cpu.get_flag(nes::Flag::OVERFLOW_), overflow_before);
}

// CPY - Ensure no other registers are affected
TEST_F(CPUArithmeticTest, cpy_other_registers_unchanged) {
  // Set up A and X registers
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xAA);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFF, 0xBB);
  execute_cycles(2);

  // Load Y register
  bus.write(0x0000, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0x0001, 0x42);
  execute_cycles(2);

  // Execute CPY
  bus.write(0x0002, (nes::u8)nes::Opcode::CPY_IMM);
  bus.write(0x0003, 0x30);
  execute_cycles(2);

  // A and X should be unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0xAA);
  EXPECT_EQ(cpu.get_x(), 0xBB);
  EXPECT_EQ(cpu.get_y(), 0x42);
}
