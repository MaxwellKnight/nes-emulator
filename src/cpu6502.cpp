#include "../include/cpu6502.h"
#include <stdexcept>

namespace cpu6502 {

void CPU::reset() {
  A = 0;
  X = 0;
  Y = 0;
  SP = 0xFF;
  status_byte = 0;
  PC = 0xFFFC;
  total_cycles = 0;
}

void CPU::run(Memory &memory) {
  while (true) {
    try {
      execute(memory);
    } catch (const std::runtime_error &e) {
      break;
    }
  }
}

u8 CPU::get_opcode_cycles(Opcode opcode) const {
  switch (opcode) {
  case Opcode::LDA_IM:
    return 2;
  case Opcode::LDA_ZP:
    return 3;
  case Opcode::STA_ZP:
    return 3;
  case Opcode::TAX:
    return 2;
  case Opcode::TXA:
    return 2;
  default:
    return 0;
  }
}

void CPU::execute(Memory &memory) {
  u8 opcode = read_byte(memory, PC++);

  // Track cycles for the instruction
  total_cycles += get_opcode_cycles(static_cast<Opcode>(opcode));

  switch (static_cast<Opcode>(opcode)) {
  case Opcode::LDA_IM: {
    A = read_byte(memory, PC++);
    update_zero_and_negative_flags(A);
    break;
  }
  case Opcode::LDA_ZP: {
    u16 addr = addr_zero_page(memory);
    A = read_byte(memory, addr);
    update_zero_and_negative_flags(A);
    break;
  }
  case Opcode::STA_ZP: {
    u16 addr = addr_zero_page(memory);
    write_byte(memory, addr, A);
    break;
  }
  case Opcode::TAX: {
    X = A;
    update_zero_and_negative_flags(X);
    break;
  }
  case Opcode::TXA: {
    A = X;
    update_zero_and_negative_flags(A);
    break;
  }
  default:
    throw std::runtime_error("Unknown opcode");
  }
}

void CPU::set_flag(Flag flag, bool value) {
  if (value) {
    status_byte |= static_cast<u8>(flag);
  } else {
    status_byte &= ~static_cast<u8>(flag);
  }
}

bool CPU::get_flag(Flag flag) const {
  return (status_byte & static_cast<u8>(flag)) != 0;
}

void CPU::update_zero_and_negative_flags(u8 value) {
  set_flag(Flag::ZERO, value == 0);
  set_flag(Flag::NEGATIVE, (value & 0x80) != 0);
}

u8 CPU::read_byte(const Memory &memory, u16 address) {
  return memory.read(address);
}

void CPU::write_byte(Memory &memory, u16 address, u8 value) {
  memory.write(address, value);
}

void CPU::push_stack(Memory &memory, u8 value) {
  write_byte(memory, 0x0100 + SP, value);
  SP--;
}

u8 CPU::pull_stack(const Memory &memory) {
  SP++;
  return read_byte(memory, 0x0100 + SP);
}

u16 CPU::addr_zero_page(const Memory &memory) {
  return read_byte(memory, PC++);
}

u16 CPU::addr_zero_page_x(const Memory &memory) {
  u8 zp_addr = read_byte(memory, PC++);
  return static_cast<u16>((zp_addr + X) & 0xFF);
}

u16 CPU::addr_absolute(const Memory &memory) {
  u16 addr_low = read_byte(memory, PC++);
  u16 addr_high = read_byte(memory, PC++);
  return (addr_high << 8) | addr_low;
}

u16 CPU::addr_absolute_x(const Memory &memory) {
  u16 base_addr = addr_absolute(memory);
  return base_addr + X;
}

u16 CPU::addr_absolute_y(const Memory &memory) {
  u16 base_addr = addr_absolute(memory);
  return base_addr + Y;
}

u16 CPU::addr_indirect_x(const Memory &memory) {
  u8 zp_addr = read_byte(memory, PC++);
  zp_addr += X;
  u16 effective_addr_low = read_byte(memory, zp_addr);
  u16 effective_addr_high = read_byte(memory, static_cast<u16>(zp_addr + 1));
  return (effective_addr_high << 8) | effective_addr_low;
}

u16 CPU::addr_indirect_y(const Memory &memory) {
  u8 zp_addr = read_byte(memory, PC++);
  u16 effective_addr_low = read_byte(memory, zp_addr);
  u16 effective_addr_high = read_byte(memory, static_cast<u16>(zp_addr + 1));
  u16 effective_addr = (effective_addr_high << 8) | effective_addr_low;
  return effective_addr + Y;
}

} // namespace cpu6502
