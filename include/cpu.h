#pragma once
#include "bus.h"
#include "memory.h"
#include <unordered_map>

namespace nes {
class CPU {
public:
  CPU(Bus &bus);
  void reset();
  void clock();
  [[nodiscard]] u8 get_accumulator() const;
  [[nodiscard]] u8 get_x() const;
  [[nodiscard]] u8 get_y() const;
  [[nodiscard]] u16 get_pc() const;
  [[nodiscard]] u8 get_sp() const;
  [[nodiscard]] u8 get_status() const;
  [[nodiscard]] u8 get_remaining_cycles() const;
  [[nodiscard]] bool get_flag(Flag flag) const;
  void set_flag(Flag flag, bool value);
  void set_sp(u8 sp);
  void print_cpu_state() const;

private:
  // Registers
  u8 _A{0};      // Accumulator
  u8 _X{0};      // X Index Register
  u8 _Y{0};      // Y Index Register
  u8 _SP{0};     // Stack Pointer
  u16 _PC{0};    // Program Counter
  u8 _status{0}; // Processor Status Register
  u32 _cycles{0};
  Bus &_bus;

  std::unordered_map<u8, Instruction> _instruction_table;

  // Instruction handlers
  // LDA
  void lda_immediate();
  void lda_zero_page();
  void lda_absolute();
  void lda_absolute_x();
  void lda_absolute_y();
  void lda_zero_page_x();
  void lda_indirect_x();
  void lda_indirect_y();

  // LDX
  void ldx_immediate();
  void ldx_absolute();
  void ldx_absolute_y();
  void ldx_zero_page();
  void ldx_zero_page_y();

  // LDY
  void ldy_immediate();
  void ldy_absolute();
  void ldy_absolute_x();
  void ldy_zero_page();
  void ldy_zero_page_x();

  // STA
  void sta_absolute();
  void sta_absolute_x();
  void sta_absolute_y();
  void sta_zero_page();
  void sta_zero_page_x();
  void sta_indirect_x();
  void sta_indirect_y();

  // STX
  void stx_absolute();
  void stx_zero_page();
  void stx_zero_page_y();

  // STY
  void sty_absolute();
  void sty_zero_page();
  void sty_zero_page_x();

  void tax();
  void tay();
  void txa();
  void tsx();
  void txs();
  void tya();

  // Stack
  void pha();
  void php();
  void pla();
  void plp();

  // Internal methods
  void write_byte(u16 address, u8 value);
  void update_zero_and_negative_flags(u8 value);
  [[nodiscard]] u8 read_byte(const u16 address);
  [[nodiscard]] bool check_page_cross(u16 addr1, u16 addr2);

  // Addressing modes
  [[nodiscard]] u16 addr_zero_page();
  [[nodiscard]] u16 addr_zero_page_y();
  [[nodiscard]] u16 addr_zero_page_x();
  [[nodiscard]] u16 addr_absolute();
  [[nodiscard]] u16 addr_absolute_x();
  [[nodiscard]] u16 addr_absolute_y();
  [[nodiscard]] u16 addr_indirect_x();
  [[nodiscard]] u16 addr_indirect_y(u8 zp_addr);
};

} // namespace nes
