#include "cpu_test_base.h"

class CPUShiftRotateTest : public CPUTestBase {};

// ASL Accumulator (0x0A)
TEST_F(CPUShiftRotateTest, asl_accumulator) {
  // Load value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x45);  // binary: 01000101
  execute_cycles(2);

  // Execute ASL on accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_ACC);
  execute_cycles(2);  // ASL Accumulator takes 2 cycles

  EXPECT_EQ(cpu.get_accumulator(), 0x8A);  // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, asl_accumulator_carry) {
  // Load value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x80);  // binary: 10000000
  execute_cycles(2);

  // Execute ASL on accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_ACC);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// ASL Absolute (0x0E)
TEST_F(CPUShiftRotateTest, asl_absolute) {
  // Set up test value in memory (using lower address range 0x0000-0x1FFF)
  bus.write(0x0042, 0x45);  // binary: 01000101

  // Execute ASL on absolute address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::ASL_ABS);
  bus.write(0xFFFD, 0x42);
  bus.write(0xFFFE, 0x00);  // Address 0x0042
  execute_cycles(6);        // ASL Absolute takes 6 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0042), 0x8A);  // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, asl_absolute_carry_and_zero) {
  // Set up test value in memory (using lower address range)
  bus.write(0x0042, 0x80);  // binary: 10000000

  // Execute ASL on absolute address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::ASL_ABS);
  bus.write(0xFFFD, 0x42);
  bus.write(0xFFFE, 0x00);  // Address 0x0042
  execute_cycles(6);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0042), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// ASL Absolute X-Indexed (0x1E)
TEST_F(CPUShiftRotateTest, asl_absolute_x) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in memory (using lower address range)
  bus.write(0x0052, 0x45);  // binary: 01000101 (at address 0x0042 + 0x10)

  // Execute ASL on absolute X-indexed address
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_ABX);
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x00);  // Base address 0x0042, X = 0x10, final = 0x0052
  execute_cycles(7);        // ASL Absolute X takes 7 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0052), 0x8A);  // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, asl_absolute_x_carry_and_zero) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in memory (using lower address range)
  bus.write(0x0052, 0x80);  // binary: 10000000 (at address 0x0042 + 0x10)

  // Execute ASL on absolute X-indexed address
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_ABX);
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x00);  // Base address 0x0042, X = 0x10, final = 0x0052
  execute_cycles(7);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0052), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// ASL Zero Page (0x06)
TEST_F(CPUShiftRotateTest, asl_zero_page) {
  // Set up test value in zero page
  bus.write(0x42, 0x45);  // binary: 01000101

  // Execute ASL on zero page address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::ASL_ZPG);
  bus.write(0xFFFD, 0x42);  // Zero page address 0x42
  execute_cycles(5);        // ASL Zero Page takes 5 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x42), 0x8A);  // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, asl_zero_page_carry_and_zero) {
  // Set up test value in zero page
  bus.write(0x42, 0x80);  // binary: 10000000

  // Execute ASL on zero page address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::ASL_ZPG);
  bus.write(0xFFFD, 0x42);  // Zero page address 0x42
  execute_cycles(5);

  EXPECT_EQ(bus.read(0x42), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// ASL Zero Page X-Indexed (0x16)
TEST_F(CPUShiftRotateTest, asl_zero_page_x) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x52, 0x45);  // binary: 01000101 (at zero page address 0x42 + 0x10)

  // Execute ASL on zero page X-indexed address
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_ZPX);
  bus.write(0xFFFF,
            0x42);    // Base zero page address 0x42, X = 0x10, final = 0x52
  execute_cycles(6);  // ASL Zero Page X takes 6 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x52), 0x8A);  // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, asl_zero_page_x_wrapping) {
  // Set X register to cause wrap-around
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0xFF);  // X = 0xFF
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x41, 0x45);  // binary: 01000101 (at wrapped address (0x42 + 0xFF) &
                          // 0xFF = 0x41)

  // Execute ASL on zero page X-indexed address with wrap
  bus.write(0xFFFE, (nes::u8)nes::Opcode::ASL_ZPX);
  bus.write(0xFFFF, 0x42);
  execute_cycles(6);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x41), 0x8A);  // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

// LSR Accumulator (0x4A)
TEST_F(CPUShiftRotateTest, lsr_accumulator) {
  // Load value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x8A);  // binary: 10001010
  execute_cycles(2);

  // Execute LSR on accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LSR_ACC);
  execute_cycles(2);  // LSR Accumulator takes 2 cycles

  EXPECT_EQ(cpu.get_accumulator(), 0x45);  // binary: 01000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, lsr_accumulator_carry) {
  // Load value into accumulator
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x01);  // binary: 00000001
  execute_cycles(2);

  // Execute LSR on accumulator
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LSR_ACC);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// LSR Absolute (0x4E)
TEST_F(CPUShiftRotateTest, lsr_absolute) {
  // Set up test value in memory (using lower address range 0x0000-0x1FFF)
  bus.write(0x0042, 0x8A);  // binary: 10001010

  // Execute LSR on absolute address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LSR_ABS);
  bus.write(0xFFFD, 0x42);
  bus.write(0xFFFE, 0x00);  // Address 0x0042
  execute_cycles(6);        // LSR Absolute takes 6 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0042), 0x45);  // binary: 01000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, lsr_absolute_carry_and_zero) {
  // Set up test value in memory (using lower address range)
  bus.write(0x0042, 0x01);  // binary: 00000001

  // Execute LSR on absolute address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LSR_ABS);
  bus.write(0xFFFD, 0x42);
  bus.write(0xFFFE, 0x00);  // Address 0x0042
  execute_cycles(6);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0042), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// LSR Absolute X-Indexed (0x5E)
TEST_F(CPUShiftRotateTest, lsr_absolute_x) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in memory (using lower address range)
  bus.write(0x0052, 0x8A);  // binary: 10001010 (at address 0x0042 + 0x10)

  // Execute LSR on absolute X-indexed address
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LSR_ABX);
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x00);  // Base address 0x0042, X = 0x10, final = 0x0052
  execute_cycles(7);        // LSR Absolute X takes 7 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0052), 0x45);  // binary: 01000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, lsr_absolute_x_carry_and_zero) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in memory (using lower address range)
  bus.write(0x0052, 0x01);  // binary: 00000001 (at address 0x0042 + 0x10)

  // Execute LSR on absolute X-indexed address
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LSR_ABX);
  bus.write(0xFFFF, 0x42);
  bus.write(0x0000, 0x00);  // Base address 0x0042, X = 0x10, final = 0x0052
  execute_cycles(7);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0052), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// LSR Zero Page (0x46)
TEST_F(CPUShiftRotateTest, lsr_zero_page) {
  // Set up test value in zero page
  bus.write(0x42, 0x8A);  // binary: 10001010

  // Execute LSR on zero page address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LSR_ZPG);
  bus.write(0xFFFD, 0x42);  // Zero page address 0x42
  execute_cycles(5);        // LSR Zero Page takes 5 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x42), 0x45);  // binary: 01000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, lsr_zero_page_carry_and_zero) {
  // Set up test value in zero page
  bus.write(0x42, 0x01);  // binary: 00000001

  // Execute LSR on zero page address
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LSR_ZPG);
  bus.write(0xFFFD, 0x42);  // Zero page address 0x42
  execute_cycles(5);

  EXPECT_EQ(bus.read(0x42), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// LSR Zero Page X-Indexed (0x56)
TEST_F(CPUShiftRotateTest, lsr_zero_page_x) {
  // Set X register
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x52, 0x8A);  // binary: 10001010 (at zero page address 0x42 + 0x10)

  // Execute LSR on zero page X-indexed address
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LSR_ZPX);
  bus.write(0xFFFF,
            0x42);    // Base zero page address 0x42, X = 0x10, final = 0x52
  execute_cycles(6);  // LSR Zero Page X takes 6 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x52), 0x45);  // binary: 01000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, lsr_zero_page_x_wrapping) {
  // Set X register to cause wrap-around
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFD, 0xFF);  // X = 0xFF
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x41, 0x8A);  // binary: 10001010 (at wrapped address (0x42 + 0xFF) &
                          // 0xFF = 0x41)

  // Execute LSR on zero page X-indexed address with wrap
  bus.write(0xFFFE, (nes::u8)nes::Opcode::LSR_ZPX);
  bus.write(0xFFFF, 0x42);
  execute_cycles(6);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x41), 0x45);  // binary: 01000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

// ROL Accumulator (0x2A)
TEST_F(CPUShiftRotateTest, rol_accumulator_carry_clear) {
  // Clear carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Load value into accumulator
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x45);  // binary: 01000101
  execute_cycles(2);

  // Execute ROL on accumulator
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);  // ROL Accumulator takes 2 cycles

  EXPECT_EQ(cpu.get_accumulator(), 0x8A);  // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, rol_accumulator_carry_set) {
  // Set carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Load value into accumulator
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x45);  // binary: 01000101
  execute_cycles(2);

  // Execute ROL on accumulator
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x8B);  // binary: 10001011
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, rol_accumulator_carry_out) {
  // Clear carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Load value into accumulator
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x80);  // binary: 10000000
  execute_cycles(2);

  // Execute ROL on accumulator
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, rol_accumulator_carry_in_and_out) {
  // Set carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Load value into accumulator
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x80);  // binary: 10000000
  execute_cycles(2);

  // Execute ROL on accumulator
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x01);  // binary: 00000001
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

// ROL Absolute (0x2E)
TEST_F(CPUShiftRotateTest, rol_absolute_carry_clear) {
  // Clear carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set up test value in memory
  bus.write(0x0042, 0x45);  // binary: 01000101

  // Execute ROL on absolute address
  bus.write(0xFFFD, (nes::u8)nes::Opcode::ROL_ABS);
  bus.write(0xFFFE, 0x42);
  bus.write(0xFFFF, 0x00);  // Address 0x0042
  execute_cycles(6);        // ROL Absolute takes 6 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0042), 0x8A);  // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, rol_absolute_carry_set) {
  // Set carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Set up test value in memory
  bus.write(0x0042, 0x45);  // binary: 01000101

  // Execute ROL on absolute address
  bus.write(0xFFFD, (nes::u8)nes::Opcode::ROL_ABS);
  bus.write(0xFFFE, 0x42);
  bus.write(0xFFFF, 0x00);  // Address 0x0042
  execute_cycles(6);

  EXPECT_EQ(bus.read(0x0042), 0x8B);  // binary: 10001011
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

// ROL Absolute X-Indexed (0x3E)
TEST_F(CPUShiftRotateTest, rol_absolute_x) {
  // Clear carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set X register
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFE, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in memory
  bus.write(0x0052, 0x45);  // binary: 01000101 (at address 0x0042 + 0x10)

  // Execute ROL on absolute X-indexed address
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROL_ABX);
  bus.write(0x0000, 0x42);
  bus.write(0x0001, 0x00);  // Base address 0x0042, X = 0x10, final = 0x0052
  execute_cycles(7);        // ROL Absolute X takes 7 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0052), 0x8A);  // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, rol_absolute_x_carry_in_and_out) {
  // Set carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Set X register
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFE, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in memory
  bus.write(0x0052, 0x80);  // binary: 10000000 (at address 0x0042 + 0x10)

  // Execute ROL on absolute X-indexed address
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROL_ABX);
  bus.write(0x0000, 0x42);
  bus.write(0x0001, 0x00);  // Base address 0x0042, X = 0x10, final = 0x0052
  execute_cycles(7);

  EXPECT_EQ(bus.read(0x0052), 0x01);  // binary: 00000001
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

// ROL Zero Page (0x26)
TEST_F(CPUShiftRotateTest, rol_zero_page) {
  // Clear carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x42, 0x45);  // binary: 01000101

  // Execute ROL on zero page address
  bus.write(0xFFFD, (nes::u8)nes::Opcode::ROL_ZPG);
  bus.write(0xFFFE, 0x42);  // Zero page address 0x42
  execute_cycles(5);        // ROL Zero Page takes 5 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x42), 0x8A);  // binary: 10001010
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, rol_zero_page_to_zero) {
  // Clear carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x42, 0x00);  // binary: 00000000

  // Execute ROL on zero page address
  bus.write(0xFFFD, (nes::u8)nes::Opcode::ROL_ZPG);
  bus.write(0xFFFE, 0x42);  // Zero page address 0x42
  execute_cycles(5);

  EXPECT_EQ(bus.read(0x42), 0x00);  // binary: 00000000
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// ROL Zero Page X-Indexed (0x36)
TEST_F(CPUShiftRotateTest, rol_zero_page_x) {
  // Set carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Set X register
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFE, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x52, 0x45);  // binary: 01000101 (at zero page address 0x42 + 0x10)

  // Execute ROL on zero page X-indexed address
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROL_ZPX);
  bus.write(0x0000, 0x42);  // Base zero page address 0x42, X = 0x10, final = 0x52
  execute_cycles(6);        // ROL Zero Page X takes 6 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x52), 0x8B);  // binary: 10001011
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, rol_zero_page_x_wrapping) {
  // Clear carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set X register to cause wrap-around
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFE, 0xFF);  // X = 0xFF
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x41, 0x80);  // binary: 10000000 (at wrapped address (0x42 + 0xFF) & 0xFF = 0x41)

  // Execute ROL on zero page X-indexed address with wrap
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROL_ZPX);
  bus.write(0x0000, 0x42);
  execute_cycles(6);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x41), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, rol_multiple_operations) {
  // Initial setup: clear carry and load 0x01 into A
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x01);  // Load A with 0000 0001
  execute_cycles(2);

  // First ROL: 0000 0001 -> 0000 0010, carry = 0
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x02);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Second ROL: 0000 0010 -> 0000 0100, carry = 0
  bus.write(0x0000, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x04);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Third ROL: 0000 0100 -> 0000 1000, carry = 0
  bus.write(0x0001, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x08);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Fourth ROL: 0000 1000 -> 0001 0000, carry = 0
  bus.write(0x0002, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x10);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Keep rotating until it wraps
  // Fifth ROL: 0001 0000 -> 0010 0000, carry = 0
  bus.write(0x0003, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x20);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Sixth ROL: 0010 0000 -> 0100 0000, carry = 0
  bus.write(0x0004, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x40);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Seventh ROL: 0100 0000 -> 1000 0000, carry = 0, negative = 1
  bus.write(0x0005, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Eighth ROL: 1000 0000 -> 0000 0000, carry = 1, zero = 1
  bus.write(0x0006, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));

  // Ninth ROL: carry = 1 -> 0000 0001, carry = 0
  bus.write(0x0007, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x01);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

// ROR Accumulator (0x6A)
TEST_F(CPUShiftRotateTest, ror_accumulator_carry_clear) {
  // Clear carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Load value into accumulator
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x8A);  // binary: 10001010
  execute_cycles(2);

  // Execute ROR on accumulator
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);  // ROR Accumulator takes 2 cycles

  EXPECT_EQ(cpu.get_accumulator(), 0x45);  // binary: 01000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, ror_accumulator_carry_set) {
  // Set carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Load value into accumulator
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x8A);  // binary: 10001010
  execute_cycles(2);

  // Execute ROR on accumulator
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0xC5);  // binary: 11000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, ror_accumulator_carry_out) {
  // Clear carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Load value into accumulator
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x01);  // binary: 00000001
  execute_cycles(2);

  // Execute ROR on accumulator
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, ror_accumulator_carry_in_and_out) {
  // Set carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Load value into accumulator
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x01);  // binary: 00000001
  execute_cycles(2);

  // Execute ROR on accumulator
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);

  EXPECT_EQ(cpu.get_accumulator(), 0x80);  // binary: 10000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

// ROR Absolute (0x6E)
TEST_F(CPUShiftRotateTest, ror_absolute_carry_clear) {
  // Clear carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set up test value in memory
  bus.write(0x0042, 0x8A);  // binary: 10001010

  // Execute ROR on absolute address
  bus.write(0xFFFD, (nes::u8)nes::Opcode::ROR_ABS);
  bus.write(0xFFFE, 0x42);
  bus.write(0xFFFF, 0x00);  // Address 0x0042
  execute_cycles(6);        // ROR Absolute takes 6 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0042), 0x45);  // binary: 01000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, ror_absolute_carry_set) {
  // Set carry flag initially
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Set up test value in memory
  bus.write(0x0042, 0x8A);  // binary: 10001010

  // Execute ROR on absolute address
  bus.write(0xFFFD, (nes::u8)nes::Opcode::ROR_ABS);
  bus.write(0xFFFE, 0x42);
  bus.write(0xFFFF, 0x00);  // Address 0x0042
  execute_cycles(6);

  EXPECT_EQ(bus.read(0x0042), 0xC5);  // binary: 11000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

// ROR Absolute X-Indexed (0x7E)
TEST_F(CPUShiftRotateTest, ror_absolute_x) {
  // Clear carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set X register
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFE, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in memory
  bus.write(0x0052, 0x8A);  // binary: 10001010 (at address 0x0042 + 0x10)

  // Execute ROR on absolute X-indexed address
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROR_ABX);
  bus.write(0x0000, 0x42);
  bus.write(0x0001, 0x00);  // Base address 0x0042, X = 0x10, final = 0x0052
  execute_cycles(7);        // ROR Absolute X takes 7 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x0052), 0x45);  // binary: 01000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, ror_absolute_x_carry_in_and_out) {
  // Set carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Set X register
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFE, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in memory
  bus.write(0x0052, 0x01);  // binary: 00000001 (at address 0x0042 + 0x10)

  // Execute ROR on absolute X-indexed address
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROR_ABX);
  bus.write(0x0000, 0x42);
  bus.write(0x0001, 0x00);  // Base address 0x0042, X = 0x10, final = 0x0052
  execute_cycles(7);

  EXPECT_EQ(bus.read(0x0052), 0x80);  // binary: 10000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

// ROR Zero Page (0x66)
TEST_F(CPUShiftRotateTest, ror_zero_page) {
  // Clear carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x42, 0x8A);  // binary: 10001010

  // Execute ROR on zero page address
  bus.write(0xFFFD, (nes::u8)nes::Opcode::ROR_ZPG);
  bus.write(0xFFFE, 0x42);  // Zero page address 0x42
  execute_cycles(5);        // ROR Zero Page takes 5 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x42), 0x45);  // binary: 01000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, ror_zero_page_to_zero) {
  // Clear carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x42, 0x00);  // binary: 00000000

  // Execute ROR on zero page address
  bus.write(0xFFFD, (nes::u8)nes::Opcode::ROR_ZPG);
  bus.write(0xFFFE, 0x42);  // Zero page address 0x42
  execute_cycles(5);

  EXPECT_EQ(bus.read(0x42), 0x00);  // binary: 00000000
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

// ROR Zero Page X-Indexed (0x76)
TEST_F(CPUShiftRotateTest, ror_zero_page_x) {
  // Set carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::SEC_IMP);
  execute_cycles(2);

  // Set X register
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFE, 0x10);  // X = 0x10
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x52, 0x8A);  // binary: 10001010 (at zero page address 0x42 + 0x10)

  // Execute ROR on zero page X-indexed address
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROR_ZPX);
  bus.write(0x0000, 0x42);  // Base zero page address 0x42, X = 0x10, final = 0x52
  execute_cycles(6);        // ROR Zero Page X takes 6 cycles

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x52), 0xC5);  // binary: 11000101
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, ror_zero_page_x_wrapping) {
  // Clear carry flag
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // Set X register to cause wrap-around
  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDX_IMM);
  bus.write(0xFFFE, 0xFF);  // X = 0xFF
  execute_cycles(2);

  // Set up test value in zero page
  bus.write(0x41, 0x01);  // binary: 00000001 (at wrapped address (0x42 + 0xFF) & 0xFF = 0x41)

  // Execute ROR on zero page X-indexed address with wrap
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROR_ZPX);
  bus.write(0x0000, 0x42);
  execute_cycles(6);

  EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  EXPECT_EQ(bus.read(0x41), 0x00);  // binary: 00000000
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, ror_multiple_operations) {
  // Initial setup: clear carry and load 0x80 into A
  bus.write(0xFFFC, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  bus.write(0xFFFD, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFE, 0x80);  // Load A with 1000 0000
  execute_cycles(2);

  // First ROR: 1000 0000 -> 0100 0000, carry = 0
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x40);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Second ROR: 0100 0000 -> 0010 0000, carry = 0
  bus.write(0x0000, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x20);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Third ROR: 0010 0000 -> 0001 0000, carry = 0
  bus.write(0x0001, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x10);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Fourth ROR: 0001 0000 -> 0000 1000, carry = 0
  bus.write(0x0002, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x08);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Keep rotating until it's 1
  // Fifth ROR: 0000 1000 -> 0000 0100, carry = 0
  bus.write(0x0003, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x04);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Sixth ROR: 0000 0100 -> 0000 0010, carry = 0
  bus.write(0x0004, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x02);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));

  // Seventh ROR: 0000 0010 -> 0000 0001, carry = 0
  bus.write(0x0005, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x01);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));

  // Eighth ROR: 0000 0001 -> 0000 0000, carry = 1, zero = 1
  bus.write(0x0006, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_TRUE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::ZERO));

  // Ninth ROR: carry = 1 -> 1000 0000, negative = 1
  bus.write(0x0007, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x80);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::ZERO));
}

TEST_F(CPUShiftRotateTest, ror_rol_combination) {
  // Start with 0x55 (01010101)
  bus.write(0xFFFC, (nes::u8)nes::Opcode::LDA_IMM);
  bus.write(0xFFFD, 0x55);
  execute_cycles(2);

  // Clear carry flag
  bus.write(0xFFFE, (nes::u8)nes::Opcode::CLC_IMP);
  execute_cycles(2);

  // ROL should give 0xAA (10101010), carry = 0
  bus.write(0xFFFF, (nes::u8)nes::Opcode::ROL_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0xAA);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_TRUE(cpu.get_flag(nes::Flag::NEGATIVE));

  // ROR should give back 0x55 (01010101), carry = 0
  bus.write(0x0000, (nes::u8)nes::Opcode::ROR_ACC);
  execute_cycles(2);
  EXPECT_EQ(cpu.get_accumulator(), 0x55);
  EXPECT_FALSE(cpu.get_flag(nes::Flag::CARRY));
  EXPECT_FALSE(cpu.get_flag(nes::Flag::NEGATIVE));
}
