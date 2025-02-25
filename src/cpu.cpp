#include "../include/cpu.h"
#include "types.h"
#include <string>

namespace nes {
CPU::CPU(Bus &bus_ref) : _bus(bus_ref) {
  reset();
  _instruction_table = {
      // LDA
      {(u8)Opcode::LDA_IM, {&CPU::op_lda, &CPU::immediate, 2, "LDA"}},
      {(u8)Opcode::LDA_ZP, {&CPU::op_lda, &CPU::zero_page, 3, "LDA"}},
      {(u8)Opcode::LDA_ABS, {&CPU::op_lda, &CPU::absolute, 4, "LDA"}},
      {(u8)Opcode::LDA_XABS, {&CPU::op_lda, &CPU::absolute_x, 4, "LDA", true}},
      {(u8)Opcode::LDA_YABS, {&CPU::op_lda, &CPU::absolute_y, 4, "LDA", true}},
      {(u8)Opcode::LDA_XZP, {&CPU::op_lda, &CPU::zero_page_x, 4, "LDA"}},
      {(u8)Opcode::LDA_XZPI, {&CPU::op_lda, &CPU::indirect_x, 6, "LDA"}},
      {(u8)Opcode::LDA_YZPI, {&CPU::op_lda, &CPU::indirect_y, 5, "LDA", true}},

      // LDX
      {(u8)Opcode::LDX_IM, {&CPU::op_ldx, &CPU::immediate, 2, "LDX"}},
      {(u8)Opcode::LDX_ABS, {&CPU::op_ldx, &CPU::absolute, 4, "LDX"}},
      {(u8)Opcode::LDX_YABS, {&CPU::op_ldx, &CPU::absolute_y, 4, "LDX", true}},
      {(u8)Opcode::LDX_ZP, {&CPU::op_ldx, &CPU::zero_page, 3, "LDX"}},
      {(u8)Opcode::LDX_YZP, {&CPU::op_ldx, &CPU::zero_page_y, 4, "LDX"}},

      // LDY
      {(u8)Opcode::LDY_IM, {&CPU::op_ldy, &CPU::immediate, 2, "LDY"}},
      {(u8)Opcode::LDY_ABS, {&CPU::op_ldy, &CPU::absolute, 4, "LDY"}},
      {(u8)Opcode::LDY_XABS, {&CPU::op_ldy, &CPU::absolute_x, 4, "LDY", true}},
      {(u8)Opcode::LDY_ZP, {&CPU::op_ldy, &CPU::zero_page, 3, "LDY"}},
      {(u8)Opcode::LDY_XZP, {&CPU::op_ldy, &CPU::zero_page_x, 4, "LDY"}},

      // STA
      {(u8)Opcode::STA_ABS, {&CPU::op_sta, &CPU::absolute, 4, "STA"}},
      {(u8)Opcode::STA_XABS, {&CPU::op_sta, &CPU::absolute_x, 5, "STA"}},
      {(u8)Opcode::STA_YABS, {&CPU::op_sta, &CPU::absolute_y, 5, "STA"}},
      {(u8)Opcode::STA_ZP, {&CPU::op_sta, &CPU::zero_page, 3, "STA"}},
      {(u8)Opcode::STA_XZP, {&CPU::op_sta, &CPU::zero_page_x, 4, "STA"}},
      {(u8)Opcode::STA_XZPI, {&CPU::op_sta, &CPU::indirect_x, 6, "STA"}},
      {(u8)Opcode::STA_YZPI, {&CPU::op_sta, &CPU::indirect_y, 6, "STA"}},

      // STX
      {(u8)Opcode::STX_ABS, {&CPU::op_stx, &CPU::absolute, 4, "STX"}},
      {(u8)Opcode::STX_ZP, {&CPU::op_stx, &CPU::zero_page, 3, "STX"}},
      {(u8)Opcode::STX_YZP, {&CPU::op_stx, &CPU::zero_page_y, 4, "STX"}},

      // STY
      {(u8)Opcode::STY_ABS, {&CPU::op_sty, &CPU::absolute, 4, "STY"}},
      {(u8)Opcode::STY_ZP, {&CPU::op_sty, &CPU::zero_page, 3, "STY"}},
      {(u8)Opcode::STY_XZP, {&CPU::op_sty, &CPU::zero_page_x, 4, "STY"}},

      // Transfer operations (implied addressing, no address needed)
      {(u8)Opcode::TAX, {&CPU::op_tax, nullptr, 2, "TAX"}},
      {(u8)Opcode::TAY, {&CPU::op_tay, nullptr, 2, "TAY"}},
      {(u8)Opcode::TSX, {&CPU::op_tsx, nullptr, 2, "TSX"}},
      {(u8)Opcode::TYA, {&CPU::op_tya, nullptr, 2, "TYA"}},
      {(u8)Opcode::TXS, {&CPU::op_txs, nullptr, 2, "TXS"}},
      {(u8)Opcode::TXA, {&CPU::op_txa, nullptr, 2, "TXA"}},

      // Stack operations (implied addressing, no address needed)
      {(u8)Opcode::PHA, {&CPU::op_pha, nullptr, 3, "PHA"}},
      {(u8)Opcode::PLA, {&CPU::op_pla, nullptr, 4, "PLA"}},
      {(u8)Opcode::PLP, {&CPU::op_plp, nullptr, 4, "PLP"}},
      {(u8)Opcode::PHP, {&CPU::op_php, nullptr, 3, "PHP"}},

      // ASL
      {(u8)Opcode::ASL_ACC, {&CPU::op_asl_acc, nullptr, 2, "ASL"}},
      {(u8)Opcode::ASL_ABS, {&CPU::op_asl, &CPU::absolute, 6, "ASL"}},
      {(u8)Opcode::ASL_XABS, {&CPU::op_asl, &CPU::absolute_x, 7, "ASL"}},
      {(u8)Opcode::ASL_ZP, {&CPU::op_asl, &CPU::zero_page, 5, "ASL"}},
      {(u8)Opcode::ASL_XZP, {&CPU::op_asl, &CPU::zero_page_x, 6, "ASL"}},

      // LSR
      {(u8)Opcode::LSR_ACC, {&CPU::op_lsr_acc, nullptr, 2, "LSR"}},
      {(u8)Opcode::LSR_ABS, {&CPU::op_lsr, &CPU::absolute, 6, "LSR"}},
      {(u8)Opcode::LSR_XABS, {&CPU::op_lsr, &CPU::absolute_x, 7, "LSR"}},
      {(u8)Opcode::LSR_ZP, {&CPU::op_lsr, &CPU::zero_page, 5, "LSR"}},
      {(u8)Opcode::LSR_XZP, {&CPU::op_lsr, &CPU::zero_page_x, 6, "LSR"}}};
}

void CPU::clock() {
  if (_cycles == 0) {
    u8 opcode = read_byte(_PC++);
    set_flag(Flag::UNUSED, true);

    auto it = _instruction_table.find(opcode);
    if (it == _instruction_table.end()) {
      throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
    }

    _cycles = it->second.cycles;

    auto addr_mode = it->second.mode;
    auto operation = it->second.operation;

    u16 addr_or_data = 0;
    if (addr_mode != nullptr) {
      bool page_crossed = false;
      addr_or_data = (this->*addr_mode)(page_crossed);

      // Add cycle for page crossing if applicable
      if (page_crossed && it->second.is_extra_cycle) {
        _cycles++;
      }
    }

    (this->*operation)(addr_or_data);
  }
  _cycles--;
}

void CPU::reset() {
  _A = 0;
  _X = 0;
  _Y = 0;
  _SP = 0xFF;
  _status = (u8)Flag::UNUSED | (u8)Flag::BREAK;
  _PC = 0xFFFC;
  _cycles = 0;
}

// Memory operations
u8 CPU::read_byte(const u16 address) { return _bus.read(address); }
void CPU::write_byte(const u16 address, const u8 value) {
  _bus.write(address, value);
}

// Getters
u8 CPU::get_accumulator() const { return _A; }
u8 CPU::get_x() const { return _X; }
u8 CPU::get_y() const { return _Y; }
u16 CPU::get_pc() const { return _PC; }
u8 CPU::get_sp() const { return _SP; }
u8 CPU::get_status() const { return _status; }
u8 CPU::get_remaining_cycles() const { return _cycles; }

// Setters
void CPU::set_sp(u8 sp) { _SP = sp; }

// Flag operations
bool CPU::get_flag(Flag flag) const { return (_status & (u8)(flag)) != 0; }

void CPU::set_flag(const Flag flag, const bool value) {
  if (value) {
    _status |= (u8)(flag);
  } else {
    _status &= ~(u8)(flag);
  }
}

void CPU::update_zero_and_negative_flags(u8 value) {
  set_flag(Flag::ZERO, value == 0);
  set_flag(Flag::NEGATIVE, (value & 0x80) != 0);
}

//////////////////////////////////////////////////////////////////////////
// ADDRESSING MODES
//////////////////////////////////////////////////////////////////////////

u16 CPU::immediate(bool &page_crossed) {
  return _PC++; // Return the PC then increment it
}

u16 CPU::zero_page(bool &page_crossed) {
  return read_byte(_PC++); // Zero page address is just a single byte
}

u16 CPU::zero_page_x(bool &page_crossed) {
  u8 zp_addr = read_byte(_PC++);
  return (u16)((zp_addr + _X) & 0xFF); // Wrap around in zero page
}

u16 CPU::zero_page_y(bool &page_crossed) {
  u8 zp_addr = read_byte(_PC++);
  return (u16)((zp_addr + _Y) & 0xFF); // Wrap around in zero page
}

u16 CPU::absolute(bool &page_crossed) {
  u16 addr_low = read_byte(_PC++);
  u16 addr_high = read_byte(_PC++);
  return (addr_high << 8) | addr_low;
}

u16 CPU::absolute_x(bool &page_crossed) {
  u16 addr_low = read_byte(_PC++);
  u16 addr_high = read_byte(_PC++);
  u16 base_addr = (addr_high << 8) | addr_low;
  u16 final_addr = base_addr + _X;

  page_crossed = ((base_addr & 0xFF00) != (final_addr & 0xFF00));
  return final_addr;
}
u16 CPU::absolute_y(bool &page_crossed) {
  u16 base_addr = absolute(page_crossed);
  u16 final_addr = base_addr + _Y;

  page_crossed = ((base_addr & 0xFF00) != (final_addr & 0xFF00));
  return final_addr;
}

u16 CPU::indirect_x(bool &page_crossed) {
  u8 zp_addr = read_byte(_PC++);
  zp_addr += _X; // Add X to the zero page address (with wrap)

  // Read two bytes from the computed zero page address
  u16 effective_addr_low = read_byte(zp_addr);
  u16 effective_addr_high = read_byte((u16)((zp_addr + 1) & 0xFF));

  return (effective_addr_high << 8) | effective_addr_low;
}

u16 CPU::indirect_y(bool &page_crossed) {
  u8 zp_addr = read_byte(_PC++);

  u16 effective_addr_low = read_byte(zp_addr);
  u16 effective_addr_high = read_byte(((u16)((zp_addr + 1) & 0xFF)));

  u16 base_addr = (effective_addr_high << 8) | effective_addr_low;
  u16 final_addr = base_addr + _Y;

  page_crossed = ((base_addr & 0xFF00) != (final_addr & 0xFF00));

  return final_addr;
}

//////////////////////////////////////////////////////////////////////////
// OPERATIONS
//////////////////////////////////////////////////////////////////////////

// Load operations
void CPU::op_lda(const u16 addr) {
  _A = read_byte(addr);
  update_zero_and_negative_flags(_A);
}

void CPU::op_ldx(const u16 addr) {
  _X = read_byte(addr);
  update_zero_and_negative_flags(_X);
}

void CPU::op_ldy(const u16 addr) {
  _Y = read_byte(addr);
  update_zero_and_negative_flags(_Y);
}

// Store operations
void CPU::op_sta(const u16 addr) { write_byte(addr, _A); }

void CPU::op_stx(const u16 addr) { write_byte(addr, _X); }

void CPU::op_sty(const u16 addr) { write_byte(addr, _Y); }

// Transfer operations
void CPU::op_tax(const u16 addr) {
  _X = _A;
  update_zero_and_negative_flags(_X);
}

void CPU::op_tay(const u16 addr) {
  _Y = _A;
  update_zero_and_negative_flags(_Y);
}

void CPU::op_txa(const u16 addr) {
  _A = _X;
  update_zero_and_negative_flags(_A);
}

void CPU::op_tsx(const u16 addr) {
  _X = _SP;
  update_zero_and_negative_flags(_X);
}

void CPU::op_txs(const u16 addr) { _SP = _X; }

void CPU::op_tya(const u16 addr) {
  _A = _Y;
  update_zero_and_negative_flags(_A);
}

// Stack operations
void CPU::op_pha(const u16 addr) {
  write_byte(0x0100 + _SP, _A);
  _SP--;
}

void CPU::op_php(const u16 addr) {
  // When pushing the status register, set bits 4 and 5 (B flag and unused flag)
  write_byte(0x0100 + _SP, _status | 0x30);
  _SP--;
}

void CPU::op_pla(const u16 addr) {
  _SP++;
  _A = read_byte(0x0100 + _SP);
  update_zero_and_negative_flags(_A);
}

void CPU::op_plp(const u16 addr) {
  _SP++;
  uint8_t pulled_status = read_byte(0x0100 + _SP);
  uint8_t break_flag = _status & 0x10;

  // Set the status register with the pulled value
  // but preserve the Break flag and force Unused flag set
  _status = (pulled_status & ~0x10) | break_flag | 0x20;
}

// ASL operations
void CPU::op_asl_acc(const u16 addr) {
  set_flag(Flag::CARRY, (_A & 0x80) != 0);
  _A <<= 1;
  update_zero_and_negative_flags(_A);
}

void CPU::op_asl(const u16 addr) {
  u8 value = read_byte(addr);
  set_flag(Flag::CARRY, (value & 0x80) != 0);
  value <<= 1;
  write_byte(addr, value);
  update_zero_and_negative_flags(value);
}

// LSR operations
void CPU::op_lsr_acc(const u16 addr) {
  set_flag(Flag::CARRY, (_A & 0x01) != 0);
  _A >>= 1;
  set_flag(Flag::NEGATIVE, 0);
  set_flag(Flag::ZERO, _A == 0);
}

void CPU::op_lsr(const u16 addr) {
  u8 value = read_byte(addr);
  set_flag(Flag::CARRY, (value & 0x01) != 0);
  value >>= 1;
  write_byte(addr, value);
  set_flag(Flag::NEGATIVE, 0);
  set_flag(Flag::ZERO, value == 0);
}

} // namespace nes
