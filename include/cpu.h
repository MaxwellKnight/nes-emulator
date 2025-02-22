#pragma once
#include "bus.h"
#include "memory.h"
#include <string>
#include <unordered_map>

namespace nes {
enum class Opcode : u8 {
  LDA_IM = 0xA9,   // LDA Immediate
  LDA_ABS = 0xAD,  // LDA Absolute
  LDA_XABS = 0xBD, // LDA X-Index Absolute
  LDA_YABS = 0xB9, // LDA Y-Index Absolute
  LDA_ZP = 0xA5,   // LDA Zero Page
  LDA_XZP = 0xB5,  // LDA X-Index Zero Page
  LDA_XZPI = 0xA1, // LDA X-Index Zero Page Indirect
  LDA_YZPI = 0xB1, // LDA Y-Index Zero Page Indirect
  LDX_IM = 0xA2,   // LDX Immediate
  LDX_ABS = 0xAE,  // LDX Absolute
  LDX_YABS = 0xBE, // LDX Y-Index Absolute
  LDX_ZP = 0xA6,   // LDX Zero Page
  LDX_YZP = 0xB6,  // LDX Y-Index Zero Page
  LDY_IM = 0xA0,   // LDY Immediate
  LDY_ABS = 0xAC,  // LDY Absolute
  LDY_XABS = 0xBC, // LDY X-Index Absolute
  LDY_ZP = 0xA4,   // LDY Zero Page
  LDY_XZP = 0xB4,  // LDY X-Index Zero Page
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
  [[nodiscard]] std::shared_ptr<Bus> get_bus() const;
  void set_flag(Flag flag, bool value);

private:
  // Registers
  u8 A{0};      // Accumulator
  u8 X{0};      // X Index Register
  u8 Y{0};      // Y Index Register
  u8 SP{0};     // Stack Pointer
  u16 PC{0};    // Program Counter
  u8 status{0}; // Processor Status Register
  u64 cycles{0};
  Bus &bus;

  using InstructionHandler = void (CPU::*)();

  struct Instruction {
    InstructionHandler handler;
    u8 cycles;
    std::string name;
  };

  std::unordered_map<u8, Instruction> instruction_table;

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

  void sta_zero_page();
  void tax();
  void txa();

  // Internal methods
  void push_stack(u8 value);
  void write_byte(u16 address, u8 value);
  void update_zero_and_negative_flags(u8 value);
  [[nodiscard]] u8 pull_stack();
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
