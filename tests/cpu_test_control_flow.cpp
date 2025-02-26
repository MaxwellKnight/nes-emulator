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
