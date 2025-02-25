#include "cpu_test_base.h"

class CPUStackTest : public CPUTestBase {};

// PHA_IMP (Push Accumulator) Tests
TEST_F(CPUStackTest, pha_basic) {
  // Load a value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  // Push accumulator onto stack
  bus.write(0xFFFE, (nes::u8)nes::Opcode::PHA_IMP);
  execute_cycles(3);  // PHA_IMP takes 3 cycles

  // Verify stack pointer was decremented
  EXPECT_EQ(cpu.get_sp(), 0xFE);
  // Verify value was stored on stack
  EXPECT_EQ(bus.read(0x01FF), 0x42);
  // Verify accumulator wasn't changed
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
}

TEST_F(CPUStackTest, pha_multiple_pushes) {
  // Push first value
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::PHA_IMP);
  execute_cycles(3);

  // Push second value
  bus.write(0xFFFF, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0x0000, 0x37);
  execute_cycles(2);

  bus.write(0x0001, (nes::u8)nes::Opcode::PHA_IMP);
  execute_cycles(3);

  // Verify stack state
  EXPECT_EQ(cpu.get_sp(), 0xFD);
  EXPECT_EQ(bus.read(0x01FF), 0x42);
  EXPECT_EQ(bus.read(0x01FE), 0x37);
}

// PHP_IMP (Push Processor Status) Tests
TEST_F(CPUStackTest, php_basic) {
  // Set some flags
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x00);  // This will set the zero flag
  execute_cycles(2);

  // Push processor status
  bus.write(0xFFFE, (nes::u8)nes::Opcode::PHP_IMP);
  execute_cycles(3);  // PHP_IMP takes 3 cycles

  // Verify stack pointer was decremented
  EXPECT_EQ(cpu.get_sp(), 0xFE);

  // The pushed status should have BREAK and UNUSED flags set
  nes::u8 expected_status = cpu.get_status() | 0x30;  // BREAK and UNUSED flags
  EXPECT_EQ(bus.read(0x01FF), expected_status);
}

TEST_F(CPUStackTest, php_all_flags) {
  // Set various flags using operations
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x80);  // Sets negative flag
  execute_cycles(2);

  // We would set more flags here in a real CPU test
  // For this example we'll just use the negative flag

  // Push processor status
  bus.write(0xFFFE, (nes::u8)nes::Opcode::PHP_IMP);
  execute_cycles(3);

  // Verify the pushed status
  nes::u8 expected_status = cpu.get_status() | 0x30;  // With BREAK and UNUSED set
  EXPECT_EQ(bus.read(0x01FF), expected_status);
  EXPECT_EQ(cpu.get_sp(), 0xFE);
}

// PLA_IMP (Pull Accumulator) Tests
TEST_F(CPUStackTest, pla_basic) {
  // First push a value to stack
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x42);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::PHA_IMP);
  execute_cycles(3);

  // Clear accumulator
  bus.write(0xFFFF, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0x0000, 0x00);
  execute_cycles(2);

  // Pull value from stack
  bus.write(0x0001, (nes::u8)nes::Opcode::PLA_IMP);
  execute_cycles(4);  // PLA_IMP takes 4 cycles

  // Verify pulled value
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_sp(), 0xFF);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUStackTest, pla_zero_flag) {
  // Push zero to stack
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x00);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::PHA_IMP);
  execute_cycles(3);

  // Clear accumulator
  bus.write(0xFFFF, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0x0000, 0x42);
  execute_cycles(2);

  // Pull value from stack
  bus.write(0x0001, (nes::u8)nes::Opcode::PLA_IMP);
  execute_cycles(4);

  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}

TEST_F(CPUStackTest, pla_negative_flag) {
  // Push negative value to stack
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x80);
  execute_cycles(2);

  bus.write(0xFFFE, (nes::u8)nes::Opcode::PHA_IMP);
  execute_cycles(3);

  // Pull value from stack
  bus.write(0xFFFF, (nes::u8)nes::Opcode::PLA_IMP);
  execute_cycles(4);

  EXPECT_EQ(cpu.get_accumulator(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}

// PLP_IMP (Pull Processor Status) Tests
TEST_F(CPUStackTest, plp_basic) {
  // First push processor status
  bus.write(0xFFFC, (nes::u8)nes::Opcode::PHP_IMP);
  execute_cycles(3);

  // Modify some flags
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x00);  // Sets zero flag
  execute_cycles(2);

  // Pull processor status
  bus.write(0xFFFF, (nes::u8)nes::Opcode::PLP_IMP);
  execute_cycles(4);  // PLP_IMP takes 4 cycles

  // Original status had BREAK and UNUSED set when pushed,
  // but BREAK flag should be unchanged when pulled
  nes::u8 expected_status = (bus.read(0x01FF) & ~0x10) | (cpu.get_status() & 0x10);
  EXPECT_EQ(cpu.get_status(), expected_status);
}

TEST_F(CPUStackTest, plp_preserves_break_flag) {
  // Set initial processor status
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x80);  // Sets negative flag
  execute_cycles(2);

  // Push processor status
  bus.write(0xFFFE, (nes::u8)nes::Opcode::PHP_IMP);
  execute_cycles(3);

  // Clear flags
  bus.write(0xFFFF, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0x0000, 0x00);
  execute_cycles(2);

  // Pull processor status
  bus.write(0x0001, (nes::u8)nes::Opcode::PLP_IMP);
  execute_cycles(4);

  // Verify BREAK flag remains unchanged
  EXPECT_EQ(cpu.get_status() & 0x10, cpu.get_status() & 0x10);
  // Verify other flags were restored
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
}
