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
