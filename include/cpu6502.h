#pragma once
#include "memory.h"
#include <cstdint>

namespace nes {
class CPU {
public:
  enum class Opcode : u8 {
    LDA_IM = 0xA9, // LDA Immediate
    LDA_ZP = 0xA5, // LDA Zero Page
    STA_ZP = 0x85, // STA Zero Page
    TAX = 0xAA,    // Transfer A to X
    TXA = 0x8A     // Transfer X to A
  };

  enum class Flag : u8 {
    CARRY = 0x01,
    ZERO = 0x02,
    INTERRUPT_DISABLE = 0x04,
    DECIMAL = 0x08,
    BREAK = 0x10,
    UNUSED = 0x20,
    _OVERFLOW = 0x40,
    NEGATIVE = 0x80
  };

  CPU() { reset(); }
  void reset();
  void run(Memory &memory);
  void execute(Memory &memory);

  [[nodiscard]] u8 get_accumulator() const { return A; }
  [[nodiscard]] u8 get_x() const { return X; }
  [[nodiscard]] u8 get_y() const { return Y; }
  [[nodiscard]] u16 get_pc() const { return PC; }
  [[nodiscard]] u8 get_sp() const { return SP; }
  [[nodiscard]] u8 get_status() const { return status_byte; }

  void set_flag(Flag flag, bool value);
  [[nodiscard]] bool get_flag(Flag flag) const;

  [[nodiscard]] u64 get_total_cycles() const { return total_cycles; }

private:
  // Registers
  u8 A = 0;           // Accumulator
  u8 X = 0;           // X Index Register
  u8 Y = 0;           // Y Index Register
  u8 SP = 0;          // Stack Pointer
  u16 PC = 0;         // Program Counter
  u8 status_byte = 0; // Processor Status Register

  u64 total_cycles = 0;

  // Internal methods
  void update_zero_and_negative_flags(u8 value);
  [[nodiscard]] u8 read_byte(const Memory &memory, u16 address);
  void write_byte(Memory &memory, u16 address, u8 value);
  void push_stack(Memory &memory, u8 value);
  [[nodiscard]] u8 pull_stack(const Memory &memory);

  // Addressing modes
  [[nodiscard]] u16 addr_zero_page(const Memory &memory);
  [[nodiscard]] u16 addr_zero_page_x(const Memory &memory);
  [[nodiscard]] u16 addr_absolute(const Memory &memory);
  [[nodiscard]] u16 addr_absolute_x(const Memory &memory);
  [[nodiscard]] u16 addr_absolute_y(const Memory &memory);
  [[nodiscard]] u16 addr_indirect_x(const Memory &memory);
  [[nodiscard]] u16 addr_indirect_y(const Memory &memory);

  // Cycle cost for each opcode
  [[nodiscard]] u8 get_opcode_cycles(Opcode opcode) const;
};
} // namespace nes
