#include "../include/cpu.h"
#include <string>
#include "types.h"

namespace nes {

CPU::CPU(Bus &bus_ref)
  : _bus(bus_ref) {
  reset();

  // Initialize all opcodes as invalid
  _instruction_table.fill({.addressed_op = nullptr, .mode = nullptr, .cycles = 0, .name = "???"});
  auto set_op = [this](const Opcode &op, const Instruction &instr) { _instruction_table[(u8)op] = instr; };

  // LDA
  set_op(Opcode::LDA_IMM, {.addressed_op = &CPU::op_lda, .mode = &CPU::immediate, .cycles = 2, .name = "LDA"});
  set_op(Opcode::LDA_ZPG, {.addressed_op = &CPU::op_lda, .mode = &CPU::zero_page, .cycles = 3, .name = "LDA"});
  set_op(Opcode::LDA_ABS, {.addressed_op = &CPU::op_lda, .mode = &CPU::absolute, .cycles = 4, .name = "LDA"});
  set_op(Opcode::LDA_ABX, {.addressed_op = &CPU::op_lda, .mode = &CPU::absolute_x, .cycles = 4, .name = "LDA", .is_extra_cycle = true});
  set_op(Opcode::LDA_ABY, {.addressed_op = &CPU::op_lda, .mode = &CPU::absolute_y, .cycles = 4, .name = "LDA", .is_extra_cycle = true});
  set_op(Opcode::LDA_ZPX, {.addressed_op = &CPU::op_lda, .mode = &CPU::zero_page_x, .cycles = 4, .name = "LDA"});
  set_op(Opcode::LDA_IZX, {.addressed_op = &CPU::op_lda, .mode = &CPU::indirect_x, .cycles = 6, .name = "LDA"});
  set_op(Opcode::LDA_IZY, {.addressed_op = &CPU::op_lda, .mode = &CPU::indirect_y, .cycles = 5, .name = "LDA", .is_extra_cycle = true});

  // LDX
  set_op(Opcode::LDX_IMM, {.addressed_op = &CPU::op_ldx, .mode = &CPU::immediate, .cycles = 2, .name = "LDX"});
  set_op(Opcode::LDX_ABS, {.addressed_op = &CPU::op_ldx, .mode = &CPU::absolute, .cycles = 4, .name = "LDX"});
  set_op(Opcode::LDX_ABY, {.addressed_op = &CPU::op_ldx, .mode = &CPU::absolute_y, .cycles = 4, .name = "LDX", .is_extra_cycle = true});
  set_op(Opcode::LDX_ZPG, {.addressed_op = &CPU::op_ldx, .mode = &CPU::zero_page, .cycles = 3, .name = "LDX"});
  set_op(Opcode::LDX_ZPY, {.addressed_op = &CPU::op_ldx, .mode = &CPU::zero_page_y, .cycles = 4, .name = "LDX"});

  // LDY
  set_op(Opcode::LDY_IMM, {.addressed_op = &CPU::op_ldy, .mode = &CPU::immediate, .cycles = 2, .name = "LDY"});
  set_op(Opcode::LDY_ABS, {.addressed_op = &CPU::op_ldy, .mode = &CPU::absolute, .cycles = 4, .name = "LDY"});
  set_op(Opcode::LDY_ABX, {.addressed_op = &CPU::op_ldy, .mode = &CPU::absolute_x, .cycles = 4, .name = "LDY", .is_extra_cycle = true});
  set_op(Opcode::LDY_ZPG, {.addressed_op = &CPU::op_ldy, .mode = &CPU::zero_page, .cycles = 3, .name = "LDY"});
  set_op(Opcode::LDY_ZPX, {.addressed_op = &CPU::op_ldy, .mode = &CPU::zero_page_x, .cycles = 4, .name = "LDY"});

  // STA
  set_op(Opcode::STA_ABS, {.addressed_op = &CPU::op_sta, .mode = &CPU::absolute, .cycles = 4, .name = "STA"});
  set_op(Opcode::STA_ABX, {.addressed_op = &CPU::op_sta, .mode = &CPU::absolute_x, .cycles = 5, .name = "STA"});
  set_op(Opcode::STA_ABY, {.addressed_op = &CPU::op_sta, .mode = &CPU::absolute_y, .cycles = 5, .name = "STA"});
  set_op(Opcode::STA_ZPG, {.addressed_op = &CPU::op_sta, .mode = &CPU::zero_page, .cycles = 3, .name = "STA"});
  set_op(Opcode::STA_ZPX, {.addressed_op = &CPU::op_sta, .mode = &CPU::zero_page_x, .cycles = 4, .name = "STA"});
  set_op(Opcode::STA_IZX, {.addressed_op = &CPU::op_sta, .mode = &CPU::indirect_x, .cycles = 6, .name = "STA"});
  set_op(Opcode::STA_IZY, {.addressed_op = &CPU::op_sta, .mode = &CPU::indirect_y, .cycles = 6, .name = "STA"});

  // STX
  set_op(Opcode::STX_ABS, {.addressed_op = &CPU::op_stx, .mode = &CPU::absolute, .cycles = 4, .name = "STX"});
  set_op(Opcode::STX_ZPG, {.addressed_op = &CPU::op_stx, .mode = &CPU::zero_page, .cycles = 3, .name = "STX"});
  set_op(Opcode::STX_ZPY, {.addressed_op = &CPU::op_stx, .mode = &CPU::zero_page_y, .cycles = 4, .name = "STX"});

  // STY
  set_op(Opcode::STY_ABS, {.addressed_op = &CPU::op_sty, .mode = &CPU::absolute, .cycles = 4, .name = "STY"});
  set_op(Opcode::STY_ZPG, {.addressed_op = &CPU::op_sty, .mode = &CPU::zero_page, .cycles = 3, .name = "STY"});
  set_op(Opcode::STY_ZPX, {.addressed_op = &CPU::op_sty, .mode = &CPU::zero_page_x, .cycles = 4, .name = "STY"});

  // Transfer operations (implied addressing)
  set_op(Opcode::TAX_IMP, {.implied_op = &CPU::op_tax, .mode = nullptr, .cycles = 2, .name = "TAX", .is_implied = true});
  set_op(Opcode::TAY_IMP, {.implied_op = &CPU::op_tay, .mode = nullptr, .cycles = 2, .name = "TAY", .is_implied = true});
  set_op(Opcode::TSX_IMP, {.implied_op = &CPU::op_tsx, .mode = nullptr, .cycles = 2, .name = "TSX", .is_implied = true});
  set_op(Opcode::TYA_IMP, {.implied_op = &CPU::op_tya, .mode = nullptr, .cycles = 2, .name = "TYA", .is_implied = true});
  set_op(Opcode::TXS_IMP, {.implied_op = &CPU::op_txs, .mode = nullptr, .cycles = 2, .name = "TXS", .is_implied = true});
  set_op(Opcode::TXA_IMP, {.implied_op = &CPU::op_txa, .mode = nullptr, .cycles = 2, .name = "TXA", .is_implied = true});

  // Stack operations (implied addressing)
  set_op(Opcode::PHA_IMP, {.implied_op = &CPU::op_pha, .mode = nullptr, .cycles = 3, .name = "PHA", .is_implied = true});
  set_op(Opcode::PLA_IMP, {.implied_op = &CPU::op_pla, .mode = nullptr, .cycles = 4, .name = "PLA", .is_implied = true});
  set_op(Opcode::PLP_IMP, {.implied_op = &CPU::op_plp, .mode = nullptr, .cycles = 4, .name = "PLP", .is_implied = true});
  set_op(Opcode::PHP_IMP, {.implied_op = &CPU::op_php, .mode = nullptr, .cycles = 3, .name = "PHP", .is_implied = true});

  // ASL
  set_op(Opcode::ASL_ACC, {.implied_op = &CPU::op_asl_acc, .mode = nullptr, .cycles = 2, .name = "ASL", .is_implied = true});
  set_op(Opcode::ASL_ABS, {.addressed_op = &CPU::op_asl, .mode = &CPU::absolute, .cycles = 6, .name = "ASL"});
  set_op(Opcode::ASL_ABX, {.addressed_op = &CPU::op_asl, .mode = &CPU::absolute_x, .cycles = 7, .name = "ASL"});
  set_op(Opcode::ASL_ZPG, {.addressed_op = &CPU::op_asl, .mode = &CPU::zero_page, .cycles = 5, .name = "ASL"});
  set_op(Opcode::ASL_ZPX, {.addressed_op = &CPU::op_asl, .mode = &CPU::zero_page_x, .cycles = 6, .name = "ASL"});

  // LSR
  set_op(Opcode::LSR_ACC, {.implied_op = &CPU::op_lsr_acc, .mode = nullptr, .cycles = 2, .name = "LSR", .is_implied = true});
  set_op(Opcode::LSR_ABS, {.addressed_op = &CPU::op_lsr, .mode = &CPU::absolute, .cycles = 6, .name = "LSR"});
  set_op(Opcode::LSR_ABX, {.addressed_op = &CPU::op_lsr, .mode = &CPU::absolute_x, .cycles = 7, .name = "LSR"});
  set_op(Opcode::LSR_ZPG, {.addressed_op = &CPU::op_lsr, .mode = &CPU::zero_page, .cycles = 5, .name = "LSR"});
  set_op(Opcode::LSR_ZPX, {.addressed_op = &CPU::op_lsr, .mode = &CPU::zero_page_x, .cycles = 6, .name = "LSR"});

  // ROL
  set_op(Opcode::ROL_ACC, {.implied_op = &CPU::op_rol_acc, .mode = nullptr, .cycles = 2, .name = "ROL", .is_implied = true});
  set_op(Opcode::ROL_ABS, {.addressed_op = &CPU::op_rol, .mode = &CPU::absolute, .cycles = 6, .name = "ROL"});
  set_op(Opcode::ROL_ABX, {.addressed_op = &CPU::op_rol, .mode = &CPU::absolute_x, .cycles = 7, .name = "ROL"});
  set_op(Opcode::ROL_ZPG, {.addressed_op = &CPU::op_rol, .mode = &CPU::zero_page, .cycles = 5, .name = "ROL"});
  set_op(Opcode::ROL_ZPX, {.addressed_op = &CPU::op_rol, .mode = &CPU::zero_page_x, .cycles = 6, .name = "ROL"});

  // ROR
  set_op(Opcode::ROR_ACC, {.implied_op = &CPU::op_ror_acc, .mode = nullptr, .cycles = 2, .name = "ROR", .is_implied = true});
  set_op(Opcode::ROR_ABS, {.addressed_op = &CPU::op_ror, .mode = &CPU::absolute, .cycles = 6, .name = "ROR"});
  set_op(Opcode::ROR_ABX, {.addressed_op = &CPU::op_ror, .mode = &CPU::absolute_x, .cycles = 7, .name = "ROR"});
  set_op(Opcode::ROR_ZPG, {.addressed_op = &CPU::op_ror, .mode = &CPU::zero_page, .cycles = 5, .name = "ROR"});
  set_op(Opcode::ROR_ZPX, {.addressed_op = &CPU::op_ror, .mode = &CPU::zero_page_x, .cycles = 6, .name = "ROR"});

  // Flags
  set_op(Opcode::CLC_IMP, {.implied_op = &CPU::op_clc, .mode = nullptr, .cycles = 2, .name = "CLC", .is_implied = true});
  set_op(Opcode::SEC_IMP, {.implied_op = &CPU::op_sec, .mode = nullptr, .cycles = 2, .name = "SEC", .is_implied = true});
}

void CPU::clock() {
  if (_cycles == 0) {
    u8 opcode = read_byte(_PC++);
    set_flag(Flag::UNUSED, true);

    const auto &instruction = _instruction_table[opcode];
    if (instruction.cycles == 0) throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));

    _cycles = instruction.cycles;

    if (instruction.is_implied) {
      // Handle implied addressing operations
      (this->*(instruction.implied_op))();
    } else {
      // Handle operations that require addressing
      auto addr_mode = instruction.mode;
      u16 addr = 0;
      if (addr_mode != nullptr) {
        addr = (this->*addr_mode)();

        if (_page_crossed && instruction.is_extra_cycle) {
          _cycles++;
          _page_crossed = false;
        }
      }

      (this->*(instruction.addressed_op))(addr);
    }
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

void CPU::write_byte(const u16 address, const u8 value) { _bus.write(address, value); }

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

u16 CPU::immediate() {
  return _PC++;  // Return the PC then increment it
}

u16 CPU::zero_page() {
  return read_byte(_PC++);  // Zero page address is just a single byte
}

u16 CPU::zero_page_x() {
  u8 zp_addr = read_byte(_PC++);
  return (u16)((zp_addr + _X) & 0xFF);  // Wrap around in zero page
}

u16 CPU::zero_page_y() {
  u8 zp_addr = read_byte(_PC++);
  return (u16)((zp_addr + _Y) & 0xFF);  // Wrap around in zero page
}

u16 CPU::absolute() {
  u16 addr_low = read_byte(_PC++);
  u16 addr_high = read_byte(_PC++);
  return (addr_high << 8) | addr_low;
}

u16 CPU::absolute_x() {
  u16 addr_low = read_byte(_PC++);
  u16 addr_high = read_byte(_PC++);
  u16 base_addr = (addr_high << 8) | addr_low;
  u16 final_addr = base_addr + _X;

  _page_crossed = ((base_addr & 0xFF00) != (final_addr & 0xFF00));
  return final_addr;
}

u16 CPU::absolute_y() {
  u16 base_addr = absolute();
  u16 final_addr = base_addr + _Y;

  _page_crossed = ((base_addr & 0xFF00) != (final_addr & 0xFF00));
  return final_addr;
}

u16 CPU::indirect_x() {
  u8 zp_addr = read_byte(_PC++);
  zp_addr += _X;  // Add X to the zero page address (with wrap)

  // Read two bytes from the computed zero page address
  u16 effective_addr_low = read_byte(zp_addr);
  u16 effective_addr_high = read_byte((u16)((zp_addr + 1) & 0xFF));

  return (effective_addr_high << 8) | effective_addr_low;
}

u16 CPU::indirect_y() {
  u8 zp_addr = read_byte(_PC++);

  u16 effective_addr_low = read_byte(zp_addr);
  u16 effective_addr_high = read_byte(((u16)((zp_addr + 1) & 0xFF)));

  u16 base_addr = (effective_addr_high << 8) | effective_addr_low;
  u16 final_addr = base_addr + _Y;

  _page_crossed = ((base_addr & 0xFF00) != (final_addr & 0xFF00));

  return final_addr;
}

//////////////////////////////////////////////////////////////////////////
// ADDRESSED OPERATIONS (operations that need an address)
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

// ASL (addressed version)
void CPU::op_asl(const u16 addr) {
  u8 value = read_byte(addr);
  set_flag(Flag::CARRY, (value & 0x80) != 0);
  value <<= 1;
  write_byte(addr, value);
  update_zero_and_negative_flags(value);
}

// LSR (addressed version)
void CPU::op_lsr(const u16 addr) {
  u8 value = read_byte(addr);
  set_flag(Flag::CARRY, (value & 0x01) != 0);
  value >>= 1;
  write_byte(addr, value);
  set_flag(Flag::NEGATIVE, 0);
  set_flag(Flag::ZERO, value == 0);
}

// ROL
void CPU::op_rol(const u16 addr) {
  u8 value = read_byte(addr);
  bool carry_bit = (value & 0x80) != 0;
  value <<= 1;

  // Put the old carry flag into bit 0
  if (get_flag(Flag::CARRY)) {
    value |= 0x01;
  }

  set_flag(Flag::CARRY, carry_bit);
  write_byte(addr, value);
  update_zero_and_negative_flags(value);
}

// ROR
void CPU::op_ror(const u16 addr) {
  u8 value = read_byte(addr);
  bool carry_bit = (value & 0x01) != 0;
  value >>= 1;

  // Put the old carry flag into bit 7
  if (get_flag(Flag::CARRY)) {
    value |= 0x80;
  }

  set_flag(Flag::CARRY, carry_bit);
  write_byte(addr, value);
  update_zero_and_negative_flags(value);
}

//////////////////////////////////////////////////////////////////////////
// IMPLIED OPERATIONS (operations that don't need an address)
//////////////////////////////////////////////////////////////////////////

// Transfer operations
void CPU::op_tax() {
  _X = _A;
  update_zero_and_negative_flags(_X);
}

void CPU::op_tay() {
  _Y = _A;
  update_zero_and_negative_flags(_Y);
}

void CPU::op_txa() {
  _A = _X;
  update_zero_and_negative_flags(_A);
}

void CPU::op_tsx() {
  _X = _SP;
  update_zero_and_negative_flags(_X);
}

void CPU::op_txs() { _SP = _X; }

void CPU::op_tya() {
  _A = _Y;
  update_zero_and_negative_flags(_A);
}

// Stack operations
void CPU::op_pha() {
  write_byte(0x0100 + _SP, _A);
  _SP--;
}

void CPU::op_php() {
  // When pushing the status register, set bits a4 and 5 (B flag and unused flag)
  write_byte(0x0100 + _SP, _status | 0x30);
  _SP--;
}

void CPU::op_pla() {
  _SP++;
  _A = read_byte(0x0100 + _SP);
  update_zero_and_negative_flags(_A);
}

void CPU::op_plp() {
  _SP++;
  uint8_t pulled_status = read_byte(0x0100 + _SP);
  uint8_t break_flag = _status & 0x10;

  // Set the status register with the pulled value
  // but preserve the Break flag and force Unused flag set
  _status = (pulled_status & ~0x10) | break_flag | 0x20;
}

// ASL, LSR, ROL accumulator operations
void CPU::op_asl_acc() {
  set_flag(Flag::CARRY, (_A & 0x80) != 0);
  _A <<= 1;
  update_zero_and_negative_flags(_A);
}

void CPU::op_lsr_acc() {
  set_flag(Flag::CARRY, (_A & 0x01) != 0);
  _A >>= 1;
  set_flag(Flag::NEGATIVE, 0);
  set_flag(Flag::ZERO, _A == 0);
}

void CPU::op_rol_acc() {
  bool carry_bit = (_A & 0x80) != 0;
  _A <<= 1;

  // Put the old carry flag into bit 0
  if (get_flag(Flag::CARRY)) {
    _A |= 0x01;
  }

  set_flag(Flag::CARRY, carry_bit);
  update_zero_and_negative_flags(_A);
}

void CPU::op_ror_acc() {
  bool carry_bit = (_A & 0x01) != 0;
  _A >>= 1;

  // Put the old carry flag into bit 7
  if (get_flag(Flag::CARRY)) {
    _A |= 0x80;
  }

  set_flag(Flag::CARRY, carry_bit);
  update_zero_and_negative_flags(_A);
}

// Flag operations
void CPU::op_sec() { set_flag(Flag::CARRY, true); }
void CPU::op_clc() { set_flag(Flag::CARRY, false); }

}  // namespace nes
