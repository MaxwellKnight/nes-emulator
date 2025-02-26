#pragma once
#include <cstdint>

namespace nes {
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;

class IAddressable;
class CPU;

// Interface for memory-mapped components
class Addressable {
 public:
  virtual ~Addressable() = default;
  virtual u8 read(u16 address) const = 0;
  virtual void write(u16 address, u8 value) = 0;
  virtual bool handles_address(u16 address) const = 0;
};

// Forward declarations for better type clarity
using AddressedOperation = void (CPU::*)(u16 addr);
using ImpliedOperation = void (CPU::*)();
using ModeHandler = u16 (CPU::*)();

// Instruction structure with support for both addressed and implied operations
struct Instruction {
  union {
    AddressedOperation addressed_op;
    ImpliedOperation implied_op;
  };
  ModeHandler mode;
  u8 cycles;
  const char *name;
  bool is_extra_cycle = false;
  bool is_implied = false;  // Flag to indicate if this is an implied operation
};

enum class Opcode : u8 {
  // Load operations
  LDA_IMM = 0xA9,  // LDA Immediate
  LDA_ZPG = 0xA5,  // LDA Zero Page
  LDA_ZPX = 0xB5,  // LDA Zero Page X-Indexed
  LDA_ABS = 0xAD,  // LDA Absolute
  LDA_ABX = 0xBD,  // LDA Absolute X-Indexed
  LDA_ABY = 0xB9,  // LDA Absolute Y-Indexed
  LDA_IZX = 0xA1,  // LDA Indirect X (Zero Page Pre-Indexed)
  LDA_IZY = 0xB1,  // LDA Indirect Y (Zero Page Post-Indexed)

  LDX_IMM = 0xA2,  // LDX Immediate
  LDX_ZPG = 0xA6,  // LDX Zero Page
  LDX_ZPY = 0xB6,  // LDX Zero Page Y-Indexed
  LDX_ABS = 0xAE,  // LDX Absolute
  LDX_ABY = 0xBE,  // LDX Absolute Y-Indexed

  LDY_IMM = 0xA0,  // LDY Immediate
  LDY_ZPG = 0xA4,  // LDY Zero Page
  LDY_ZPX = 0xB4,  // LDY Zero Page X-Indexed
  LDY_ABS = 0xAC,  // LDY Absolute
  LDY_ABX = 0xBC,  // LDY Absolute X-Indexed

  // Store operations
  STA_ZPG = 0x85,  // STA Zero Page
  STA_ZPX = 0x95,  // STA Zero Page X-Indexed
  STA_ABS = 0x8D,  // STA Absolute
  STA_ABX = 0x9D,  // STA Absolute X-Indexed
  STA_ABY = 0x99,  // STA Absolute Y-Indexed
  STA_IZX = 0x81,  // STA Indirect X (Zero Page Pre-Indexed)
  STA_IZY = 0x91,  // STA Indirect Y (Zero Page Post-Indexed)

  STX_ZPG = 0x86,  // STX Zero Page
  STX_ZPY = 0x96,  // STX Zero Page Y-Indexed
  STX_ABS = 0x8E,  // STX Absolute

  STY_ZPG = 0x84,  // STY Zero Page
  STY_ZPX = 0x94,  // STY Zero Page X-Indexed
  STY_ABS = 0x8C,  // STY Absolute

  // Register transfers (implied addressing)
  TAX_IMP = 0xAA,  // Transfer A to X
  TXA_IMP = 0x8A,  // Transfer X to A
  TAY_IMP = 0xA8,  // Transfer A to Y
  TYA_IMP = 0x98,  // Transfer Y to A
  TSX_IMP = 0xBA,  // Transfer Stack Pointer to X
  TXS_IMP = 0x9A,  // Transfer X to Stack Pointer

  // Stack operations (implied addressing)
  PHA_IMP = 0x48,  // Push A to Stack
  PHP_IMP = 0x08,  // Push Processor Status to Stack
  PLA_IMP = 0x68,  // Pull A from Stack
  PLP_IMP = 0x28,  // Pull Processor Status from Stack

  // Shift operations
  ASL_ACC = 0x0A,  // Arithmetic Shift Left Accumulator
  ASL_ZPG = 0x06,  // Arithmetic Shift Left Zero Page
  ASL_ZPX = 0x16,  // Arithmetic Shift Left Zero Page X-Indexed
  ASL_ABS = 0x0E,  // Arithmetic Shift Left Absolute
  ASL_ABX = 0x1E,  // Arithmetic Shift Left Absolute X-Indexed

  LSR_ACC = 0x4A,  // Logical Shift Right Accumulator
  LSR_ZPG = 0x46,  // Logical Shift Right Zero Page
  LSR_ZPX = 0x56,  // Logical Shift Right Zero Page X-Indexed
  LSR_ABS = 0x4E,  // Logical Shift Right Absolute
  LSR_ABX = 0x5E,  // Logical Shift Right Absolute X-Indexed

  ROL_ACC = 0x2A,  // Rotate Left Accumulator
  ROL_ZPG = 0x26,  // Rotate Left Zero Page
  ROL_ZPX = 0x36,  // Rotate Left Zero Page X-Indexed
  ROL_ABS = 0x2E,  // Rotate Left Absolute
  ROL_ABX = 0x3E,  // Rotate Left Absolute X-Indexed

  ROR_ACC = 0x6A,  // Rotate Right Accumulator
  ROR_ZPG = 0x66,  // Rotate Right Zero Page
  ROR_ZPX = 0x76,  // Rotate Right Zero Page X-Indexed
  ROR_ABS = 0x6E,  // Rotate Right Absolute
  ROR_ABX = 0x7E,  // Rotate Right Absolute X-Indexed

  // Flag operations (implied addressing)
  CLC_IMP = 0x18,  // Clear Carry Flag
  SEC_IMP = 0x38,  // Set Carry Flag
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

}  // namespace nes
