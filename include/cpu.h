#pragma once

#include "bus.h"
#include "types.h"
#include <unordered_map>

namespace nes {
class CPU {
private:
  // CPU Registers
  u8 _A;      // Accumulator
  u8 _X;      // X Register
  u8 _Y;      // Y Register
  u8 _SP;     // Stack Pointer
  u8 _status; // Status Register
  u16 _PC;    // Program Counter
  u8 _cycles; // Remaining cycles for current instruction

  // Reference to the bus for memory access
  Bus &_bus;

  // Instruction table mapping opcodes to handlers
  std::unordered_map<u8, Instruction> _instruction_table;

  // Memory access methods
  u8 read_byte(u16 address);
  void write_byte(const u16 address, const u8 value);

  // Flag operations
  void set_flag(const Flag flag, const bool value);
  void update_zero_and_negative_flags(const u8 value);

  // Addressing modes
  u16 immediate(bool &page_crossed);
  u16 zero_page(bool &page_crossed);
  u16 zero_page_x(bool &page_crossed);
  u16 zero_page_y(bool &page_crossed);
  u16 absolute(bool &page_crossed);
  u16 absolute_x(bool &page_crossed);
  u16 absolute_y(bool &page_crossed);
  u16 indirect_x(bool &page_crossed);
  u16 indirect_y(bool &page_crossed);

  // Operations
  // Load operations
  void op_lda(const u16 addr);
  void op_ldx(const u16 addr);
  void op_ldy(const u16 addr);

  // Store operations
  void op_sta(const u16 addr);
  void op_stx(const u16 addr);
  void op_sty(const u16 addr);

  // Transfer operations
  void op_tax(const u16 addr);
  void op_tay(const u16 addr);
  void op_txa(const u16 addr);
  void op_tya(const u16 addr);
  void op_tsx(const u16 addr);
  void op_txs(const u16 addr);

  // Stack operations
  void op_pha(const u16 addr);
  void op_php(const u16 addr);
  void op_pla(const u16 addr);
  void op_plp(const u16 addr);

  // Shift operations
  void op_asl_acc(const u16 addr);
  void op_asl(const u16 addr);
  void op_lsr_acc(const u16 addr);
  void op_lsr(const u16 addr);

public:
  CPU(Bus &bus_ref);

  // Core methods
  void clock();
  void reset();

  // Getters for testing/debugging
  u8 get_accumulator() const;
  u8 get_x() const;
  u8 get_y() const;
  u16 get_pc() const;
  u8 get_sp() const;
  u8 get_status() const;
  u8 get_remaining_cycles() const;
  bool get_flag(Flag flag) const;

  // Setters for testing
  void set_sp(u8 sp);
};

} // namespace nes
