#include "cpu_test_base.h"

using nes::Flag;
using nes::Opcode;
using nes::u8;
using nes::u16;

// Unofficial / "illegal" 6502 opcodes. These run programs from RAM ($0200) so
// there is room for multi-instruction setups, and verify the documented stable
// behaviour that blargg's instr_test exercises.
class CPUIllegalTest : public CPUTestBase {
 protected:
  // Write a program at $0200 and point the CPU at it.
  void load(std::initializer_list<u8> prog) {
    u16 addr = 0x0200;
    for (u8 b : prog) bus.cpu_write(addr++, b);
    cpu.set_pc(0x0200);
  }
};

// LAX (zero page, $A7): load the same value into A and X.
TEST_F(CPUIllegalTest, lax_loads_a_and_x) {
  bus.cpu_write(0x0010, 0x42);
  load({0xA7, 0x10});
  execute_cycles(3);
  EXPECT_EQ(cpu.get_accumulator(), 0x42);
  EXPECT_EQ(cpu.get_x(), 0x42);
  EXPECT_FALSE(cpu.get_flag(Flag::ZERO));
}

// SAX (zero page, $87): store A & X, leaving flags untouched.
TEST_F(CPUIllegalTest, sax_stores_a_and_x) {
  load({(u8)Opcode::LDA_IMM, 0xF0, (u8)Opcode::LDX_IMM, 0x3C, 0x87, 0x10});
  execute_cycles(2 + 2 + 3);
  EXPECT_EQ(bus.cpu_read(0x0010), 0xF0 & 0x3C);
}

// SLO (zero page, $07): ASL memory, then ORA the result into A.
TEST_F(CPUIllegalTest, slo_shifts_then_ora) {
  bus.cpu_write(0x0010, 0x01);
  load({(u8)Opcode::LDA_IMM, 0x10, 0x07, 0x10});
  execute_cycles(2 + 5);
  EXPECT_EQ(bus.cpu_read(0x0010), 0x02);     // 0x01 << 1
  EXPECT_EQ(cpu.get_accumulator(), 0x12);    // 0x10 | 0x02
  EXPECT_FALSE(cpu.get_flag(Flag::CARRY));   // no bit 7 shifted out
}

// DCP (zero page, $C7): DEC memory, then CMP against A.
TEST_F(CPUIllegalTest, dcp_decrements_then_compares) {
  bus.cpu_write(0x0010, 0x03);
  load({(u8)Opcode::LDA_IMM, 0x05, 0xC7, 0x10});
  execute_cycles(2 + 5);
  EXPECT_EQ(bus.cpu_read(0x0010), 0x02);
  EXPECT_TRUE(cpu.get_flag(Flag::CARRY));   // A(5) >= mem(2)
  EXPECT_FALSE(cpu.get_flag(Flag::ZERO));
}

// ISC (zero page, $E7): INC memory, then SBC the result from A.
TEST_F(CPUIllegalTest, isc_increments_then_sbc) {
  bus.cpu_write(0x0010, 0x02);
  // SEC so SBC has no borrow; A=5; ISC -> mem=3; A = 5 - 3 = 2.
  load({(u8)Opcode::SEC_IMP, (u8)Opcode::LDA_IMM, 0x05, 0xE7, 0x10});
  execute_cycles(2 + 2 + 5);
  EXPECT_EQ(bus.cpu_read(0x0010), 0x03);
  EXPECT_EQ(cpu.get_accumulator(), 0x02);
  EXPECT_TRUE(cpu.get_flag(Flag::CARRY));   // no borrow
}

// RRA (zero page, $67): ROR memory, then ADC the result into A.
TEST_F(CPUIllegalTest, rra_rotates_then_adc) {
  bus.cpu_write(0x0010, 0x02);
  // CLC; A=0x10; RRA -> mem = 0x02>>1 = 0x01; A = 0x10 + 0x01 = 0x11.
  load({(u8)Opcode::CLC_IMP, (u8)Opcode::LDA_IMM, 0x10, 0x67, 0x10});
  execute_cycles(2 + 2 + 5);
  EXPECT_EQ(bus.cpu_read(0x0010), 0x01);
  EXPECT_EQ(cpu.get_accumulator(), 0x11);
}

// A multi-byte NOP ($04, zero page) consumes its operand and changes nothing,
// leaving the following instruction correctly aligned.
TEST_F(CPUIllegalTest, unofficial_nop_is_inert_and_aligned) {
  load({0x04, 0x10, (u8)Opcode::LDA_IMM, 0x7E});  // *NOP $10 ; LDA #$7E
  execute_cycles(3 + 2);
  EXPECT_EQ(cpu.get_accumulator(), 0x7E) << "NOP must not desync the PC";
}

// Every opcode 0x00-0xFF must be registered (non-zero cycle count) so the core
// can never abort on an undefined opcode mid-game.
TEST_F(CPUIllegalTest, all_256_opcodes_registered) {
  for (int i = 0; i < 256; i++) {
    auto ins = cpu.get_instruction(static_cast<Opcode>(i));
    EXPECT_NE(ins.cycles, 0) << "opcode 0x" << std::hex << i << " is unregistered";
  }
}
