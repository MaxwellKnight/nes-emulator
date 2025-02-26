#include "cpu_test_base.h"

class CPULogicalTest : public CPUTestBase {};

// AND - Basic AND operation with immediate addressing
TEST_F(CPULogicalTest, and_immediate_basic) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xAA);  // Load 0xAA (10101010) into A
  execute_cycles(2);

  // Execute AND immediate
  bus.write(0xFFFE, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0xFFFF, 0x55);  // AND with 0x55 (01010101)
  execute_cycles(2);

  // Result should be 0x00 (00000000)
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// AND - Testing with all bits set
TEST_F(CPULogicalTest, and_all_bits_set) {
  // Load accumulator with all bits set
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xFF);  // Load 0xFF (11111111) into A
  execute_cycles(2);

  // Execute AND immediate with all bits set
  bus.write(0xFFFE, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0xFFFF, 0xFF);  // AND with 0xFF (11111111)
  execute_cycles(2);

  // Result should be 0xFF (11111111)
  EXPECT_EQ(cpu.get_accumulator(), 0xFF);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// AND - Testing with no bits set
TEST_F(CPULogicalTest, and_no_bits_set) {
  // Load accumulator with all bits set
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xFF);  // Load 0xFF (11111111) into A
  execute_cycles(2);

  // Execute AND immediate with no bits set
  bus.write(0xFFFE, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0xFFFF, 0x00);  // AND with 0x00 (00000000)
  execute_cycles(2);

  // Result should be 0x00 (00000000)
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// AND - Testing negative flag set
TEST_F(CPULogicalTest, and_negative_flag) {
  // Load accumulator with value that has bit 7 set
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xFF);  // Load 0xFF (11111111) into A
  execute_cycles(2);

  // Execute AND immediate with value that keeps bit 7 set
  bus.write(0xFFFE, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0xFFFF, 0x80);  // AND with 0x80 (10000000)
  execute_cycles(2);

  // Result should be 0x80 (10000000)
  EXPECT_EQ(cpu.get_accumulator(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// AND - Testing combination of bits
TEST_F(CPULogicalTest, and_combination) {
  // Load accumulator with a bit pattern
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xF0);  // Load 0xF0 (11110000) into A
  execute_cycles(2);

  // Execute AND immediate with different bit pattern
  bus.write(0xFFFE, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0xFFFF, 0x0F);  // AND with 0x0F (00001111)
  execute_cycles(2);

  // Result should be 0x00 (00000000)
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// AND - Testing with specific bit pattern
TEST_F(CPULogicalTest, and_specific_pattern) {
  // Load accumulator with a bit pattern
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xAA);  // Load 0xAA (10101010) into A
  execute_cycles(2);

  // Execute AND immediate with same bit pattern
  bus.write(0xFFFE, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0xFFFF, 0xAA);  // AND with 0xAA (10101010)
  execute_cycles(2);

  // Result should be 0xAA (10101010)
  EXPECT_EQ(cpu.get_accumulator(), 0xAA);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// AND - Testing mask operation
TEST_F(CPULogicalTest, and_mask_operation) {
  // Load accumulator with a value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x5A);  // Load 0x5A (01011010) into A
  execute_cycles(2);

  // Execute AND immediate as a mask to keep only lower 4 bits
  bus.write(0xFFFE, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0xFFFF, 0x0F);  // AND with 0x0F (00001111) - lower 4 bits mask
  execute_cycles(2);

  // Result should be 0x0A (00001010)
  EXPECT_EQ(cpu.get_accumulator(), 0x0A);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// Tests for different addressing modes

// AND - Zero Page addressing mode
TEST_F(CPULogicalTest, and_zero_page) {
  // Load accumulator with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xFF);  // Load 0xFF into A
  execute_cycles(2);

  // Set up value in zero page
  bus.write(0x42, 0x55);  // Put 0x55 at zero page address 0x42

  // Execute AND zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::AND_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // Result: 0xFF AND 0x55 = 0x55
  EXPECT_EQ(cpu.get_accumulator(), 0x55);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// AND - Absolute addressing mode
TEST_F(CPULogicalTest, and_absolute) {
  // Load accumulator with value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xF0);  // Load 0xF0 into A
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x1234, 0x0F);  // Put 0x0F at address 0x1234

  // Execute AND absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::AND_ABS);
  bus.write(0xFFFF, 0x34);  // Low byte of address
  bus.write(0x0000, 0x12);  // High byte of address
  execute_cycles(4);

  // Result: 0xF0 AND 0x0F = 0x00
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// AND - Absolute X-indexed addressing mode
TEST_F(CPULogicalTest, and_absolute_x) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x10);  // X = 0x10
  execute_cycles(2);

  // Load accumulator with value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0xAA);  // Load 0xAA into A
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x1244, 0x55);  // 0x1234 + 0x10 = 0x1244, put 0x55 there

  // Execute AND absolute X-indexed
  bus.write(0x0000, (nes::u8)nes::Opcode::AND_ABX);
  bus.write(0x0001, 0x34);  // Low byte of base address
  bus.write(0x0002, 0x12);  // High byte of base address
  execute_cycles(4);        // No page crossing

  // Result: 0xAA AND 0x55 = 0x00
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// AND - Absolute X-indexed with page crossing
TEST_F(CPULogicalTest, and_absolute_x_page_crossing) {
  // Set X register to cause page crossing
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0xFF);  // X = 0xFF
  execute_cycles(2);

  // Load accumulator with value
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0xF0);  // Load 0xF0 into A
  execute_cycles(2);

  // Set up value in memory
  bus.write(0x12FE, 0x0F);  // 0x1200 + 0xFF = 0x12FF (page crossing), put 0x0F there

  // Execute AND absolute X-indexed with page crossing
  bus.write(0x0000, (nes::u8)nes::Opcode::AND_ABX);
  bus.write(0x0001, 0xFF);  // Low byte of base address
  bus.write(0x0002, 0x11);  // High byte of base address
  execute_cycles(5);        // Page crossing adds 1 cycle

  // Result: 0xF0 AND 0x0F = 0x00
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// AND - Multiple operations in sequence
TEST_F(CPULogicalTest, and_sequence) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xFF);  // Load 0xFF (all bits set) into A
  execute_cycles(2);

  // First AND: 0xFF AND 0xF0 = 0xF0
  bus.write(0xFFFE, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0xFFFF, 0xF0);  // AND with 0xF0 (high nibble mask)
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0xF0);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Second AND: 0xF0 AND 0x33 = 0x30
  bus.write(0x0000, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0x0001, 0x33);  // AND with 0x33 (00110011)
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x30);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Third AND: 0x30 AND 0x0F = 0x00
  bus.write(0x0002, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0x0003, 0x0F);  // AND with 0x0F (low nibble mask)
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// AND - Verify only flags Z and N are affected
TEST_F(CPULogicalTest, and_flags_not_affected) {
  // Set some flags
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);  // Set carry flag
  execute_cycles(2);

  // Load accumulator with value
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0xFF);
  execute_cycles(2);

  // Remember carry flag state (should be set)
  bool carry_before = cpu.get_flag(nes::Flag::CARRY);

  // Execute AND
  bus.write(0xFFFF, (nes::u8)nes::Opcode::AND_IMM);
  bus.write(0x0000, 0x55);
  execute_cycles(2);

  // Carry flag should be unchanged
  EXPECT_EQ(cpu.get_flag(nes::Flag::CARRY), carry_before);
}

// BIT - Basic operation with Zero Page addressing
TEST_F(CPULogicalTest, bit_zero_page_basic) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x0F);  // Load 0x0F (00001111) into A
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x42, 0xF0);  // Put 0xF0 (11110000) at zero page address 0x42

  // Execute BIT zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // Accumulator should be unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0x0F);

  // A & M = 0x0F & 0xF0 = 0x00, so Z flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));

  // Bit 7 of memory is 1, so N flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Bit 6 of memory is 1, so V flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::OVERFLOW_));

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// BIT - Testing with memory value that has bits 6 and 7 clear
TEST_F(CPULogicalTest, bit_flags_clear) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x0F);  // Load 0x0F (00001111) into A
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x42, 0x0F);  // Put 0x0F (00001111) at zero page address 0x42

  // Execute BIT zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // Accumulator should be unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0x0F);

  // A & M = 0x0F & 0x0F = 0x0F, so Z flag should be clear
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));

  // Bit 7 of memory is 0, so N flag should be clear
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Bit 6 of memory is 0, so V flag should be clear
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// BIT - Test with non-zero AND but bits 6 and 7 set
TEST_F(CPULogicalTest, bit_non_zero_and_high_bits_set) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xFF);  // Load 0xFF (11111111) into A
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x42, 0xC3);  // Put 0xC3 (11000011) at zero page address 0x42

  // Execute BIT zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // Accumulator should be unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0xFF);

  // A & M = 0xFF & 0xC3 = 0xC3, so Z flag should be clear
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));

  // Bit 7 of memory is 1, so N flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Bit 6 of memory is 1, so V flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// BIT - Test with zero AND but bit 7 set and bit 6 clear
TEST_F(CPULogicalTest, bit_zero_and_bit7_set_bit6_clear) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x0F);  // Load 0x0F (00001111) into A
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x42, 0x80);  // Put 0x80 (10000000) at zero page address 0x42

  // Execute BIT zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // Accumulator should be unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0x0F);

  // A & M = 0x0F & 0x80 = 0x00, so Z flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));

  // Bit 7 of memory is 1, so N flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Bit 6 of memory is 0, so V flag should be clear
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// BIT - Test with zero AND, bit 7 clear and bit 6 set
TEST_F(CPULogicalTest, bit_zero_and_bit7_clear_bit6_set) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x0F);  // Load 0x0F (00001111) into A
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x42, 0x40);  // Put 0x40 (01000000) at zero page address 0x42

  // Execute BIT zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // Accumulator should be unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0x0F);

  // A & M = 0x0F & 0x40 = 0x00, so Z flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));

  // Bit 7 of memory is 0, so N flag should be clear
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Bit 6 of memory is 1, so V flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// BIT - Test with absolute addressing mode
TEST_F(CPULogicalTest, bit_absolute) {
  // Load accumulator with initial value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xF0);  // Load 0xF0 (11110000) into A
  execute_cycles(2);

  // Set up test value in memory
  bus.write(0x1234, 0xCC);  // Put 0xCC (11001100) at address 0x1234

  // Execute BIT absolute
  bus.write(0xFFFE, (nes::u8)nes::Opcode::BIT_ABS);
  bus.write(0xFFFF, 0x34);  // Low byte of address
  bus.write(0x0000, 0x12);  // High byte of address
  execute_cycles(4);

  // Accumulator should be unchanged
  EXPECT_EQ(cpu.get_accumulator(), 0xF0);

  // A & M = 0xF0 & 0xCC = 0xC0, so Z flag should be clear
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));

  // Bit 7 of memory is 1, so N flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Bit 6 of memory is 1, so V flag should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::OVERFLOW_));
}

// BIT - Verify other flags are not affected
TEST_F(CPULogicalTest, bit_other_flags_not_affected) {
  // Set some flags that BIT should not affect
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);  // Set carry flag
  execute_cycles(2);

  // Load accumulator with value
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0xFF);
  execute_cycles(2);

  // Remember carry flag state (should be set)
  bool carry_before = cpu.get_flag(nes::Flag::CARRY);

  // Set up test value in zero page
  bus.write(0x42, 0x80);  // Put 0x80 (10000000) at zero page address 0x42

  // Execute BIT zero page
  bus.write(0xFFFF, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0x0000, 0x42);  // Zero page address
  execute_cycles(3);

  // Carry flag should be unchanged
  EXPECT_EQ(cpu.get_flag(nes::Flag::CARRY), carry_before);
}

// BIT - Common use case: testing if specific bits are set
TEST_F(CPULogicalTest, bit_testing_specific_bits) {
  // Load accumulator with bit mask (we want to test bit 3)
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x08);  // Load 0x08 (00001000) into A
  execute_cycles(2);

  // Case 1: Bit 3 is set in memory
  bus.write(0x42, 0x08);  // Put 0x08 (00001000) at zero page address 0x42

  // Execute BIT zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // A & M = 0x08 & 0x08 = 0x08, so Z flag should be clear (bit is set)
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));

  // Case 2: Bit 3 is not set in memory
  bus.write(0x43, 0x04);  // Put 0x04 (00000100) at zero page address 0x43

  // Execute BIT zero page
  bus.write(0x0000, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0x0001, 0x43);  // Zero page address
  execute_cycles(3);

  // A & M = 0x08 & 0x04 = 0x00, so Z flag should be set (bit is clear)
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// BIT - Use case: Copying bits 6 and 7 to processor flags
TEST_F(CPULogicalTest, bit_copy_status_bits) {
  // Load accumulator with any value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0xFF);  // Value doesn't matter as long as it won't result in Z=1
  execute_cycles(2);

  // First case: set both flags (bits 6 and 7 set)
  bus.write(0x42, 0xC0);  // Put 0xC0 (11000000) at zero page address 0x42

  // Execute BIT zero page
  bus.write(0xFFFE, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0xFFFF, 0x42);  // Zero page address
  execute_cycles(3);

  // N and V should be set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::OVERFLOW_));

  // Second case: set N, clear V (bit 7 set, bit 6 clear)
  bus.write(0x43, 0x80);  // Put 0x80 (10000000) at zero page address 0x43

  // Execute BIT zero page
  bus.write(0x0000, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0x0001, 0x43);  // Zero page address
  execute_cycles(3);

  // N should be set, V should be clear
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));

  // Third case: clear N, set V (bit 7 clear, bit 6 set)
  bus.write(0x44, 0x40);  // Put 0x40 (01000000) at zero page address 0x44

  // Execute BIT zero page
  bus.write(0x0002, (nes::u8)nes::Opcode::BIT_ZPG);
  bus.write(0x0003, 0x44);  // Zero page address
  execute_cycles(3);

  // N should be clear, V should be set
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::OVERFLOW_));
}
