#pragma once
#include <cstdint>
#include <string>

namespace nes {
class CPU;
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;

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

using InstructionHandler = void (CPU::*)();

struct Instruction {
  InstructionHandler handler;
  u8 cycles;
  std::string name;
};

} // namespace nes
