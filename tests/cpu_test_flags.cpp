#include "cpu_test_base.h"

class CPUFlagTest : public CPUTestBase {};

// CLC - Clear Carry Flag (0x18)
TEST_F(CPUFlagTest, clc_clears_carry_flag) {
  // Set the carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC);
  execute_cycles(2);

  // Verify carry flag is set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));

  // Execute CLC instruction
  bus.write(0xFFFD, (nes::u8)nes::Opcode::CLC);
  execute_cycles(2);  // CLC takes 2 cycles

  // Verify carry flag was cleared
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUFlagTest, clc_leaves_other_flags_unchanged) {
  // Set up initial flags
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC);  // Set carry flag
  execute_cycles(2);

  // Load a negative value to set the negative flag
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFE, 0x80);  // Load 0x80 into A to set negative flag
  execute_cycles(2);

  // Remember the initial status
  nes::u8 initialStatus = cpu.get_status();

  // Execute CLC instruction
  bus.write(0xFFFF, (nes::u8)nes::Opcode::CLC);
  execute_cycles(2);

  // Verify only carry flag was affected
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_EQ(cpu.get_status() & ~(nes::u8)nes::Flag::CARRY, initialStatus & ~(nes::u8)nes::Flag::CARRY);
}

// SEC - Set Carry Flag (0x38)
TEST_F(CPUFlagTest, sec_sets_carry_flag) {
  // Clear the carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC);
  execute_cycles(2);

  // Verify carry flag is cleared
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Execute SEC instruction
  bus.write(0xFFFD, (nes::u8)nes::Opcode::SEC);
  execute_cycles(2);  // SEC takes 2 cycles

  // Verify carry flag was set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

TEST_F(CPUFlagTest, sec_leaves_other_flags_unchanged) {
  // Set up initial flags
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC);  // Clear carry flag
  execute_cycles(2);

  // Load a negative value to set the negative flag
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IM);
  bus.write(0xFFFE, 0x80);  // Load 0x80 into A to set negative flag
  execute_cycles(2);

  // Remember the initial status
  nes::u8 initialStatus = cpu.get_status();

  // Execute SEC instruction
  bus.write(0xFFFF, (nes::u8)nes::Opcode::SEC);
  execute_cycles(2);

  // Verify only carry flag was affected
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_EQ(cpu.get_status() & ~(nes::u8)nes::Flag::CARRY, initialStatus & ~(nes::u8)nes::Flag::CARRY);
}

// Test both CLC and SEC in sequence
TEST_F(CPUFlagTest, clc_sec_sequence) {
  // Execute CLC
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC);
  execute_cycles(2);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  bus.write(0xFFFD, (nes::u8)nes::Opcode::SEC);
  execute_cycles(2);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));

  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC);
  execute_cycles(2);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
}

// Test that CLC and SEC don't affect PC beyond normal instruction execution
TEST_F(CPUFlagTest, clc_sec_pc_advancement) {
  // Get initial PC
  nes::u16 initial_pc = cpu.get_pc();

  // Execute CLC
  bus.write(initial_pc, (nes::u8)nes::Opcode::CLC);
  execute_cycles(2);

  // Check PC advanced by 1 byte
  EXPECT_EQ(cpu.get_pc(), initial_pc + 1);

  // Execute SEC
  nes::u16 sec_pc = cpu.get_pc();
  bus.write(sec_pc, (nes::u8)nes::Opcode::SEC);
  execute_cycles(2);

  // Check PC advanced by 1 byte
  EXPECT_EQ(cpu.get_pc(), sec_pc + 1);
}
