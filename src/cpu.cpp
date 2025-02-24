#include "../include/cpu.h"
#include "types.h"
#include <string>

namespace nes {

CPU::CPU(Bus &bus_ref) : _bus(bus_ref) {
  reset();
  _instruction_table = {
      // LDA
      {(u8)Opcode::LDA_IM, {&CPU::lda_immediate, 2, "LDA"}},
      {(u8)Opcode::LDA_ZP, {&CPU::lda_zero_page, 3, "LDA"}},
      {(u8)Opcode::LDA_ABS, {&CPU::lda_absolute, 4, "LDA"}},
      {(u8)Opcode::LDA_XABS, {&CPU::lda_absolute_x, 4, "LDA"}},
      {(u8)Opcode::LDA_YABS, {&CPU::lda_absolute_y, 4, "LDA"}},
      {(u8)Opcode::LDA_XZP, {&CPU::lda_zero_page_x, 4, "LDA"}},
      {(u8)Opcode::LDA_XZPI, {&CPU::lda_indirect_x, 6, "LDA"}},
      {(u8)Opcode::LDA_YZPI, {&CPU::lda_indirect_y, 5, "LDA"}},
      // LDX
      {(u8)Opcode::LDX_IM, {&CPU::ldx_immediate, 2, "LDX"}},
      {(u8)Opcode::LDX_ABS, {&CPU::ldx_absolute, 4, "LDX"}},
      {(u8)Opcode::LDX_YABS, {&CPU::ldx_absolute_y, 4, "LDX"}},
      {(u8)Opcode::LDX_ZP, {&CPU::ldx_zero_page, 3, "LDX"}},
      {(u8)Opcode::LDX_YZP, {&CPU::ldx_zero_page, 4, "LDX"}},
      // LDY
      {(u8)Opcode::LDY_IM, {&CPU::ldy_immediate, 2, "LDY"}},
      {(u8)Opcode::LDY_ABS, {&CPU::ldy_absolute, 4, "LDY"}},
      {(u8)Opcode::LDY_XABS, {&CPU::ldy_absolute_x, 4, "LDY"}},
      {(u8)Opcode::LDY_ZP, {&CPU::ldy_zero_page, 3, "LDY"}},
      {(u8)Opcode::LDY_XZP, {&CPU::ldy_zero_page_x, 4, "LDY"}},
      // STA
      {(u8)Opcode::STA_ABS, {&CPU::sta_absolute, 4, "STA"}},
      {(u8)Opcode::STA_XABS, {&CPU::sta_absolute_x, 5, "STA"}},
      {(u8)Opcode::STA_YABS, {&CPU::sta_absolute_y, 5, "STA"}},
      {(u8)Opcode::STA_ZP, {&CPU::sta_zero_page, 3, "STA"}},
      {(u8)Opcode::STA_XZP, {&CPU::sta_zero_page_x, 4, "STA"}},
      {(u8)Opcode::STA_XZPI, {&CPU::sta_indirect_x, 6, "STA"}},
      {(u8)Opcode::STA_YZPI, {&CPU::sta_indirect_y, 6, "STA"}},
      // STX
      {(u8)Opcode::STX_ABS, {&CPU::stx_absolute, 4, "STX"}},
      {(u8)Opcode::STX_ZP, {&CPU::stx_zero_page, 3, "STX"}},
      {(u8)Opcode::STX_YZP, {&CPU::stx_zero_page_y, 4, "STX"}},
      // STY
      {(u8)Opcode::STY_ABS, {&CPU::sty_absolute, 4, "STY"}},
      {(u8)Opcode::STY_ZP, {&CPU::sty_zero_page, 3, "STY"}},
      {(u8)Opcode::STY_XZP, {&CPU::sty_zero_page_x, 4, "STY"}},
      // Transfer
      {(u8)Opcode::TAX, {&CPU::tax, 2, "TAX"}},
      {(u8)Opcode::TAY, {&CPU::tay, 2, "TAY"}},
      {(u8)Opcode::TSX, {&CPU::tsx, 2, "TSX"}},
      {(u8)Opcode::TYA, {&CPU::tya, 2, "TYA"}},
      {(u8)Opcode::TXS, {&CPU::txs, 2, "TXS"}},
      {(u8)Opcode::TXA, {&CPU::txa, 2, "TXA"}},
      // Stack
      {(u8)Opcode::PHA, {&CPU::pha, 3, "PHA"}},
      {(u8)Opcode::PLA, {&CPU::pla, 4, "PHA"}},
      {(u8)Opcode::PLP, {&CPU::plp, 4, "PLP"}},
      {(u8)Opcode::PHP, {&CPU::php, 3, "PHP"}},
      // ASL
      {(u8)Opcode::ASL_ACC, {&CPU::asl_accumulator, 2, "ASL"}},
      {(u8)Opcode::ASL_XABS, {&CPU::asl_absolute_x, 7, "ASL"}},
      {(u8)Opcode::ASL_ZP, {&CPU::asl_zero_page, 5, "ASL"}},
      {(u8)Opcode::ASL_XZP, {&CPU::asl_zero_page_x, 6, "ASL"}},
      {(u8)Opcode::ASL_ABS, {&CPU::asl_absolute, 6, "ASL"}}};
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
    (this->*it->second.handler)();
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
u8 CPU::read_byte(u16 address) { return _bus.read(address); }
void CPU::write_byte(u16 address, u8 value) { _bus.write(address, value); }

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

void CPU::set_flag(Flag flag, bool value) {
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

// Instruction Handlers
// LDX
void CPU::lda_immediate() {
  _A = read_byte(_PC++);
  update_zero_and_negative_flags(_A);
}

void CPU::lda_zero_page() {
  u16 addr = addr_zero_page();
  _A = read_byte(addr);
  update_zero_and_negative_flags(_A);
}

void CPU::lda_absolute() {
  u16 addr = addr_absolute();
  _A = read_byte(addr);
  update_zero_and_negative_flags(_A);
}

void CPU::lda_absolute_x() {
  u16 base_addr = addr_absolute();
  u16 addr = addr_absolute_x();
  if (check_page_cross(base_addr, addr)) {
    _cycles++;
  }
  _A = read_byte(addr);
  update_zero_and_negative_flags(_A);
}

void CPU::lda_absolute_y() {
  u16 base_addr = addr_absolute();
  u16 addr = addr_absolute_y();
  if (check_page_cross(base_addr, addr)) {
    _cycles++;
  }
  _A = read_byte(addr);
  update_zero_and_negative_flags(_A);
}

void CPU::lda_zero_page_x() {
  u16 addr = addr_zero_page_x();
  _A = read_byte(addr);
  update_zero_and_negative_flags(_A);
}

void CPU::lda_indirect_x() {
  u16 addr = addr_indirect_x();
  _A = read_byte(addr);
  update_zero_and_negative_flags(_A);
}

void CPU::lda_indirect_y() {
  u8 zp_addr = read_byte(_PC++);
  u16 addr = addr_indirect_y(zp_addr);
  if (check_page_cross(addr - _Y, addr)) {
    _cycles++;
  }
  _A = read_byte(addr);
  update_zero_and_negative_flags(_A);
}

// LDX
void CPU::ldx_immediate() {
  _X = read_byte(_PC++);
  update_zero_and_negative_flags(_X);
}

void CPU::ldx_zero_page() {
  u16 addr = addr_zero_page();
  _X = read_byte(addr);
  update_zero_and_negative_flags(_X);
}

void CPU::ldx_absolute() {
  u16 addr = addr_absolute();
  _X = read_byte(addr);
  update_zero_and_negative_flags(_X);
}

void CPU::ldx_absolute_y() {
  u16 base_addr = addr_absolute();
  u16 addr = addr_absolute_y();
  if (check_page_cross(base_addr, addr)) {
    _cycles++;
  }
  _X = read_byte(addr);
  update_zero_and_negative_flags(_X);
}

void CPU::ldx_zero_page_y() {
  u8 zp_addr = read_byte(_PC++);
  u16 addr = addr_indirect_y(zp_addr);
  if (check_page_cross(addr - _Y, addr)) {
    _cycles++;
  }
  _X = read_byte(addr);
  update_zero_and_negative_flags(_X);
}

// LDY
void CPU::ldy_immediate() {
  _Y = read_byte(_PC++);
  update_zero_and_negative_flags(_Y);
}

void CPU::ldy_zero_page() {
  u16 addr = addr_zero_page();
  _Y = read_byte(addr);
  update_zero_and_negative_flags(_Y);
}

void CPU::ldy_absolute() {
  u16 addr = addr_absolute();
  _Y = read_byte(addr);
  update_zero_and_negative_flags(_Y);
}

void CPU::ldy_zero_page_x() {
  u16 addr = addr_zero_page_x();
  _Y = read_byte(addr);
  update_zero_and_negative_flags(_Y);
}

void CPU::ldy_absolute_x() {
  u16 base_addr = addr_absolute();
  u16 addr = base_addr + _X;
  if (check_page_cross(base_addr, addr)) {
    _cycles++;
  }
  _Y = read_byte(addr);
  update_zero_and_negative_flags(_Y);
}

// STA
void CPU::sta_zero_page() {
  u16 addr = addr_zero_page();
  write_byte(addr, _A);
}

void CPU::sta_absolute() {
  u16 addr = addr_absolute();
  write_byte(addr, _A);
}

void CPU::sta_absolute_x() {
  u16 addr = addr_absolute_x();
  write_byte(addr, _A);
}

void CPU::sta_absolute_y() {
  u16 addr = addr_absolute_y();
  write_byte(addr, _A);
}

void CPU::sta_zero_page_x() {
  u16 addr = addr_zero_page_x();
  write_byte(addr, _A);
}

void CPU::sta_indirect_x() {
  u16 addr = addr_indirect_x();
  write_byte(addr, _A);
}

void CPU::sta_indirect_y() {
  u8 zp_addr = read_byte(_PC++);
  u16 addr = addr_indirect_y(zp_addr);
  write_byte(addr, _A);
}

// STX
void CPU::stx_zero_page() {
  u16 addr = addr_zero_page();
  write_byte(addr, _X);
}

void CPU::stx_absolute() {
  u16 addr = addr_absolute();
  write_byte(addr, _X);
}

void CPU::stx_zero_page_y() {
  u16 addr = addr_zero_page_y();
  write_byte(addr, _X);
}

// STY
void CPU::sty_zero_page() {
  u16 addr = addr_zero_page();
  write_byte(addr, _Y);
}

void CPU::sty_absolute() {
  u16 addr = addr_absolute();
  write_byte(addr, _Y);
}

void CPU::sty_zero_page_x() {
  u16 addr = addr_zero_page_x();
  write_byte(addr, _Y);
}

void CPU::tax() {
  _X = _A;
  update_zero_and_negative_flags(_X);
}

void CPU::tay() {
  _Y = _A;
  update_zero_and_negative_flags(_Y);
}

void CPU::txa() {
  _A = _X;
  update_zero_and_negative_flags(_A);
}

void CPU::tsx() {
  _X = _SP;
  update_zero_and_negative_flags(_X);
}

void CPU::txs() { _SP = _X; }

void CPU::tya() {
  _A = _Y;
  update_zero_and_negative_flags(_A);
}

// Stack
void CPU::pha() { _bus.write(0x0100 + _SP--, _A); }
void CPU::php() { _bus.write(0x0100 + _SP--, _status); }
void CPU::pla() {
  _A = _bus.read(0x0100 + ++_SP);
  update_zero_and_negative_flags(_A);
}

void CPU::plp() {
  uint8_t pulled_status = _bus.read(0x0100 + ++_SP);
  uint8_t break_flag = _status & 0x10;

  // Set the status register with the pulled value
  // but preserve the Break flag and force Unused flag set
  _status = (pulled_status & ~0x10) | break_flag | 0x20;
}

// ASL
void CPU::asl_accumulator() {
  set_flag(Flag::CARRY, (_A & 0x80) != 0);
  _A <<= 1;
  update_zero_and_negative_flags(_A);
}

void CPU::asl_absolute() {
  u16 addr = addr_absolute();
  u8 value = read_byte(addr);

  set_flag(Flag::CARRY, (value & 0x80) != 0);

  value <<= 1;
  write_byte(addr, value);
  update_zero_and_negative_flags(value);
}

void CPU::asl_absolute_x() {
  u16 addr = addr_absolute_x();
  u8 value = read_byte(addr);

  set_flag(Flag::CARRY, (value & 0x80) != 0);

  value <<= 1;
  write_byte(addr, value);
  update_zero_and_negative_flags(value);
}

void CPU::asl_zero_page() {
  u16 addr = addr_zero_page();
  u8 value = read_byte(addr);

  set_flag(Flag::CARRY, (value & 0x80) != 0);

  value <<= 1;
  write_byte(addr, value);
  update_zero_and_negative_flags(value);
}

void CPU::asl_zero_page_x() {
  u16 addr = addr_zero_page_x();
  u8 value = read_byte(addr);

  set_flag(Flag::CARRY, (value & 0x80) != 0);

  value <<= 1;
  write_byte(addr, value);
  update_zero_and_negative_flags(value);
}

// Addressing modes
u16 CPU::addr_zero_page() { return read_byte(_PC++); }

u16 CPU::addr_zero_page_x() {
  u8 zp_addr = read_byte(_PC++);
  return static_cast<u16>((zp_addr + _X) & 0xFF);
}

u16 CPU::addr_zero_page_y() {
  u8 zp_addr = read_byte(_PC++);
  return static_cast<u16>((zp_addr + _Y) & 0xFF);
}

u16 CPU::addr_absolute() {
  u16 addr_low = read_byte(_PC++);
  u16 addr_high = read_byte(_PC++);
  return (addr_high << 8) | addr_low;
}

u16 CPU::addr_absolute_x() {
  u16 base_addr = addr_absolute();
  return base_addr + _X;
}

u16 CPU::addr_absolute_y() {
  u16 base_addr = addr_absolute();
  return base_addr + _Y;
}

u16 CPU::addr_indirect_x() {
  u8 zp_addr = read_byte(_PC++);
  zp_addr += _X;
  u16 effective_addr_low = read_byte(zp_addr);
  u16 effective_addr_high = read_byte(static_cast<u16>(zp_addr + 1));
  return (effective_addr_high << 8) | effective_addr_low;
}

u16 CPU::addr_indirect_y(u8 zp_addr) {
  u16 effective_addr_low = read_byte(zp_addr);
  u16 effective_addr_high = read_byte(static_cast<u16>(zp_addr + 1));
  return ((effective_addr_high << 8) | effective_addr_low) + _Y;
}

bool CPU::check_page_cross(u16 addr1, u16 addr2) {
  return (addr1 & 0xFF00) != (addr2 & 0xFF00);
}
} // namespace nes
