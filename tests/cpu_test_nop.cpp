#include "cpu_test_base.h"

class CPU_NOP_Test : public CPUTestBase {
 protected:
  void execute_nop_instruction() {
    // Reset CPU
    cpu.reset();
    cpu.set_pc(0x0400);
    bus.write(0x0400, static_cast<nes::u8>(nes::Opcode::NOP_IMP));
  }
};

// Test that NOP advances the PC by 1 byte
TEST_F(CPU_NOP_Test, nop_advances_pc) {
  execute_nop_instruction();

  // Save initial PC
  nes::u16 initial_pc = cpu.get_pc();

  // Execute NOP instruction
  execute_cycles(2);

  // Verify PC advanced by 1 byte
  EXPECT_EQ(cpu.get_pc(), initial_pc + 1);
}

// Test that NOP takes exactly 2 cycles
TEST_F(CPU_NOP_Test, nop_takes_two_cycles) {
  execute_nop_instruction();

  // Execute with exactly 2 cycles
  execute_cycles(2);

  // Verify all cycles were consumed
  EXPECT_EQ(cpu.get_remaining_cycles(), 0);

  // Reset and try with fewer cycles to verify it's not complete
  execute_nop_instruction();
  execute_cycles(1);

  // Verify instruction isn't complete yet
  EXPECT_EQ(cpu.get_remaining_cycles(), 1);
}

// Test that NOP doesn't affect any CPU flags
TEST_F(CPU_NOP_Test, nop_leaves_flags_unchanged) {
  execute_nop_instruction();

  // Set some flags to known values before NOP
  cpu.set_flag(nes::Flag::CARRY, true);
  cpu.set_flag(nes::Flag::ZERO, true);
  cpu.set_flag(nes::Flag::NEGATIVE, true);
  cpu.set_flag(nes::Flag::DECIMAL, true);
  cpu.set_flag(nes::Flag::OVERFLOW_, true);
  cpu.set_flag(nes::Flag::INTERRUPT_DISABLE, true);

  // Save the status before NOP
  nes::u8 status_before = cpu.get_status();

  // Execute NOP instruction
  execute_cycles(2);

  // Verify status wasn't changed
  EXPECT_EQ(cpu.get_status(), status_before);
}

// Test that NOP doesn't affect any CPU registers
TEST_F(CPU_NOP_Test, nop_leaves_registers_unchanged) {
  execute_nop_instruction();

  // Set registers to known values
  nes::u8 a_val = 0x42;
  nes::u8 x_val = 0x69;
  nes::u8 y_val = 0xAB;
  nes::u8 sp_val = cpu.get_sp();

  // Load values into registers using LDA/LDX/LDY instructions
  bus.write(0x0400, static_cast<nes::u8>(nes::Opcode::LDA_IMM));
  bus.write(0x0401, a_val);
  execute_cycles(2);

  bus.write(0x0402, static_cast<nes::u8>(nes::Opcode::LDX_IMM));
  bus.write(0x0403, x_val);
  execute_cycles(2);

  bus.write(0x0404, static_cast<nes::u8>(nes::Opcode::LDY_IMM));
  bus.write(0x0405, y_val);
  execute_cycles(2);

  // Write and execute NOP
  bus.write(0x0406, static_cast<nes::u8>(nes::Opcode::NOP_IMP));
  execute_cycles(2);

  // Verify registers weren't changed
  EXPECT_EQ(cpu.get_accumulator(), a_val);
  EXPECT_EQ(cpu.get_x(), x_val);
  EXPECT_EQ(cpu.get_y(), y_val);
  EXPECT_EQ(cpu.get_sp(), sp_val);
}

// Test multiple consecutive NOPs
TEST_F(CPU_NOP_Test, multiple_consecutive_nops) {
  // Reset CPU
  cpu.reset();
  cpu.set_pc(0x0400);

  // Write 3 consecutive NOP instructions
  bus.write(0x0400, static_cast<nes::u8>(nes::Opcode::NOP_IMP));
  bus.write(0x0401, static_cast<nes::u8>(nes::Opcode::NOP_IMP));
  bus.write(0x0402, static_cast<nes::u8>(nes::Opcode::NOP_IMP));

  // Execute all 3 NOPs (6 cycles total)
  execute_cycles(6);

  // Verify PC advanced by 3 bytes (1 for each NOP)
  EXPECT_EQ(cpu.get_pc(), 0x0403);
}

// Test NOP in between other instructions
TEST_F(CPU_NOP_Test, nop_between_other_instructions) {
  // Reset CPU
  cpu.reset();
  cpu.set_pc(0x0400);

  // Write a sequence of instructions with NOP in between
  bus.write(0x0400, static_cast<nes::u8>(nes::Opcode::LDA_IMM));  // 2 cycles
  bus.write(0x0401, 0x42);
  bus.write(0x0402, static_cast<nes::u8>(nes::Opcode::NOP_IMP));  // 2 cycles
  bus.write(0x0403, static_cast<nes::u8>(nes::Opcode::LDX_IMM));  // 2 cycles
  bus.write(0x0404, 0x69);

  // Execute all instructions (6 cycles total)
  execute_cycles(6);

  // Verify PC is at the end and registers have correct values
  EXPECT_EQ(cpu.get_pc(), 0x0405);
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_x(), 0x69);
}

// Test NOP's memory impact (it should have none)
TEST_F(CPU_NOP_Test, nop_memory_impact) {
  // Reset CPU
  cpu.reset();
  cpu.set_pc(0x0400);

  // Write a value to a memory location
  nes::u8 test_value = 0x55;
  nes::u16 test_address = 0x0300;
  bus.write(test_address, test_value);

  // Write NOP instruction
  bus.write(0x0400, static_cast<nes::u8>(nes::Opcode::NOP_IMP));

  // Execute NOP
  execute_cycles(2);

  // Verify memory wasn't changed
  EXPECT_EQ(bus.read(test_address), test_value);
}
