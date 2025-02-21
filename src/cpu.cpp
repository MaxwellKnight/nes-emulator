#include "../include/cpu.h"
#include <stdexcept>

namespace nes {

CPU::CPU() {
  init_instruction_table();
  reset();
}

void CPU::init_instruction_table() {
  instruction_table = {{static_cast<u8>(Opcode::LDA_IM),
                        {&CPU::lda_immediate, 2, "LDA Immediate"}},
                       {static_cast<u8>(Opcode::LDA_ZP),
                        {&CPU::lda_zero_page, 3, "LDA Zero Page"}},
                       {static_cast<u8>(Opcode::LDA_ABS),
                        {&CPU::lda_absolute, 4, "LDA Absolute"}},
                       {static_cast<u8>(Opcode::LDA_XABS),
                        {&CPU::lda_absolute_x, 4, "LDA Absolute,X"}},
                       {static_cast<u8>(Opcode::LDA_YABS),
                        {&CPU::lda_absolute_y, 4, "LDA Absolute,Y"}},
                       {static_cast<u8>(Opcode::LDA_XZP),
                        {&CPU::lda_zero_page_x, 4, "LDA Zero Page,X"}},
                       {static_cast<u8>(Opcode::LDA_XZPI),
                        {&CPU::lda_indirect_x, 6, "LDA (Indirect,X)"}},
                       {static_cast<u8>(Opcode::LDA_YZPI),
                        {&CPU::lda_indirect_y, 5, "LDA (Indirect),Y"}},
                       {static_cast<u8>(Opcode::STA_ZP),
                        {&CPU::sta_zero_page, 3, "STA Zero Page"}},
                       {static_cast<u8>(Opcode::TAX), {&CPU::tax, 2, "TAX"}},
                       {static_cast<u8>(Opcode::TXA), {&CPU::txa, 2, "TXA"}}};
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

void CPU::clock(Memory &memory) {
  if (cycles == 0) {
    u8 opcode = read_byte(memory, PC++);
    set_flag(Flag::UNUSED, true);

    auto it = instruction_table.find(opcode);
    if (it == instruction_table.end()) {
      throw std::runtime_error("Unknown opcode: " + std::to_string(opcode));
    }

    cycles = it->second.cycles;
    (this->*it->second.handler)(memory);
  }
  cycles--;
}

// Instruction Handlers
void CPU::lda_immediate(Memory &memory) {
  A = read_byte(memory, PC++);
  update_zero_and_negative_flags(A);
}

void CPU::lda_zero_page(Memory &memory) {
  u16 addr = addr_zero_page(memory);
  A = read_byte(memory, addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_absolute(Memory &memory) {
  u16 addr = addr_absolute(memory);
  A = read_byte(memory, addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_absolute_x(Memory &memory) {
  u16 base_addr = addr_absolute(memory);
  u16 addr = addr_absolute_x(memory);
  if (check_page_cross(base_addr, addr)) {
    cycles++;
  }
  A = read_byte(memory, addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_absolute_y(Memory &memory) {
  u16 base_addr = addr_absolute(memory);
  u16 addr = addr_absolute_y(memory);
  if (check_page_cross(base_addr, addr)) {
    cycles++;
  }
  A = read_byte(memory, addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_zero_page_x(Memory &memory) {
  u16 addr = addr_zero_page_x(memory);
  A = read_byte(memory, addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_indirect_x(Memory &memory) {
  u16 addr = addr_indirect_x(memory);
  A = read_byte(memory, addr);
  update_zero_and_negative_flags(A);
}

void CPU::lda_indirect_y(Memory &memory) {
  u8 zp_addr = read_byte(memory, PC++);
  u16 addr = addr_indirect_y(memory, zp_addr);
  if (check_page_cross(addr - Y, addr)) {
    cycles++;
  }
  A = read_byte(memory, addr);
  update_zero_and_negative_flags(A);
}

void CPU::sta_zero_page(Memory &memory) {
  u16 addr = addr_zero_page(memory);
  write_byte(memory, addr, A);
}

void CPU::tax(Memory &memory) {
  X = A;
  update_zero_and_negative_flags(X);
}

void CPU::txa(Memory &memory) {
  A = X;
  update_zero_and_negative_flags(A);
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

// Addressing modes
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

u16 CPU::addr_indirect_y(const Memory &memory, u8 zp_addr) {
  u16 effective_addr_low = read_byte(memory, zp_addr);
  u16 effective_addr_high = read_byte(memory, static_cast<u16>(zp_addr + 1));
  return ((effective_addr_high << 8) | effective_addr_low) + Y;
}

bool CPU::check_page_cross(u16 addr1, u16 addr2) {
  return (addr1 & 0xFF00) != (addr2 & 0xFF00);
}

} // namespace nes
