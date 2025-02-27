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

  void execute_bmi_instruction(nes::u8 offset, bool negative_flag) {
    cpu.reset();
    cpu.set_pc(0x1000);
    cpu.set_flag(nes::Flag::NEGATIVE, negative_flag);

    // Write BMI instruction and offset
    bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BMI_REL));
    bus.write(0x1001, offset);

    // Verify the instruction was written correctly
    EXPECT_EQ(bus.read(0x1000), static_cast<nes::u8>(nes::Opcode::BMI_REL));
    EXPECT_EQ(bus.read(0x1001), offset);
  }

  void execute_bne_instruction(nes::u8 offset, bool zero_flag) {
    cpu.reset();
    cpu.set_pc(0x1000);
    cpu.set_flag(nes::Flag::ZERO, zero_flag);

    // Write BNE instruction and offset
    bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BNE_REL));
    bus.write(0x1001, offset);

    // Verify the instruction was written correctly
    EXPECT_EQ(bus.read(0x1000), static_cast<nes::u8>(nes::Opcode::BNE_REL));
    EXPECT_EQ(bus.read(0x1001), offset);
  }

  void execute_bpl_instruction(nes::u8 offset, bool negative_flag) {
    cpu.reset();
    cpu.set_pc(0x1000);
    cpu.set_flag(nes::Flag::NEGATIVE, negative_flag);

    // Write BPL instruction and offset
    bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BPL_REL));
    bus.write(0x1001, offset);

    // Verify the instruction was written correctly
    EXPECT_EQ(bus.read(0x1000), static_cast<nes::u8>(nes::Opcode::BPL_REL));
    EXPECT_EQ(bus.read(0x1001), offset);
  }

  void execute_bvc_instruction(nes::u8 offset, bool overflow_flag) {
    cpu.reset();
    cpu.set_pc(0x1000);
    cpu.set_flag(nes::Flag::OVERFLOW_, overflow_flag);

    // Write BVC instruction and offset
    bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BVC_REL));
    bus.write(0x1001, offset);

    // Verify the instruction was written correctly
    EXPECT_EQ(bus.read(0x1000), static_cast<nes::u8>(nes::Opcode::BVC_REL));
    EXPECT_EQ(bus.read(0x1001), offset);
  }

  void execute_bvs_instruction(nes::u8 offset, bool overflow_flag) {
    cpu.reset();
    cpu.set_pc(0x1000);
    cpu.set_flag(nes::Flag::OVERFLOW_, overflow_flag);

    // Write BVS instruction and offset
    bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BVS_REL));
    bus.write(0x1001, offset);

    // Verify the instruction was written correctly
    EXPECT_EQ(bus.read(0x1000), static_cast<nes::u8>(nes::Opcode::BVS_REL));
    EXPECT_EQ(bus.read(0x1001), offset);
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

// BMI - Branch on Result Minus (N = 1) Tests
TEST_F(CPUBranchTest, bmi_rel_negative_set_positive_offset) {
  // Setup instruction with negative flag set and positive offset
  execute_bmi_instruction(0x10, true);  // +16 offset
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) + 0x10 (offset) = 0x1012
  EXPECT_EQ(cpu.get_pc(), 0x1012);
}

TEST_F(CPUBranchTest, bmi_rel_negative_set_negative_offset) {
  // Setup instruction with negative flag set and negative offset
  execute_bmi_instruction(0xF0, true);  // 0xF0 is -16 in two's complement
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) - 0x10 (offset) = 0x0FF2
  EXPECT_EQ(cpu.get_pc(), 0x0FF2);
}

TEST_F(CPUBranchTest, bmi_rel_negative_clear) {
  // Setup instruction with negative flag clear (branch not taken)
  execute_bmi_instruction(0x10, false);
  // Execute instruction cycles (2 base only, no branch)
  execute_cycles(2);
  // Verify PC only advances by instruction length
  EXPECT_EQ(cpu.get_pc(), 0x1002);
}

TEST_F(CPUBranchTest, bmi_rel_page_boundary_cross) {
  // Set PC to a location near page boundary
  cpu.reset();
  cpu.set_pc(0x10F0);
  // Set negative flag
  cpu.set_flag(nes::Flag::NEGATIVE, true);
  // Write BMI instruction with offset that crosses page boundary
  bus.write(0x10F0, static_cast<nes::u8>(nes::Opcode::BMI_REL));
  bus.write(0x10F1, 0x40);  // +64 offset

  // Verify instruction was written properly
  EXPECT_EQ(bus.read(0x10F0), static_cast<nes::u8>(nes::Opcode::BMI_REL));
  EXPECT_EQ(bus.read(0x10F1), 0x40);

  // Execute instruction cycles (2 base + 1 branch taken + 1 page cross)
  execute_cycles(4);
  // Verify PC: 0x10F0 (base) + 0x2 (instruction length) + 0x40 (offset) = 0x1132
  EXPECT_EQ(cpu.get_pc(), 0x1132);
}

// BNE - Branch on Result Not Zero (Z = 0) Tests

TEST_F(CPUBranchTest, bne_rel_zero_clear_positive_offset) {
  // Setup instruction with zero flag clear and positive offset
  execute_bne_instruction(0x10, false);  // +16 offset
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) + 0x10 (offset) = 0x1012
  EXPECT_EQ(cpu.get_pc(), 0x1012);
}

TEST_F(CPUBranchTest, bne_rel_zero_clear_negative_offset) {
  // Setup instruction with zero flag clear and negative offset
  execute_bne_instruction(0xF0, false);  // 0xF0 is -16 in two's complement
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) - 0x10 (offset) = 0x0FF2
  EXPECT_EQ(cpu.get_pc(), 0x0FF2);
}

TEST_F(CPUBranchTest, bne_rel_zero_set) {
  // Setup instruction with zero flag set (branch not taken)
  execute_bne_instruction(0x10, true);
  // Execute instruction cycles (2 base only, no branch)
  execute_cycles(2);
  // Verify PC only advances by instruction length
  EXPECT_EQ(cpu.get_pc(), 0x1002);
}

TEST_F(CPUBranchTest, bne_rel_page_boundary_cross) {
  // Set PC to a location near page boundary
  cpu.reset();
  cpu.set_pc(0x10F0);
  // Clear zero flag
  cpu.set_flag(nes::Flag::ZERO, false);
  // Write BNE instruction with offset that crosses page boundary
  bus.write(0x10F0, static_cast<nes::u8>(nes::Opcode::BNE_REL));
  bus.write(0x10F1, 0x40);  // +64 offset

  // Verify instruction was written properly
  EXPECT_EQ(bus.read(0x10F0), static_cast<nes::u8>(nes::Opcode::BNE_REL));
  EXPECT_EQ(bus.read(0x10F1), 0x40);

  // Execute instruction cycles (2 base + 1 branch taken + 1 page cross)
  execute_cycles(4);
  // Verify PC: 0x10F0 (base) + 0x2 (instruction length) + 0x40 (offset) = 0x1132
  EXPECT_EQ(cpu.get_pc(), 0x1132);
}

// BPL - Branch on Result Plus (N = 0) Tests
TEST_F(CPUBranchTest, bpl_rel_negative_clear_positive_offset) {
  // Setup instruction with negative flag clear and positive offset
  execute_bpl_instruction(0x10, false);  // +16 offset
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) + 0x10 (offset) = 0x1012
  EXPECT_EQ(cpu.get_pc(), 0x1012);
}

TEST_F(CPUBranchTest, bpl_rel_negative_clear_negative_offset) {
  // Setup instruction with negative flag clear and negative offset
  execute_bpl_instruction(0xF0, false);  // 0xF0 is -16 in two's complement
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) - 0x10 (offset) = 0x0FF2
  EXPECT_EQ(cpu.get_pc(), 0x0FF2);
}

TEST_F(CPUBranchTest, bpl_rel_negative_set) {
  // Setup instruction with negative flag set (branch not taken)
  execute_bpl_instruction(0x10, true);
  // Execute instruction cycles (2 base only, no branch)
  execute_cycles(2);
  // Verify PC only advances by instruction length
  EXPECT_EQ(cpu.get_pc(), 0x1002);
}

TEST_F(CPUBranchTest, bpl_rel_page_boundary_cross) {
  // Set PC to a location near page boundary
  cpu.reset();
  cpu.set_pc(0x10F0);
  // Clear negative flag
  cpu.set_flag(nes::Flag::NEGATIVE, false);
  // Write BPL instruction with offset that crosses page boundary
  bus.write(0x10F0, static_cast<nes::u8>(nes::Opcode::BPL_REL));
  bus.write(0x10F1, 0x40);  // +64 offset

  // Verify instruction was written properly
  EXPECT_EQ(bus.read(0x10F0), static_cast<nes::u8>(nes::Opcode::BPL_REL));
  EXPECT_EQ(bus.read(0x10F1), 0x40);

  // Execute instruction cycles (2 base + 1 branch taken + 1 page cross)
  execute_cycles(4);
  // Verify PC: 0x10F0 (base) + 0x2 (instruction length) + 0x40 (offset) = 0x1132
  EXPECT_EQ(cpu.get_pc(), 0x1132);
}

// BVC - Branch on Overflow Clear (V = 0) Tests
TEST_F(CPUBranchTest, bvc_rel_overflow_clear_positive_offset) {
  // Setup instruction with overflow flag clear and positive offset
  execute_bvc_instruction(0x10, false);  // +16 offset
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) + 0x10 (offset) = 0x1012
  EXPECT_EQ(cpu.get_pc(), 0x1012);
}

TEST_F(CPUBranchTest, bvc_rel_overflow_clear_negative_offset) {
  // Setup instruction with overflow flag clear and negative offset
  execute_bvc_instruction(0xF0, false);  // 0xF0 is -16 in two's complement
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) - 0x10 (offset) = 0x0FF2
  EXPECT_EQ(cpu.get_pc(), 0x0FF2);
}

TEST_F(CPUBranchTest, bvc_rel_overflow_set) {
  // Setup instruction with overflow flag set (branch not taken)
  execute_bvc_instruction(0x10, true);
  // Execute instruction cycles (2 base only, no branch)
  execute_cycles(2);
  // Verify PC only advances by instruction length
  EXPECT_EQ(cpu.get_pc(), 0x1002);
}

TEST_F(CPUBranchTest, bvc_rel_page_boundary_cross) {
  // Set PC to a location near page boundary
  cpu.reset();
  cpu.set_pc(0x10F0);
  // Clear overflow flag
  cpu.set_flag(nes::Flag::OVERFLOW_, false);
  // Write BVC instruction with offset that crosses page boundary
  bus.write(0x10F0, static_cast<nes::u8>(nes::Opcode::BVC_REL));
  bus.write(0x10F1, 0x40);  // +64 offset

  // Verify instruction was written properly
  EXPECT_EQ(bus.read(0x10F0), static_cast<nes::u8>(nes::Opcode::BVC_REL));
  EXPECT_EQ(bus.read(0x10F1), 0x40);

  // Execute instruction cycles (2 base + 1 branch taken + 1 page cross)
  execute_cycles(4);
  // Verify PC: 0x10F0 (base) + 0x2 (instruction length) + 0x40 (offset) = 0x1132
  EXPECT_EQ(cpu.get_pc(), 0x1132);
}

TEST_F(CPUBranchTest, bvc_rel_leaves_other_flags_unchanged) {
  // Setup flags with known values
  cpu.reset();
  cpu.set_pc(0x1000);
  cpu.set_flag(nes::Flag::OVERFLOW_, false);  // Clear V flag for branch to be taken
  cpu.set_flag(nes::Flag::CARRY, true);       // Set some other flags
  cpu.set_flag(nes::Flag::ZERO, true);
  cpu.set_flag(nes::Flag::NEGATIVE, true);
  nes::u8 initial_status = cpu.get_status();

  // Write BVC instruction and offset
  bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BVC_REL));
  bus.write(0x1001, 0x10);

  // Execute instruction
  execute_cycles(3);

  // Verify other flags weren't affected
  EXPECT_EQ(cpu.get_status(), initial_status);
}

// BVS - Branch on Overflow Set (V = 1) Tests
TEST_F(CPUBranchTest, bvs_rel_overflow_set_positive_offset) {
  // Setup instruction with overflow flag set and positive offset
  execute_bvs_instruction(0x10, true);  // +16 offset
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) + 0x10 (offset) = 0x1012
  EXPECT_EQ(cpu.get_pc(), 0x1012);
}

TEST_F(CPUBranchTest, bvs_rel_overflow_set_negative_offset) {
  // Setup instruction with overflow flag set and negative offset
  execute_bvs_instruction(0xF0, true);  // 0xF0 is -16 in two's complement
  // Execute instruction cycles (2 base + 1 branch taken)
  execute_cycles(3);
  // Verify PC: 0x1000 (base) + 0x2 (instruction length) - 0x10 (offset) = 0x0FF2
  EXPECT_EQ(cpu.get_pc(), 0x0FF2);
}

TEST_F(CPUBranchTest, bvs_rel_overflow_clear) {
  // Setup instruction with overflow flag clear (branch not taken)
  execute_bvs_instruction(0x10, false);
  // Execute instruction cycles (2 base only, no branch)
  execute_cycles(2);
  // Verify PC only advances by instruction length
  EXPECT_EQ(cpu.get_pc(), 0x1002);
}

TEST_F(CPUBranchTest, bvs_rel_page_boundary_cross) {
  // Set PC to a location near page boundary
  cpu.reset();
  cpu.set_pc(0x10F0);
  // Set overflow flag
  cpu.set_flag(nes::Flag::OVERFLOW_, true);
  // Write BVS instruction with offset that crosses page boundary
  bus.write(0x10F0, static_cast<nes::u8>(nes::Opcode::BVS_REL));
  bus.write(0x10F1, 0x40);  // +64 offset

  // Verify instruction was written properly
  EXPECT_EQ(bus.read(0x10F0), static_cast<nes::u8>(nes::Opcode::BVS_REL));
  EXPECT_EQ(bus.read(0x10F1), 0x40);

  // Execute instruction cycles (2 base + 1 branch taken + 1 page cross)
  execute_cycles(4);
  // Verify PC: 0x10F0 (base) + 0x2 (instruction length) + 0x40 (offset) = 0x1132
  EXPECT_EQ(cpu.get_pc(), 0x1132);
}

TEST_F(CPUBranchTest, bvs_rel_leaves_other_flags_unchanged) {
  // Setup flags with known values
  cpu.reset();
  cpu.set_pc(0x1000);
  cpu.set_flag(nes::Flag::OVERFLOW_, true);  // Set V flag for branch to be taken
  cpu.set_flag(nes::Flag::CARRY, true);      // Set some other flags
  cpu.set_flag(nes::Flag::ZERO, false);
  cpu.set_flag(nes::Flag::NEGATIVE, true);
  nes::u8 initial_status = cpu.get_status();

  // Write BVS instruction and offset
  bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BVS_REL));
  bus.write(0x1001, 0x10);

  execute_cycles(3);
  EXPECT_EQ(cpu.get_status(), initial_status);
}

TEST_F(CPUBranchTest, bvc_simple) {
  cpu.reset();
  cpu.set_pc(0x0200);
  cpu.set_flag(nes::Flag::OVERFLOW_, false);

  // Write a BVC instruction with a small positive offset
  bus.write(0x0200, static_cast<nes::u8>(nes::Opcode::BVC_REL));
  bus.write(0x0201, 0x04);  // Branch forward 4 bytes

  execute_cycles(3);

  EXPECT_EQ(cpu.get_pc(), 0x0206);
}

TEST_F(CPUBranchTest, bvs_simple) {
  cpu.reset();
  cpu.set_pc(0x0200);
  cpu.set_flag(nes::Flag::OVERFLOW_, true);

  // Write a BVS instruction with a small positive offset
  bus.write(0x0200, static_cast<nes::u8>(nes::Opcode::BVS_REL));
  bus.write(0x0201, 0x04);  // Branch forward 4 bytes

  execute_cycles(3);
  EXPECT_EQ(cpu.get_pc(), 0x0206);
}

// Simplified BVC test for branch not taken
TEST_F(CPUBranchTest, bvc_simple_not_taken) {
  // Set up a simple BVC instruction
  cpu.reset();
  cpu.set_pc(0x0200);
  cpu.set_flag(nes::Flag::OVERFLOW_, true);  // Set overflow flag - branch should not be taken

  // Write a BVC instruction with a small positive offset
  bus.write(0x0200, static_cast<nes::u8>(nes::Opcode::BVC_REL));
  bus.write(0x0201, 0x04);  // Branch forward 4 bytes (but should be ignored)

  execute_cycles(2);

  EXPECT_EQ(cpu.get_pc(), 0x0202);
}

// Fix for consecutive branches test
TEST_F(CPUBranchTest, bvc_bvs_consecutive_branches) {
  // First setup BVC with overflow clear (branch taken)
  cpu.reset();
  cpu.set_pc(0x1000);
  cpu.set_flag(nes::Flag::OVERFLOW_, false);
  bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::BVC_REL));
  bus.write(0x1001, 0x10);
  execute_cycles(3);
  EXPECT_EQ(cpu.get_pc(), 0x1012);

  // Then BVS with overflow still clear (branch not taken)
  cpu.reset();
  cpu.set_pc(0x1100);
  cpu.set_flag(nes::Flag::OVERFLOW_, false);
  bus.write(0x1100, static_cast<nes::u8>(nes::Opcode::BVS_REL));
  bus.write(0x1101, 0x20);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_pc(), 0x1102);

  // Now BVC with overflow set (branch not taken)
  cpu.reset();
  cpu.set_pc(0x1200);
  cpu.set_flag(nes::Flag::OVERFLOW_, true);
  bus.write(0x1200, static_cast<nes::u8>(nes::Opcode::BVC_REL));
  bus.write(0x1201, 0x10);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_pc(), 0x1202);

  // Finally BVS with overflow set (branch taken)
  cpu.reset();
  cpu.set_pc(0x1300);
  cpu.set_flag(nes::Flag::OVERFLOW_, true);
  bus.write(0x1300, static_cast<nes::u8>(nes::Opcode::BVS_REL));
  bus.write(0x1301, 0x20);
  execute_cycles(3);
  EXPECT_EQ(cpu.get_pc(), 0x1322);
}
