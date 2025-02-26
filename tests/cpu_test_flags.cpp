#include "cpu_test_base.h"

class CPUFlagTest : public CPUTestBase {};

// CLC_IMP - Clear Carry Flag (0x18)
TEST_F(CPUFlagTest, clc_clears_carry_flag) {
  // Set the carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Verify carry flag is set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));

  // Execute CLC_IMP instruction
  bus.write(0xFFFD, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);  // CLC_IMP takes 2 cycles

  // Verify carry flag was cleared
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUFlagTest, clc_leaves_other_flags_unchanged) {
  // Set up initial flags
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);  // Set carry flag
  execute_cycles(2);

  // Load a negative value to set the negative flag
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x80);  // Load 0x80 into A to set negative flag
  execute_cycles(2);

  // Remember the initial status
  nes::u8 initialStatus = cpu.get_status();

  // Execute CLC_IMP instruction
  bus.write(0xFFFF, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Verify only carry flag was affected
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_EQ(cpu.get_status() & ~(nes::u8)nes::Flag::CARRY, initialStatus & ~(nes::u8)nes::Flag::CARRY);
}

// SEC_IMP - Set Carry Flag (0x38)
TEST_F(CPUFlagTest, sec_sets_carry_flag) {
  // Clear the carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Verify carry flag is cleared
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Execute SEC_IMP instruction
  bus.write(0xFFFD, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);  // SEC_IMP takes 2 cycles

  // Verify carry flag was set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUFlagTest, sec_leaves_other_flags_unchanged) {
  // Set up initial flags
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);  // Clear carry flag
  execute_cycles(2);

  // Load a negative value to set the negative flag
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x80);  // Load 0x80 into A to set negative flag
  execute_cycles(2);

  // Remember the initial status
  nes::u8 initialStatus = cpu.get_status();

  // Execute SEC_IMP instruction
  bus.write(0xFFFF, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Verify only carry flag was affected
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_EQ(cpu.get_status() & ~(nes::u8)nes::Flag::CARRY, initialStatus & ~(nes::u8)nes::Flag::CARRY);
}

// Test both CLC_IMP and SEC_IMP in sequence
TEST_F(CPUFlagTest, clc_sec_sequence) {
  // Execute CLC_IMP
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  bus.write(0xFFFD, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));

  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
}

// Test that CLC_IMP and SEC_IMP don't affect PC beyond normal instruction execution
TEST_F(CPUFlagTest, clc_sec_pc_advancement) {
  // Get initial PC
  nes::u16 initial_pc = cpu.get_pc();

  // Execute CLC_IMP
  bus.write(initial_pc, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Check PC advanced by 1 byte
  EXPECT_EQ(cpu.get_pc(), initial_pc + 1);

  // Execute SEC_IMP
  nes::u16 sec_pc = cpu.get_pc();
  bus.write(sec_pc, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Check PC advanced by 1 byte
  EXPECT_EQ(cpu.get_pc(), sec_pc + 1);
}

// CLI - Clear Interrupt Disable Flag (0x58)
TEST_F(CPUFlagTest, cli_clears_interrupt_flag) {
  // Set the interrupt flag initially (using SEI)
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEI_IMP);
  execute_cycles(2);

  // Verify interrupt flag is set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE));

  // Execute CLI instruction
  bus.write(0xFFFD, (nes::u8)nes::Opcode::CLI_IMP);
  execute_cycles(2);  // CLI takes 2 cycles

  // Verify interrupt flag was cleared
  EXPECT_FALSE(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUFlagTest, cli_leaves_other_flags_unchanged) {
  // Set up initial flags
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEI_IMP);  // Set interrupt flag
  execute_cycles(2);

  // Set carry flag
  bus.write(0xFFFD, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Load a negative value to set the negative flag
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x80);  // Load 0x80 into A to set negative flag
  execute_cycles(2);

  // Remember the initial status
  nes::u8 initialStatus = cpu.get_status();

  // Execute CLI instruction
  bus.write(0x0000, (nes::u8)nes::Opcode::CLI_IMP);
  execute_cycles(2);

  // Verify only interrupt flag was affected
  EXPECT_FALSE(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE));
  EXPECT_EQ(cpu.get_status() & ~(nes::u8)nes::Flag::INTERRUPT_DISABLE, initialStatus & ~(nes::u8)nes::Flag::INTERRUPT_DISABLE);
}

// CLD - Clear Decimal Mode Flag (0xD8)
TEST_F(CPUFlagTest, cld_clears_decimal_flag) {
  // Set the decimal flag initially (using SED)
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SED_IMP);
  execute_cycles(2);

  // Verify decimal flag is set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::DECIMAL));

  // Execute CLD instruction
  bus.write(0xFFFD, (nes::u8)nes::Opcode::CLD_IMP);
  execute_cycles(2);  // CLD takes 2 cycles

  // Verify decimal flag was cleared
  EXPECT_FALSE(cpu.get_flag(nes::Flag::DECIMAL));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUFlagTest, cld_leaves_other_flags_unchanged) {
  // Set up initial flags
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SED_IMP);  // Set decimal flag
  execute_cycles(2);

  // Set carry flag
  bus.write(0xFFFD, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Load a negative value to set the negative flag
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFF, 0x80);  // Load 0x80 into A to set negative flag
  execute_cycles(2);

  // Remember the initial status
  nes::u8 initialStatus = cpu.get_status();

  // Execute CLD instruction
  bus.write(0x0000, (nes::u8)nes::Opcode::CLD_IMP);
  execute_cycles(2);

  // Verify only decimal flag was affected
  EXPECT_FALSE(cpu.get_flag(nes::Flag::DECIMAL));
  EXPECT_EQ(cpu.get_status() & ~(nes::u8)nes::Flag::DECIMAL, initialStatus & ~(nes::u8)nes::Flag::DECIMAL);
}

// CLV - Clear Overflow Flag (0xB8)
TEST_F(CPUFlagTest, clv_clears_overflow_flag) {
  // Need to set overflow flag through arithmetic operation
  // Load values that will cause overflow in ADC
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);  // Clear carry first
  execute_cycles(2);

  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x50);  // Load 0x50 (+80) into A
  execute_cycles(2);

  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x50);  // Add 0x50 (+80) to A, causing overflow (result is 0xA0)
  execute_cycles(2);

  // Verify overflow flag is set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::OVERFLOW_));

  // Execute CLV instruction
  bus.write(0x0001, (nes::u8)nes::Opcode::CLV_IMP);
  execute_cycles(2);  // CLV takes 2 cycles

  // Verify overflow flag was cleared
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUFlagTest, clv_leaves_other_flags_unchanged) {
  // Set up overflow and other flags
  // Clear carry first
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Load a value that will cause overflow
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x50);  // Load 0x50 (+80) into A
  execute_cycles(2);

  // Add value to cause overflow
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ADC_IMM);
  bus.write(0x0000, 0x50);  // Add 0x50 (+80) to A, causing overflow
  execute_cycles(2);

  // Also set carry flag
  bus.write(0x0001, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Remember the initial status
  nes::u8 initialStatus = cpu.get_status();

  // Execute CLV instruction
  bus.write(0x0002, (nes::u8)nes::Opcode::CLV_IMP);
  execute_cycles(2);

  // Verify only overflow flag was affected
  EXPECT_FALSE(cpu.get_flag(nes::Flag::OVERFLOW_));
  EXPECT_EQ(cpu.get_status() & ~(nes::u8)nes::Flag::OVERFLOW_, initialStatus & ~(nes::u8)nes::Flag::OVERFLOW_);
}

// Test a sequence of all flag clear operations
TEST_F(CPUFlagTest, flag_clear_sequence) {
  // Reset for a clean start
  cpu.reset();
  nes::u16 pc = cpu.get_pc();  // Should be 0xFFFC

  // Set all flags that can be cleared
  bus.write(pc++, (nes::u8)nes::Opcode::SEC_IMP);  // Set carry
  bus.write(pc++, (nes::u8)nes::Opcode::SEI_IMP);  // Set interrupt disable
  bus.write(pc++, (nes::u8)nes::Opcode::SED_IMP);  // Set decimal

  // Execute the 3 SET instructions
  execute_cycles(6);  // 3 instructions × 2 cycles each

  // Verify all flags are set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::DECIMAL));

  pc = cpu.get_pc();  // Get current PC after executing the SET instructions
  bus.write(pc++, (nes::u8)nes::Opcode::CLC_IMP);
  bus.write(pc++, (nes::u8)nes::Opcode::CLI_IMP);
  bus.write(pc++, (nes::u8)nes::Opcode::CLD_IMP);

  // Execute the instructions one at a time to verify each step
  execute_cycles(2);  // CLC
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::DECIMAL));

  execute_cycles(2);  // CLI
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::DECIMAL));

  execute_cycles(2);  // CLD
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::DECIMAL));
}
