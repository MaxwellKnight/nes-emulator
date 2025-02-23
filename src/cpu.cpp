#include "../include/cpu.h"
#include <stdexcept>

namespace nes {

CPU::CPU(Bus &bus_ref) : bus(bus_ref) {
  reset();
  instruction_table = {
      {static_cast<u8>(Opcode::LDA_IM), {&CPU::lda_immediate, 2, "LDA"}},
      {static_cast<u8>(Opcode::LDA_ZP), {&CPU::lda_zero_page, 3, "LDA"}},
      {static_cast<u8>(Opcode::LDA_ABS), {&CPU::lda_absolute, 4, "LDA"}},
      {static_cast<u8>(Opcode::LDA_XABS), {&CPU::lda_absolute_x, 4, "LDA"}},
      {static_cast<u8>(Opcode::LDA_YABS), {&CPU::lda_absolute_y, 4, "LDA"}},
      {static_cast<u8>(Opcode::LDA_XZP), {&CPU::lda_zero_page_x, 4, "LDA"}},
      {static_cast<u8>(Opcode::LDA_XZPI), {&CPU::lda_indirect_x, 6, "LDA"}},
      {static_cast<u8>(Opcode::LDA_YZPI), {&CPU::lda_indirect_y, 5, "LDA"}},
      // LDX
      {static_cast<u8>(Opcode::LDX_IM), {&CPU::ldx_immediate, 2, "LDX"}},
      {static_cast<u8>(Opcode::LDX_ABS), {&CPU::ldx_absolute, 4, "LDX"}},
      {static_cast<u8>(Opcode::LDX_YABS), {&CPU::ldx_absolute_y, 4, "LDX"}},
      {static_cast<u8>(Opcode::LDX_ZP), {&CPU::ldx_zero_page, 3, "LDX"}},
      {static_cast<u8>(Opcode::LDX_YZP), {&CPU::ldx_zero_page, 4, "LDX"}},
      // LDY
      {static_cast<u8>(Opcode::LDY_IM), {&CPU::ldy_immediate, 2, "LDY"}},
      {static_cast<u8>(Opcode::LDY_ABS), {&CPU::ldy_absolute, 4, "LDY"}},
      {static_cast<u8>(Opcode::LDY_XABS), {&CPU::ldy_absolute_x, 4, "LDY"}},
      {static_cast<u8>(Opcode::LDY_ZP), {&CPU::ldy_zero_page, 3, "LDY"}},
      {static_cast<u8>(Opcode::LDY_XZP), {&CPU::ldy_zero_page_x, 4, "LDY"}},
      //
      {static_cast<u8>(Opcode::STA_ZP),
       {&CPU::sta_zero_page, 3, "STA Zero Page"}},
      {static_cast<u8>(Opcode::TAX), {&CPU::tax, 2, "TAX"}},
      {static_cast<u8>(Opcode::TXA), {&CPU::txa, 2, "TXA"}}};
}

void CPU::clock() {
  if (cycles == 0) {
    u8 opcode = read_byte(PC++);
    set_flag(Flag::UNUSED, true);

    auto it = instruction_table.find(opcode);
    if (it == instruction_table.end()) {
      throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
    }

    cycles = it->second.cycles;
    (this->*it->second.handler)();
  }
  cycles--;
}

void CPU::reset() {
  A = 0;
  X = 0;
  Y = 0;
  SP = 0xFF;
  status = static_cast<u8>(Flag::UNUSED) | static_cast<u8>(Flag::BREAK);
  PC = 0xFFFC;
  cycles = 0;
}

// Getters
u8 CPU::get_accumulator() const { return A; }
u8 CPU::get_x() const { return X; }
u8 CPU::get_y() const { return Y; }
u16 CPU::get_pc() const { return PC; }
u8 CPU::get_sp() const { return SP; }
u8 CPU::get_status() const { return status; }
u8 CPU::get_remaining_cycles() const { return cycles; }

// Flag operations
void CPU::set_flag(Flag flag, bool value) {
  if (value) {
    status |= static_cast<u8>(flag);
  } else {
    status &= ~static_cast<u8>(flag);
  }
}

bool CPU::get_flag(Flag flag) const {
  return (status & static_cast<u8>(flag)) != 0;
}

void CPU::update_zero_and_negative_flags(u8 value) {
  set_flag(Flag::ZERO, value == 0);
  set_flag(Flag::NEGATIVE, (value & 0x80) != 0);
}

u8 CPU::read_byte(u16 address) { return bus.read(address); }

void CPU::write_byte(u16 address, u8 value) { bus.write(address, value); }

void CPU::push_stack(u8 value) {
  write_byte(0x0100 + SP, value);
  SP--;
}

u8 CPU::pull_stack() {
  SP++;
  return read_byte(0x0100 + SP);
}

// Instruction Handlers
// LDX
void CPU::lda_immediate() {
  A = read_byte(PC++);
  update_zero_and_negative_flags(A);
}

void CPU::lda_zero_page() {
  u16 addr = addr_zero_page();
  A = read_byte(addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_absolute() {
  u16 addr = addr_absolute();
  A = read_byte(addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_absolute_x() {
  u16 base_addr = addr_absolute();
  u16 addr = addr_absolute_x();
  if (check_page_cross(base_addr, addr)) {
    cycles++;
  }
  A = read_byte(addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_absolute_y() {
  u16 base_addr = addr_absolute();
  u16 addr = addr_absolute_y();
  if (check_page_cross(base_addr, addr)) {
    cycles++;
  }
  A = read_byte(addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_zero_page_x() {
  u16 addr = addr_zero_page_x();
  A = read_byte(addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_indirect_x() {
  u16 addr = addr_indirect_x();
  A = read_byte(addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_indirect_y() {
  u8 zp_addr = read_byte(PC++);
  u16 addr = addr_indirect_y(zp_addr);
  if (check_page_cross(addr - Y, addr)) {
    cycles++;
  }
  A = read_byte(addr);
  update_zero_and_negative_flags(A);
}

// LDX
void CPU::ldx_immediate() {
  X = read_byte(PC++);
  update_zero_and_negative_flags(X);
}

void CPU::ldx_zero_page() {
  u16 addr = addr_zero_page();
  X = read_byte(addr);
  update_zero_and_negative_flags(X);
}

void CPU::ldx_absolute() {
  u16 addr = addr_absolute();
  X = read_byte(addr);
  update_zero_and_negative_flags(X);
}

void CPU::ldx_absolute_y() {
  u16 base_addr = addr_absolute();
  u16 addr = addr_absolute_y();
  if (check_page_cross(base_addr, addr)) {
    cycles++;
  }
  X = read_byte(addr);
  update_zero_and_negative_flags(X);
}

void CPU::ldx_zero_page_y() {
  u8 zp_addr = read_byte(PC++);
  u16 addr = addr_indirect_y(zp_addr);
  if (check_page_cross(addr - Y, addr)) {
    cycles++;
  }
  X = read_byte(addr);
  update_zero_and_negative_flags(X);
}

// LDY
void CPU::ldy_immediate() {
  Y = read_byte(PC++);
  update_zero_and_negative_flags(Y);
}

void CPU::ldy_zero_page() {
  u16 addr = addr_zero_page();
  Y = read_byte(addr);
  update_zero_and_negative_flags(Y);
}

void CPU::ldy_absolute() {
  u16 addr = addr_absolute();
  Y = read_byte(addr);
  update_zero_and_negative_flags(Y);
}

void CPU::ldy_zero_page_x() {
  u16 addr = addr_zero_page_x();
  Y = read_byte(addr);
  update_zero_and_negative_flags(Y);
}

void CPU::ldy_absolute_x() {
  u16 base_addr = addr_absolute();
  u16 addr = base_addr + X;
  if (check_page_cross(base_addr, addr)) {
    cycles++;
  }
  Y = read_byte(addr);
  update_zero_and_negative_flags(Y);
}

//
void CPU::sta_zero_page() {
  u16 addr = addr_zero_page();
  write_byte(addr, A);
}

void CPU::tax() {
  X = A;
  update_zero_and_negative_flags(X);
}

void CPU::txa() {
  A = X;
  update_zero_and_negative_flags(A);
}

// Addressing modes
u16 CPU::addr_zero_page() { return read_byte(PC++); }

u16 CPU::addr_zero_page_x() {
  u8 zp_addr = read_byte(PC++);
  return static_cast<u16>((zp_addr + X) & 0xFF);
}

u16 CPU::addr_zero_page_y() {
  u8 zp_addr = read_byte(PC++);
  return static_cast<u16>((zp_addr + Y) & 0xFF);
}

u16 CPU::addr_absolute() {
  u16 addr_low = read_byte(PC++);
  u16 addr_high = read_byte(PC++);
  return (addr_high << 8) | addr_low;
}

u16 CPU::addr_absolute_x() {
  u16 base_addr = addr_absolute();
  return base_addr + X;
}

u16 CPU::addr_absolute_y() {
  u16 base_addr = addr_absolute();
  return base_addr + Y;
}

u16 CPU::addr_indirect_x() {
  u8 zp_addr = read_byte(PC++);
  zp_addr += X;
  u16 effective_addr_low = read_byte(zp_addr);
  u16 effective_addr_high = read_byte(static_cast<u16>(zp_addr + 1));
  return (effective_addr_high << 8) | effective_addr_low;
}

u16 CPU::addr_indirect_y(u8 zp_addr) {
  u16 effective_addr_low = read_byte(zp_addr);
  u16 effective_addr_high = read_byte(static_cast<u16>(zp_addr + 1));
  return ((effective_addr_high << 8) | effective_addr_low) + Y;
}

bool CPU::check_page_cross(u16 addr1, u16 addr2) {
  return (addr1 & 0xFF00) != (addr2 & 0xFF00);
}

} // namespace nes
