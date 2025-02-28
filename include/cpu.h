#pragma once
#include <array>
#include <cstddef>
#include "bus.h"
#include "types.h"

namespace nes {

class CPU {
 private:
  // CPU Registers
  u8 _A;       // Accumulator
  u8 _X;       // X Register
  u8 _Y;       // Y Register
  u8 _SP;      // Stack Pointer
  u8 _status;  // Status Register
  u16 _PC;     // Program Counter
  u8 _cycles;  // Remaining cycles for current instruction

  // Reference to the bus for memory access
  Bus &_bus;

  // Used for addressing mode to know if a page is crossed to add a cycle
  bool _page_crossed = false;

  // Instruction table mapping opcodes to handlers
  static constexpr size_t INSTRUCTION_TABLE_SIZE = 256;
  std::array<Instruction, INSTRUCTION_TABLE_SIZE> _instruction_table;

  // Flag operations
  void update_zero_and_negative_flags(const u8 value);

  // Addressing modes
  u16 immediate();
  u16 zero_page();
  u16 zero_page_x();
  u16 zero_page_y();
  u16 absolute();
  u16 absolute_x();
  u16 absolute_y();
  u16 indirect_x();
  u16 indirect_y();
  u16 absolute_indirect();
  u16 relative();

  // Operations that require an address
  // Load operations
  void op_lda(u16 addr);
  void op_ldx(u16 addr);
  void op_ldy(u16 addr);
  // Store operations
  void op_sta(u16 addr);
  void op_stx(u16 addr);
  void op_sty(u16 addr);
  // Shift/Rotate operations
  void op_asl(u16 addr);
  void op_lsr(u16 addr);
  void op_rol(u16 addr);
  void op_ror(u16 addr);
  // Arithmetic operations
  void op_adc(u16 addr);
  void op_sbc(u16 addr);
  void op_cmp(u16 addr);
  void op_cpx(u16 addr);
  void op_cpy(u16 addr);
  // Logical operations
  void op_and(u16 addr);
  void op_bit(u16 addr);
  void op_eor(u16 addr);
  void op_ora(u16 addr);
  // Increment/Decrement operations
  void op_inc(u16 addr);
  void op_dec(u16 addr);
  // Branching operations
  void op_bcc(u16 addr);
  void op_bcs(u16 addr);
  void op_beq(u16 addr);
  void op_bmi(u16 addr);
  void op_bne(u16 addr);
  void op_bpl(u16 addr);
  void op_bvc(u16 addr);
  void op_bvs(u16 addr);
  // Control-Flow operations
  void op_jmp(u16 addr);
  void op_jsr(u16 addr);
  void op_jmp_ind(u16 addr);

  // Operations that don't require an address (implied operations)
  // No operation
  void op_nop();
  // Transfer operations
  void op_tax();
  void op_tay();
  void op_txa();
  void op_tya();
  void op_tsx();
  void op_txs();
  // Stack operations
  void op_pha();
  void op_php();
  void op_pla();
  void op_plp();
  // Shift/Rotate operations
  void op_asl_acc();
  void op_lsr_acc();
  void op_rol_acc();
  void op_ror_acc();
  // Increment/Decrement operations
  void op_inx();
  void op_iny();
  void op_dex();
  void op_dey();
  // Flag operations
  void op_clc();
  void op_cld();
  void op_cli();
  void op_clv();
  void op_sec();
  void op_sed();
  void op_sei();
  // Control-Flow operations
  void op_brk();
  void op_rti();
  void op_rts();

 public:
  CPU(Bus &bus_ref);
  ~CPU() = default;

  // Core methods
  void clock();
  void reset();

  // Getters
  u8 get_accumulator() const;
  u8 get_x() const;
  u8 get_y() const;
  u16 get_pc() const;
  u8 get_sp() const;
  u8 get_status() const;
  u8 get_remaining_cycles() const;
  bool get_flag(Flag flag) const;
  Instruction get_instruction(const Opcode opcode) const;

  // Setters
  void set_sp(u8 sp);
  void set_pc(u16 sp);
  void set_flag(const Flag flag, const bool value);
  void set_status(const u8 status);

  // Memory access methods
  u8 read_byte(u16 address);
  void write_byte(const u16 address, const u8 value);
};

}  // namespace nes
