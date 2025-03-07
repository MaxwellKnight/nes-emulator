#include "cpu_test_base.h"

class CPUControlFlowTest : public CPUTestBase {
 protected:
  void execute_jmp_abs_instruction(nes::u16 address) {
    // Reset and set PC
    cpu.reset();
    cpu.set_pc(0x1000);

    // Write JMP_ABS instruction and address
    bus.write(0x1000, static_cast<nes::u8>(nes::Opcode::JMP_ABS));
    bus.write(0x1001, static_cast<nes::u8>(address & 0xFF));         // Low byte
    bus.write(0x1002, static_cast<nes::u8>((address >> 8) & 0xFF));  // High byte
  }

  void execute_jmp_ind_instruction(nes::u16 address, nes::u16 indirect_address) {
    // Reset and set PC
    cpu.reset();
    cpu.set_pc(address);
    // Write JMP_IND instruction and address
    bus.write(address, static_cast<nes::u8>(nes::Opcode::JMP_IND));
    bus.write(address + 1, static_cast<nes::u8>(indirect_address & 0xFF));         // Low byte
    bus.write(address + 2, static_cast<nes::u8>((indirect_address >> 8) & 0xFF));  // High byte
  }
};

TEST_F(CPUControlFlowTest, jmp_abs) {
  // Setup JMP_ABS to address 0x1234
  execute_jmp_abs_instruction(0x1234);

  // Execute instruction cycles
  execute_cycles(3);

  // Verify PC set to absolute address
  EXPECT_EQ(cpu.get_pc(), 0x1234);
}

TEST_F(CPUControlFlowTest, jmp_ind_page_boundary_bug) {
  // Set up memory at the wrap-around boundary
  bus.write(0x07FF, 0x80);
  bus.write(0x0700, 0x50);  // This simulates the wrap-around to 0x0700

  // Setup JMP_IND instruction
  cpu.reset();
  cpu.set_pc(0xFFFC);
  bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::JMP_IND));
  bus.write(0xFFFD, 0xFF);  // Low byte of indirect address
  bus.write(0xFFFE, 0x07);  // High byte of indirect address

  // Execute instruction cycles
  execute_cycles(5);

  // Verify PC is set to 0x5080
  EXPECT_EQ(cpu.get_pc(), 0x5080);
}

// BRK - Break Command (0x00)
TEST_F(CPUControlFlowTest, brk_pushes_pc_plus_two_to_stack) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x8000;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Execute BRK instruction
  bus.write(initial_pc, (nes::u8)nes::Opcode::BRK_IMP);
  execute_cycles(7);  // BRK takes 7 cycles

  // Calculate expected values
  nes::u16 expected_pushed_pc = initial_pc + 2;
  nes::u8 expected_sp = initial_sp - 3;  // PC (2 bytes) + status register (1 byte)

  // Check that SP was decremented appropriately
  EXPECT_EQ(cpu.get_sp(), expected_sp);

  // According to 6502 behavior, PCH is pushed first, then PCL, then status
  // So they should be at SP+3, SP+2, and SP+1 respectively
  nes::u8 pch = bus.read(0x0100 + expected_sp + 3);
  nes::u8 pcl = bus.read(0x0100 + expected_sp + 2);
  nes::u16 pushed_pc = (pch << 8) | pcl;

  // Verify PC+2 was pushed to the stack
  EXPECT_EQ(pushed_pc, expected_pushed_pc);
}

TEST_F(CPUControlFlowTest, brk_pushes_status_register_to_stack) {
  // Set up initial PC and status register
  cpu.reset();
  cpu.set_pc(0x8000);

  // Set some flags to test they are pushed correctly
  cpu.set_flag(nes::Flag::CARRY, true);
  cpu.set_flag(nes::Flag::DECIMAL, true);
  nes::u8 initial_status = cpu.get_status();
  nes::u8 initial_sp = cpu.get_sp();

  // Execute BRK instruction
  bus.write(0x8000, (nes::u8)nes::Opcode::BRK_IMP);
  execute_cycles(7);

  // The status register is pushed after PCH and PCL, so it should be at SP+1
  // where SP is the final stack pointer after BRK
  nes::u8 final_sp = initial_sp - 3;
  nes::u8 pushed_status = bus.read(0x0100 + final_sp + 1);

  // The status should have BREAK and UNUSED flags set when pushed to stack
  nes::u8 expected_status = initial_status | (nes::u8)nes::Flag::BREAK | (nes::u8)nes::Flag::UNUSED;

  EXPECT_EQ(pushed_status, expected_status);
}

TEST_F(CPUControlFlowTest, brk_sets_interrupt_disable_flag) {
  // Set up initial PC and clear interrupt flag
  cpu.reset();
  cpu.set_pc(0x8000);
  cpu.set_flag(nes::Flag::INTERRUPT_DISABLE, false);

  // Verify interrupt disable flag is cleared
  EXPECT_FALSE(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE));

  // Execute BRK instruction
  bus.write(0x8000, (nes::u8)nes::Opcode::BRK_IMP);
  execute_cycles(7);

  // Verify interrupt disable flag was set
  EXPECT_TRUE(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE));
}

TEST_F(CPUControlFlowTest, brk_loads_interrupt_vector_to_pc) {
  // Set up initial PC
  cpu.reset();
  cpu.set_pc(0x8000);

  // Set up interrupt vector at $FFFE-$FFFF
  nes::u16 interrupt_vector = 0x1234;
  bus.write(0xFFFE, interrupt_vector & 0xFF);         // Low byte
  bus.write(0xFFFF, (interrupt_vector >> 8) & 0xFF);  // High byte

  // Execute BRK instruction
  bus.write(0x8000, (nes::u8)nes::Opcode::BRK_IMP);
  execute_cycles(7);

  // Verify PC was loaded with interrupt vector
  EXPECT_EQ(cpu.get_pc(), interrupt_vector);
}

TEST_F(CPUControlFlowTest, brk_takes_seven_cycles) {
  // Set up initial PC
  cpu.reset();
  cpu.set_pc(0x8000);

  // Execute BRK instruction with exactly 7 cycles
  bus.write(0x8000, (nes::u8)nes::Opcode::BRK_IMP);
  execute_cycles(7);

  // Verify all cycles were consumed
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);

  // Verify the instruction completed (PC should be at interrupt vector)
  EXPECT_NE(cpu.get_pc(), 0x8001);  // PC should not be at the next instruction

  // Try with fewer cycles (6) to confirm it doesn't complete
  cpu.reset();
  cpu.set_pc(0x8000);
  bus.write(0x8000, (nes::u8)nes::Opcode::BRK_IMP);
  execute_cycles(6);

  // Verify instruction isn't complete yet
  EXPECT_EQ(cpu.get_remaining_cycles(), 1);
}

TEST_F(CPUControlFlowTest, brk_leaves_other_registers_unchanged) {
  // Set up initial register values
  cpu.reset();
  cpu.set_pc(0x8000);

  // Set up a value in accumulator and X/Y registers
  bus.write(0x8000, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0x8001, 0x42);
  execute_cycles(2);

  bus.write(0x8002, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0x8003, 0x69);
  execute_cycles(2);

  bus.write(0x8004, (nes::u8)nes::Opcode::LDY_IMM);
  bus.write(0x8005, 0x55);
  execute_cycles(2);

  // Save register values before BRK
  nes::u8 a_before = cpu.get_accumulator();
  nes::u8 x_before = cpu.get_x();
  nes::u8 y_before = cpu.get_y();

  // Execute BRK instruction
  bus.write(0x8006, (nes::u8)nes::Opcode::BRK_IMP);
  execute_cycles(7);

  // Verify A, X, and Y registers weren't affected
  EXPECT_EQ(cpu.get_accumulator(), a_before);
  EXPECT_EQ(cpu.get_x(), x_before);
  EXPECT_EQ(cpu.get_y(), y_before);
}

TEST_F(CPUControlFlowTest, brk_leaves_other_flags_unchanged) {
  // Set up initial PC
  cpu.reset();
  cpu.set_pc(0x8000);

  // Set some flags but not others
  cpu.set_flag(nes::Flag::CARRY, true);
  cpu.set_flag(nes::Flag::ZERO, true);
  cpu.set_flag(nes::Flag::NEGATIVE, false);
  cpu.set_flag(nes::Flag::DECIMAL, true);
  cpu.set_flag(nes::Flag::OVERFLOW_, false);

  // Save flags before BRK (except INTERRUPT_DISABLE and BREAK which we expect to change)
  nes::u8 flags_before = cpu.get_status() & ~((nes::u8)nes::Flag::INTERRUPT_DISABLE | (nes::u8)nes::Flag::BREAK);

  // Execute BRK instruction
  bus.write(0x8000, (nes::u8)nes::Opcode::BRK_IMP);
  execute_cycles(7);

  // Verify flags weren't affected (except INTERRUPT_DISABLE and BREAK)
  nes::u8 flags_after = cpu.get_status() & ~((nes::u8)nes::Flag::INTERRUPT_DISABLE | (nes::u8)nes::Flag::BREAK);
  EXPECT_EQ(flags_after, flags_before);
}

// JSR Instruction Tests
TEST_F(CPUControlFlowTest, jsr_pushes_return_address_to_stack) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Write JSR instruction with target address
  // JSR $0600
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::JSR_ABS));
  bus.write(initial_pc + 1, 0x00);  // Low byte of target
  bus.write(initial_pc + 2, 0x06);  // High byte of target

  // Execute JSR instruction (takes 6 cycles)
  execute_cycles(6);

  // Calculate expected values
  // According to 6502 specs, JSR pushes the address of the last byte of the JSR instruction
  nes::u16 expected_return_addr = initial_pc + 2;  // Address of the last byte of JSR instruction
  nes::u8 expected_sp = initial_sp - 2;            // Return address (2 bytes)

  // Check that SP was decremented appropriately
  EXPECT_EQ(cpu.get_sp(), expected_sp);

  // According to 6502 behavior, PC high byte is pushed first, then PC low byte
  // So we need to read from SP+1 and SP+2
  nes::u8 pushed_pcl = bus.read(0x0100 + expected_sp + 1);
  nes::u8 pushed_pch = bus.read(0x0100 + expected_sp + 2);
  nes::u16 pushed_addr = (pushed_pch << 8) | pushed_pcl;

  // Verify correct return address was pushed to the stack
  EXPECT_EQ(pushed_addr, expected_return_addr);
}

TEST_F(CPUControlFlowTest, jsr_transfers_control_to_target_address) {
  // Set up initial PC
  cpu.reset();
  // Use an address within the valid RAM range (0x0000-0x1FFF)
  cpu.set_pc(0x0400);
  nes::u16 target_address = 0x0600;

  // Write JSR instruction with target address 0x0600
  bus.write(0x0400, static_cast<nes::u8>(nes::Opcode::JSR_ABS));
  bus.write(0x0401, 0x00);  // Low byte of target
  bus.write(0x0402, 0x06);  // High byte of target

  // Execute JSR instruction
  execute_cycles(6);

  // Verify PC is now set to the target address
  EXPECT_EQ(cpu.get_pc(), target_address);
}

TEST_F(CPUControlFlowTest, jsr_leaves_flags_unchanged) {
  // Set up initial PC and set some flags
  cpu.reset();
  cpu.set_pc(0x0400);

  // Set some flags to test
  cpu.set_flag(nes::Flag::CARRY, true);
  cpu.set_flag(nes::Flag::ZERO, true);
  cpu.set_flag(nes::Flag::NEGATIVE, false);
  cpu.set_flag(nes::Flag::DECIMAL, true);
  cpu.set_flag(nes::Flag::OVERFLOW_, false);
  nes::u8 initial_status = cpu.get_status();

  // Write JSR instruction with target address 0x0600
  bus.write(0x0400, static_cast<nes::u8>(nes::Opcode::JSR_ABS));
  bus.write(0x0401, 0x00);  // Low byte of target
  bus.write(0x0402, 0x06);  // High byte of target

  // Execute JSR instruction
  execute_cycles(6);

  // Verify flags weren't affected
  EXPECT_EQ(cpu.get_status(), initial_status);
}

TEST_F(CPUControlFlowTest, jsr_takes_six_cycles) {
  // Set up initial PC
  cpu.reset();
  cpu.set_pc(0x0400);

  // Write JSR instruction with target address 0x0600
  bus.write(0x0400, static_cast<nes::u8>(nes::Opcode::JSR_ABS));
  bus.write(0x0401, 0x00);  // Low byte of target
  bus.write(0x0402, 0x06);  // High byte of target

  // Execute JSR instruction with exactly 6 cycles
  execute_cycles(6);

  // Verify all cycles were consumed
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);

  // Verify the instruction completed (PC should be at target address)
  EXPECT_EQ(cpu.get_pc(), 0x0600);

  // Try with fewer cycles (5) to confirm it doesn't complete
  cpu.reset();
  cpu.set_pc(0x0400);

  // Write the same instruction again
  bus.write(0x0400, static_cast<nes::u8>(nes::Opcode::JSR_ABS));
  bus.write(0x0401, 0x00);
  bus.write(0x0402, 0x06);

  execute_cycles(5);

  // Verify instruction isn't complete yet
  EXPECT_EQ(cpu.get_remaining_cycles(), 1);
}

// RTI Instruction Tests
TEST_F(CPUControlFlowTest, rti_pulls_status_register_from_stack) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Set some initial processor status flags
  cpu.set_flag(nes::Flag::CARRY, false);
  cpu.set_flag(nes::Flag::ZERO, false);
  cpu.set_flag(nes::Flag::NEGATIVE, false);
  cpu.set_flag(nes::Flag::DECIMAL, false);
  cpu.set_flag(nes::Flag::OVERFLOW_, false);
  cpu.set_flag(nes::Flag::INTERRUPT_DISABLE, false);

  // Prepare a different status register value to pull from stack
  nes::u8 status_to_pull = 0;
  status_to_pull |= (nes::u8)nes::Flag::CARRY;
  status_to_pull |= (nes::u8)nes::Flag::ZERO;
  status_to_pull |= (nes::u8)nes::Flag::DECIMAL;
  status_to_pull |= (nes::u8)nes::Flag::UNUSED;  // UNUSED bit should always be 1

  // Push return PC (high byte first, then low byte) and status to stack
  nes::u16 return_addr = 0x0600;

  // Push PCH (high byte)
  cpu.set_sp(initial_sp);
  bus.write(0x0100 + initial_sp, (return_addr >> 8) & 0xFF);
  cpu.set_sp(initial_sp - 1);

  // Push PCL (low byte)
  bus.write(0x0100 + (initial_sp - 1), return_addr & 0xFF);
  cpu.set_sp(initial_sp - 2);

  // Push status register
  bus.write(0x0100 + (initial_sp - 2), status_to_pull);
  cpu.set_sp(initial_sp - 3);

  // Write RTI instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTI_IMP));

  // Execute RTI instruction (takes 6 cycles)
  execute_cycles(6);

  // Verify processor status was restored correctly
  // Note: The BREAK flag should be cleared after RTI
  nes::u8 expected_status = status_to_pull & ~((nes::u8)nes::Flag::BREAK);
  expected_status |= (nes::u8)nes::Flag::UNUSED;  // UNUSED should always be 1

  EXPECT_EQ(cpu.get_status(), expected_status);
}

TEST_F(CPUControlFlowTest, rti_pulls_pc_from_stack) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Push return PC (high byte first, then low byte) and status to stack
  nes::u16 return_addr = 0x0600;
  nes::u8 status_to_pull = cpu.get_status();

  // Push PCH (high byte)
  cpu.set_sp(initial_sp);
  bus.write(0x0100 + initial_sp, (return_addr >> 8) & 0xFF);
  cpu.set_sp(initial_sp - 1);

  // Push PCL (low byte)
  bus.write(0x0100 + (initial_sp - 1), return_addr & 0xFF);
  cpu.set_sp(initial_sp - 2);

  // Push status register
  bus.write(0x0100 + (initial_sp - 2), status_to_pull);
  cpu.set_sp(initial_sp - 3);

  // Write RTI instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTI_IMP));

  // Execute RTI instruction
  execute_cycles(6);

  // Verify PC was restored correctly
  EXPECT_EQ(cpu.get_pc(), return_addr);
}

TEST_F(CPUControlFlowTest, rti_increments_sp_by_three) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Set a lower SP to simulate values already pushed
  nes::u8 lowered_sp = initial_sp - 3;
  cpu.set_sp(lowered_sp);

  // Push return PC (high byte first, then low byte) and status to stack
  nes::u16 return_addr = 0x0600;
  nes::u8 status_to_pull = cpu.get_status();

  // Push status register
  bus.write(0x0100 + lowered_sp + 1, status_to_pull);

  // Push PCL (low byte)
  bus.write(0x0100 + lowered_sp + 2, return_addr & 0xFF);

  // Push PCH (high byte)
  bus.write(0x0100 + lowered_sp + 3, (return_addr >> 8) & 0xFF);

  // Write RTI instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTI_IMP));

  // Execute RTI instruction
  execute_cycles(6);

  // Verify SP was incremented by 3
  EXPECT_EQ(cpu.get_sp(), lowered_sp + 3);
}

TEST_F(CPUControlFlowTest, rti_takes_six_cycles) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Set a lower SP to simulate values already pushed
  nes::u8 lowered_sp = initial_sp - 3;
  cpu.set_sp(lowered_sp);

  // Push return PC and status to stack
  nes::u16 return_addr = 0x0600;
  nes::u8 status_to_pull = cpu.get_status();

  // Push status register
  bus.write(0x0100 + lowered_sp + 1, status_to_pull);

  // Push PCL (low byte)
  bus.write(0x0100 + lowered_sp + 2, return_addr & 0xFF);

  // Push PCH (high byte)
  bus.write(0x0100 + lowered_sp + 3, (return_addr >> 8) & 0xFF);

  // Write RTI instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTI_IMP));

  // Execute RTI instruction with exactly 6 cycles
  execute_cycles(6);

  // Verify all cycles were consumed
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);

  // Verify the instruction completed (PC should be at return address)
  EXPECT_EQ(cpu.get_pc(), return_addr);

  // Try with fewer cycles (5) to confirm it doesn't complete
  cpu.reset();
  cpu.set_pc(initial_pc);
  cpu.set_sp(lowered_sp);

  // Setup the same stack state again
  bus.write(0x0100 + lowered_sp + 1, status_to_pull);
  bus.write(0x0100 + lowered_sp + 2, return_addr & 0xFF);
  bus.write(0x0100 + lowered_sp + 3, (return_addr >> 8) & 0xFF);

  // Write RTI instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTI_IMP));

  execute_cycles(5);

  // Verify instruction isn't complete yet
  EXPECT_EQ(cpu.get_remaining_cycles(), 1);
}

TEST_F(CPUControlFlowTest, rti_after_brk) {
  // This test simulates a BRK instruction followed by RTI
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);

  // Set some initial processor status flags to test
  cpu.set_flag(nes::Flag::CARRY, true);
  cpu.set_flag(nes::Flag::ZERO, false);
  cpu.set_flag(nes::Flag::INTERRUPT_DISABLE, false);

  // Set up interrupt vector
  nes::u16 interrupt_handler = 0x0500;
  bus.write(0xFFFE, interrupt_handler & 0xFF);         // Low byte
  bus.write(0xFFFF, (interrupt_handler >> 8) & 0xFF);  // High byte

  // Write BRK instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::BRK_IMP));

  // Execute BRK instruction (takes 7 cycles)
  execute_cycles(7);

  EXPECT_EQ(cpu.get_pc(), interrupt_handler);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE));

  // The status that was pushed to the stack had B flag set
  // Read the status that was pushed to the stack
  nes::u8 pushed_status = bus.read(0x0100 + cpu.get_sp() + 1);

  EXPECT_TRUE(pushed_status & (nes::u8)nes::Flag::BREAK);

  // Write RTI instruction at interrupt handler
  bus.write(interrupt_handler, static_cast<nes::u8>(nes::Opcode::RTI_IMP));
  execute_cycles(6);

  // Verify PC is back at the instruction after BRK
  EXPECT_EQ(cpu.get_pc(), initial_pc + 2);

  // Verify processor status was restored from the stack
  // Note: The status pushed to stack by BRK had B flag set, but RTI should clear it
  nes::u8 expected_status = pushed_status & ~((nes::u8)nes::Flag::BREAK);

  // Since UNUSED flag is always set, we'll ignore it in the comparison
  EXPECT_EQ(cpu.get_status() & ~((nes::u8)nes::Flag::UNUSED), expected_status & ~((nes::u8)nes::Flag::UNUSED));

  // Also verify specific flags
  EXPECT_EQ(cpu.get_flag(nes::Flag::INTERRUPT_DISABLE), (pushed_status & (nes::u8)nes::Flag::INTERRUPT_DISABLE) != 0);
  EXPECT_EQ(cpu.get_flag(nes::Flag::CARRY), (pushed_status & (nes::u8)nes::Flag::CARRY) != 0);
  EXPECT_EQ(cpu.get_flag(nes::Flag::ZERO), (pushed_status & (nes::u8)nes::Flag::ZERO) != 0);
}

TEST_F(CPUControlFlowTest, rti_handles_break_flag_correctly) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Set a lower SP to simulate values already pushed
  nes::u8 lowered_sp = initial_sp - 3;
  cpu.set_sp(lowered_sp);

  nes::u16 return_addr = 0x0600;
  nes::u8 status_with_break = (nes::u8)nes::Flag::BREAK | (nes::u8)nes::Flag::UNUSED;

  // Push status register with BREAK and UNUSED flags set
  bus.write(0x0100 + lowered_sp + 1, status_with_break);
  // Push PCL (low byte)
  bus.write(0x0100 + lowered_sp + 2, return_addr & 0xFF);
  // Push PCH (high byte)
  bus.write(0x0100 + lowered_sp + 3, (return_addr >> 8) & 0xFF);
  // Write RTI instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTI_IMP));
  // Execute RTI instruction
  execute_cycles(6);

  EXPECT_FALSE(cpu.get_flag(nes::Flag::BREAK));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::UNUSED));
}

// RTS Instruction Tests
TEST_F(CPUControlFlowTest, rts_pulls_pc_from_stack) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Push return address (high byte first, then low byte) to stack
  // Note: RTS expects the address-1 on stack, as it will add 1 after pulling
  nes::u16 pushed_addr = 0x05FF;  // We push 0x05FF so RTS will jump to 0x0600

  // Push PCH (high byte)
  cpu.set_sp(initial_sp);
  bus.write(0x0100 + initial_sp, (pushed_addr >> 8) & 0xFF);
  cpu.set_sp(initial_sp - 1);

  // Push PCL (low byte)
  bus.write(0x0100 + (initial_sp - 1), pushed_addr & 0xFF);
  cpu.set_sp(initial_sp - 2);

  // Write RTS instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTS_IMP));

  // Execute RTS instruction (takes 6 cycles)
  execute_cycles(6);

  // Verify PC was restored correctly and incremented by 1
  EXPECT_EQ(cpu.get_pc(), pushed_addr + 1);
}

TEST_F(CPUControlFlowTest, rts_increments_sp_by_two) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Set a lower SP to simulate values already pushed
  nes::u8 lowered_sp = initial_sp - 2;
  cpu.set_sp(lowered_sp);

  // Push return address to stack
  nes::u16 pushed_addr = 0x05FF;

  // Push PCL (low byte)
  bus.write(0x0100 + lowered_sp + 1, pushed_addr & 0xFF);

  // Push PCH (high byte)
  bus.write(0x0100 + lowered_sp + 2, (pushed_addr >> 8) & 0xFF);

  // Write RTS instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTS_IMP));

  // Execute RTS instruction
  execute_cycles(6);

  // Verify SP was incremented by 2
  EXPECT_EQ(cpu.get_sp(), lowered_sp + 2);
}

TEST_F(CPUControlFlowTest, rts_leaves_flags_unchanged) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Set some flags to test they remain unchanged
  cpu.set_flag(nes::Flag::CARRY, true);
  cpu.set_flag(nes::Flag::ZERO, true);
  cpu.set_flag(nes::Flag::NEGATIVE, false);
  cpu.set_flag(nes::Flag::DECIMAL, true);
  cpu.set_flag(nes::Flag::OVERFLOW_, false);
  nes::u8 initial_status = cpu.get_status();

  // Push return address to stack
  nes::u16 pushed_addr = 0x05FF;

  // Push PCH (high byte)
  cpu.set_sp(initial_sp);
  bus.write(0x0100 + initial_sp, (pushed_addr >> 8) & 0xFF);
  cpu.set_sp(initial_sp - 1);

  // Push PCL (low byte)
  bus.write(0x0100 + (initial_sp - 1), pushed_addr & 0xFF);
  cpu.set_sp(initial_sp - 2);

  // Write RTS instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTS_IMP));

  // Execute RTS instruction
  execute_cycles(6);

  // Verify flags weren't affected
  EXPECT_EQ(cpu.get_status(), initial_status);
}

TEST_F(CPUControlFlowTest, rts_takes_six_cycles) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Set a lower SP to simulate values already pushed
  nes::u8 lowered_sp = initial_sp - 2;
  cpu.set_sp(lowered_sp);

  // Push return address to stack
  nes::u16 pushed_addr = 0x05FF;

  // Push PCL (low byte)
  bus.write(0x0100 + lowered_sp + 1, pushed_addr & 0xFF);

  // Push PCH (high byte)
  bus.write(0x0100 + lowered_sp + 2, (pushed_addr >> 8) & 0xFF);

  // Write RTS instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTS_IMP));

  // Execute RTS instruction with exactly 6 cycles
  execute_cycles(6);

  // Verify all cycles were consumed
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);

  // Verify the instruction completed (PC should be at return address + 1)
  EXPECT_EQ(cpu.get_pc(), pushed_addr + 1);

  // Try with fewer cycles (5) to confirm it doesn't complete
  cpu.reset();
  cpu.set_pc(initial_pc);
  cpu.set_sp(lowered_sp);

  // Setup the same stack state again
  bus.write(0x0100 + lowered_sp + 1, pushed_addr & 0xFF);
  bus.write(0x0100 + lowered_sp + 2, (pushed_addr >> 8) & 0xFF);

  // Write RTS instruction
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTS_IMP));

  execute_cycles(5);

  // Verify instruction isn't complete yet
  EXPECT_EQ(cpu.get_remaining_cycles(), 1);
}

TEST_F(CPUControlFlowTest, jsr_followed_by_rts) {
  // This test simulates a JSR instruction followed by RTS
  // to test the complete subroutine mechanism

  // Set up initial PC
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Target address for JSR
  nes::u16 subroutine_addr = 0x0600;

  // Write JSR instruction at initial PC
  bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::JSR_ABS));
  bus.write(initial_pc + 1, subroutine_addr & 0xFF);         // Low byte
  bus.write(initial_pc + 2, (subroutine_addr >> 8) & 0xFF);  // High byte

  // Write RTS instruction at subroutine address
  bus.write(subroutine_addr, static_cast<nes::u8>(nes::Opcode::RTS_IMP));

  // Execute JSR instruction (takes 6 cycles)
  execute_cycles(6);

  // Verify PC is at subroutine address
  EXPECT_EQ(cpu.get_pc(), subroutine_addr);

  // Verify SP was decremented by 2
  EXPECT_EQ(cpu.get_sp(), initial_sp - 2);

  // Now execute RTS instruction (takes 6 cycles)
  execute_cycles(6);

  // Verify PC is back at the instruction after JSR
  EXPECT_EQ(cpu.get_pc(), initial_pc + 3);

  // Verify SP is back at original value
  EXPECT_EQ(cpu.get_sp(), initial_sp);
}

TEST_F(CPUControlFlowTest, rts_adds_one_to_pulled_address) {
  // Set up initial PC and stack pointer
  cpu.reset();
  nes::u16 initial_pc = 0x0400;
  cpu.set_pc(initial_pc);
  nes::u8 initial_sp = cpu.get_sp();

  // Test several different addresses to verify +1 behavior
  std::vector<nes::u16> test_addresses = {
    0x0000,  // Edge case: lowest address
    0x05FF,  // Normal case
    0x0FFF,  // Page boundary
    0xFFFF   // Edge case: highest address
  };

  for (const auto& addr : test_addresses) {
    // Reset for each test
    cpu.reset();
    cpu.set_pc(initial_pc);
    nes::u8 current_sp = cpu.get_sp();

    // Push the test address to the stack
    // Push PCH (high byte)
    cpu.set_sp(current_sp);
    bus.write(0x0100 + current_sp, (addr >> 8) & 0xFF);
    cpu.set_sp(current_sp - 1);

    // Push PCL (low byte)
    bus.write(0x0100 + (current_sp - 1), addr & 0xFF);
    cpu.set_sp(current_sp - 2);

    // Write RTS instruction
    bus.write(initial_pc, static_cast<nes::u8>(nes::Opcode::RTS_IMP));

    // Execute RTS instruction
    execute_cycles(6);

    // Calculate expected address (with wrap-around for 0xFFFF)
    nes::u16 expected_addr = (addr == 0xFFFF) ? 0x0000 : addr + 1;

    // Verify PC was set to address + 1
    EXPECT_EQ(cpu.get_pc(), expected_addr);
  }
}
