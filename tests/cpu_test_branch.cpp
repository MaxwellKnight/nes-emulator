#include "cpu_test_base.h"

class CPUBranchTest : public CPUTestBase {
 protected:
  // Simulate full instruction setup and execution
  void execute_bcc_instruction(nes::u8 offset, bool carry_set) {
    // Reset and set PC
    cpu.reset();
    cpu.set_pc(0x1000);

    // Set carry flag based on test condition
    cpu.set_flag(nes::Flag::CARRY, carry_set);

    // Write BCC instruction and offset
    bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BCC_REL));
    bus.write(0x1001, offset);
  }

  void execute_bcs_instruction(nes::u8 offset, bool carry_flag) {
    cpu.reset();
    cpu.set_pc(0x1000);
    cpu.set_flag(nes::Flag::CARRY, carry_flag);

    // Write BCS instruction and offset
    bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BCS_REL));
    bus.write(0x1001, offset);
  }

  void execute_beq_instruction(nes::u8 offset, bool zero_flag) {
    cpu.reset();
    cpu.set_pc(0x1000);
    cpu.set_flag(nes::Flag::ZERO, zero_flag);

    // Write BEQ instruction and offset
    bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BEQ_REL));
    bus.write(0x1001, offset);
  }
};

// Test BCC when Carry flag is clear (branch should be taken)
TEST_F(CPUBranchTest, bcc_rel_carry_clear_branch_taken) {
  // Setup instruction with carry clear and positive offset
  execute_bcc_instruction(0x10, false);

  // Execute instruction cycles
  execute_cycles(3);

  // Verify PC
  EXPECT_EQ(cpu.get_pc(), 0x1012);  // PC + instruction length + offset
}

// Test BCC when Carry flag is set (branch should not be taken)
TEST_F(CPUBranchTest, bcc_rel_carry_clear_branch_not_taken) {
  // Setup instruction with carry set
  execute_bcc_instruction(0x10, true);

  // Execute instruction cycles
  execute_cycles(2);

  // Verify PC
  EXPECT_EQ(cpu.get_pc(), 0x1002);  // PC only moves past instruction
}

// Test BCC with a negative (backward) branch offset
TEST_F(CPUBranchTest, bcc_rel_negative_offset) {
  // Setup instruction with carry clear and negative offset
  execute_bcc_instruction(0xF0, false);  // 0xF0 is two's complement for -16

  // Execute instruction cycles
  execute_cycles(3);

  // Verify PC
  EXPECT_EQ(cpu.get_pc(), 0x0FF2);  // PC + instruction length - 16
}

// Test BCC when branching crosses a page boundary
TEST_F(CPUBranchTest, bcc_rel_page_boundry_cross) {
  // Set PC to a location near page boundary
  cpu.reset();
  cpu.set_pc(0x10F0);

  // Clear carry flag
  cpu.set_flag(nes::Flag::CARRY, false);

  // Write BCC instruction with offset that crosses page boundary
  bus.write(0x10F0, static_cast<nes::u8>(nes::Opcode::BCC_REL));
  bus.write(0x10F1, 0x40);

  // Execute instruction cycles
  // 2 cycles base + 1 cycle for branch taken + 1 cycle for page boundary crossing
  execute_cycles(4);

  // Verify PC
  EXPECT_EQ(cpu.get_pc(), 0x1132);  // PC + instruction length + offset
}

// Test BCC multiple times in sequence
TEST_F(CPUBranchTest, bcc_rel_multiple_consecutive_branches) {
  // First branch with Carry clear
  execute_bcc_instruction(0x10, false);

  // Execute instruction cycles
  execute_cycles(3);

  // Verify first branch
  EXPECT_EQ(cpu.get_pc(), 0x1012);

  // Second branch with Carry set (should not branch)
  execute_bcc_instruction(0x20, true);

  // Execute instruction cycles
  execute_cycles(2);

  // Verify second branch
  EXPECT_EQ(cpu.get_pc(), 0x1002);
}

// BCS - Branch on Carry Set Tests
TEST_F(CPUBranchTest, bcs_rel_carry_set_positive_offset) {
  // Setup instruction with carry set and positive offset
  execute_bcs_instruction(0x10, true);  // +16 offset
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) + 0x10 (offset) = 0x1012
  EXPECT_EQ(cpu.get_pc(), 0x1012);
}

TEST_F(CPUBranchTest, bcs_rel_carry_set_negative_offset) {
  // Setup instruction with carry set and negative offset
  execute_bcs_instruction(0xF0, true);  // 0xF0 is -16 in two's complement
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) - 0x10 (offset) = 0x0FF2
  EXPECT_EQ(cpu.get_pc(), 0x0FF2);
}

TEST_F(CPUBranchTest, bcs_rel_carry_clear) {
  // Setup instruction with carry clear (branch not taken)
  execute_bcs_instruction(0x10, false);
  // Execute instruction cycles (2 base only, no branch)
  execute_cycles(2);
  // Verify PC only advances by instruction length
  EXPECT_EQ(cpu.get_pc(), 0x1002);
}

// Test BCS when branching crosses a page boundary
TEST_F(CPUBranchTest, bcs_rel_page_boundry_cross) {
  // Reset and set PC to a location near page boundary
  cpu.reset();
  cpu.set_pc(0x10F0);

  // Set carry flag
  cpu.set_flag(nes::Flag::CARRY, true);

  // Write BCS instruction with offset that crosses page boundary
  bus.write(0x10F0, static_cast<nes::u8>(nes::Opcode::BCS_REL));
  bus.write(0x10F1, 0x40);

  // Execute 4 cycles (2 base + 1 for branch taken + 1 for page cross)
  execute_cycles(4);

  // Verify PC
  EXPECT_EQ(cpu.get_pc(), 0x1132);

  // Verify no remaining cycles
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// Test BEQ when branching crosses a page boundary
TEST_F(CPUBranchTest, beq_rel_page_boundry_cross) {
  // Reset and set PC to a location near page boundary
  cpu.reset();
  cpu.set_pc(0x10F0);

  // Set zero flag
  cpu.set_flag(nes::Flag::ZERO, true);

  // Write BEQ instruction with offset that crosses page boundary
  bus.write(0x10F0, static_cast<nes::u8>(nes::Opcode::BEQ_REL));
  bus.write(0x10F1, 0x40);

  // Execute 4 cycles (2 base + 1 for branch taken + 1 for page cross)
  execute_cycles(4);

  // Verify PC
  EXPECT_EQ(cpu.get_pc(), 0x1132);

  // Verify no remaining cycles
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
}

// BEQ - Branch on Result Zero Tests
TEST_F(CPUBranchTest, beq_rel_zero_set_positive_offset) {
  // Setup instruction with zero flag set and positive offset
  execute_beq_instruction(0x10, true);  // +16 offset
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) + 0x10 (offset) = 0x1012
  EXPECT_EQ(cpu.get_pc(), 0x1012);
}

TEST_F(CPUBranchTest, beq_rel_zero_set_negative_offset) {
  // Setup instruction with zero flag set and negative offset
  execute_beq_instruction(0xF0, true);  // 0xF0 is -16 in two's complement
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) - 0x10 (offset) = 0x0FF2
  EXPECT_EQ(cpu.get_pc(), 0x0FF2);
}

TEST_F(CPUBranchTest, beq_rel_zero_clear) {
  // Setup instruction with zero flag clear (branch not taken)
  execute_beq_instruction(0x10, false);
  // Execute instruction cycles (2 base only, no branch)
  execute_cycles(2);
  // Verify PC only advances by instruction length
  EXPECT_EQ(cpu.get_pc(), 0x1002);
}
