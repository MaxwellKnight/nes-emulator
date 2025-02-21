#pragma once
#include "memory.h"
#include <cstdint>
#include <functional>
#include <string>
#include <unordered_map>

namespace nes {

class CPU {
public:
  enum class Opcode : u8 {
    LDA_IM = 0xA9,   // LDA Immediate
    LDA_ABS = 0xAD,  // LDA Absolute
    LDA_XABS = 0xBD, // LDA X-Index Absolute
    LDA_YABS = 0xB9, // LDA Y-Index Absolute
    LDA_ZP = 0xA5,   // LDA Zero Page
    LDA_XZP = 0xB5,  // LDA X-Index Zero Page
    LDA_XZPI = 0xA1, // LDA X-Index Zero Page Indirect
    LDA_YZPI = 0xB1, // LDA Y-Index Zero Page Indirect
    STA_ZP = 0x85,   // STA Zero Page
    TAX = 0xAA,      // Transfer A to X
    TXA = 0x8A       // Transfer X to A
  };

  enum class Flag : u8 {
    CARRY = 0x01,
    ZERO = 0x02,
    INTERRUPT_DISABLE = 0x04,
    DECIMAL = 0x08,
    BREAK = 0x10,
    UNUSED = 0x20,
    OVERFLOW_ = 0x40,
    NEGATIVE = 0x80
  };

  CPU();
  void reset();
  void clock(Memory &memory);
  [[nodiscard]] u8 get_accumulator() const;
  [[nodiscard]] u8 get_x() const;
  [[nodiscard]] u8 get_y() const;
  [[nodiscard]] u16 get_pc() const;
  [[nodiscard]] u8 get_sp() const;
  [[nodiscard]] u8 get_status() const;
  [[nodiscard]] u8 get_remaining_cycles() const;
  void set_flag(Flag flag, bool value);
  [[nodiscard]] bool get_flag(Flag flag) const;
  [[nodiscard]] u64 get_total_cycles() const { return cycles; }

private:
  using InstructionHandler = void (CPU::*)(Memory &);

  struct InstructionInfo {
    InstructionHandler handler;
    u8 cycles;
    std::string name;
  };

  std::unordered_map<u8, InstructionInfo> instruction_table;

  void init_instruction_table();

  // Instruction handlers
  void lda_immediate(Memory &memory);
  void lda_zero_page(Memory &memory);
  void lda_absolute(Memory &memory);
  void lda_absolute_x(Memory &memory);
  void lda_absolute_y(Memory &memory);
  void lda_zero_page_x(Memory &memory);
  void lda_indirect_x(Memory &memory);
  void lda_indirect_y(Memory &memory);
  void sta_zero_page(Memory &memory);
  void tax(Memory &memory);
  void txa(Memory &memory);

  // Registers
  u8 A{0};      // Accumulator
  u8 X{0};      // X Index Register
  u8 Y{0};      // Y Index Register
  u8 SP{0};     // Stack Pointer
  u16 PC{0};    // Program Counter
  u8 status{0}; // Processor Status Register
  u64 cycles{0};

  // Internal methods
  void update_zero_and_negative_flags(u8 value);
  [[nodiscard]] u8 read_byte(const Memory &memory, u16 address);
  void write_byte(Memory &memory, u16 address, u8 value);
  void push_stack(Memory &memory, u8 value);
  [[nodiscard]] bool check_page_cross(u16 addr1, u16 addr2);
  [[nodiscard]] u8 pull_stack(const Memory &memory);

  // Addressing modes
  [[nodiscard]] u16 addr_zero_page(const Memory &memory);
  [[nodiscard]] u16 addr_zero_page_x(const Memory &memory);
  [[nodiscard]] u16 addr_absolute(const Memory &memory);
  [[nodiscard]] u16 addr_absolute_x(const Memory &memory);
  [[nodiscard]] u16 addr_absolute_y(const Memory &memory);
  [[nodiscard]] u16 addr_indirect_x(const Memory &memory);
  [[nodiscard]] u16 addr_indirect_y(const Memory &memory, u8 zp_addr);
};

} // namespace nes
