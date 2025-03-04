#pragma once
#include <cstdint>

namespace nes {
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i8 = std::int8_t;

class Addressable;
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

  // Arithmetic operations
  ADC_IMM = 0x69,  // ADC Immediate
  ADC_ABS = 0x6D,  // ADC Absolute
  ADC_ABX = 0x7D,  // ADC Absolute X-Indexed
  ADC_ABY = 0x79,  // ADC Absolute Y-Indexed
  ADC_ZPG = 0x65,  // ADC Zero Page
  ADC_ZPX = 0x75,  // ADC Zero Page X-Indexed
  ADC_IZX = 0x61,  // ADC Indirect X (Zero Page Pre-Indexed)
  ADC_IZY = 0x71,  // ADC Indirect Y (Zero Page Post-Indexed)

  SBC_IMM = 0xE9,  // SBC Immediate
  SBC_ABS = 0xED,  // SBC Absolute
  SBC_ABX = 0xFD,  // SBC Absolute X-Indexed
  SBC_ABY = 0xF9,  // SBC Absolute Y-Indexed
  SBC_ZPG = 0xE5,  // SBC Zero Page
  SBC_ZPX = 0xF5,  // SBC Zero Page X-Indexed
  SBC_IZX = 0xE1,  // SBC Indirect X (Zero Page Pre-Indexed)
  SBC_IZY = 0xF1,  // SBC Indirect Y (Zero Page Post-Indexed)

  CMP_IMM = 0xC9,  // CMP Immediate
  CMP_ABS = 0xCD,  // CMP Absolute
  CMP_ABX = 0xDD,  // CMP Absolute X-Indexed
  CMP_ABY = 0xD9,  // CMP Absolute Y-Indexed
  CMP_ZPG = 0xC5,  // CMP Zero Page
  CMP_ZPX = 0xD5,  // CMP Zero Page X-Indexed
  CMP_IZX = 0xC1,  // CMP Indirect X (Zero Page Pre-Indexed)
  CMP_IZY = 0xD1,  // CMP Indirect Y (Zero Page Post-Indexed)

  CPX_IMM = 0xE0,  // CPX Immediate
  CPX_ABS = 0xEC,  // CPX Absolute
  CPX_ZPG = 0xE4,  // CPX Absolute

  CPY_IMM = 0xC0,  // CPY Immediate
  CPY_ABS = 0xCC,  // CPY Absolute
  CPY_ZPG = 0xC4,  // CPY Absolute

  // Logical operations
  // AND
  AND_IMM = 0x29,  // AND Immediate
  AND_ABS = 0x2D,  // AND Absolute
  AND_ABX = 0x3D,  // AND Absolute X-Indexed
  AND_ABY = 0x39,  // AND Absolute Y-Indexed
  AND_ZPG = 0x25,  // AND Zero Page
  AND_ZPX = 0x35,  // AND Zero Page X-Indexed
  AND_IZX = 0x21,  // AND Indirect X (Zero Page Pre-Indexed)
  AND_IZY = 0x31,  // AND Indirect Y (Zero Page Post-Indexed)

  // EOR
  EOR_IMM = 0x49,  // EOR Immediate
  EOR_ABS = 0x4D,  // EOR Absolute
  EOR_ABX = 0x5D,  // EOR Absolute X-Indexed
  EOR_ABY = 0x59,  // EOR Absolute Y-Indexed
  EOR_ZPG = 0x45,  // EOR Zero Page
  EOR_ZPX = 0x55,  // EOR Zero Page X-Indexed
  EOR_IZX = 0x41,  // EOR Indirect X (Zero Page Pre-Indexed)
  EOR_IZY = 0x51,  // EOR Indirect Y (Zero Page Post-Indexed)

  // ORA
  ORA_IMM = 0x09,  // ORA Immediate
  ORA_ABS = 0x0D,  // ORA Absolute
  ORA_ABX = 0x1D,  // ORA Absolute X-Indexed
  ORA_ABY = 0x19,  // ORA Absolute Y-Indexed
  ORA_ZPG = 0x05,  // ORA Zero Page
  ORA_ZPX = 0x15,  // ORA Zero Page X-Indexed
  ORA_IZX = 0x01,  // ORA Indirect X (Zero Page Pre-Indexed)
  ORA_IZY = 0x11,  // ORA Indirect Y (Zero Page Post-Indexed)

  // BIT
  BIT_ABS = 0x2C,  // Test Bits in memory witht he accumulator Absolute
  BIT_ZPG = 0x24,  // Test Bits in memory witht he accumulator Zero Page

  // Increment/Decrement operations
  // INC
  INC_ABS = 0xEE,  // Increment Absolute
  INC_ABX = 0xFE,  // Increment Absolute X-Indexed
  INC_ZPG = 0xE6,  // Increment Absolute Zero Page
  INC_ZPX = 0xF6,  // Increment Absolute Zero Page X-Indexed

  // DEC
  DEC_ABS = 0xCE,  // Decrement Absolute
  DEC_ABX = 0xDE,  // Decrement Absolute X-Indexed
  DEC_ZPG = 0xC6,  // Decrement Absolute Zero Page
  DEC_ZPX = 0xD6,  // Decrement Absolute Zero Page X-Indexed

  // INX, INY
  INX_IMP = 0xE8,  // Increment Index Register X Implied
  INY_IMP = 0xC8,  // Increment Index Register Y Implied

  // DEX, DEY
  DEX_IMP = 0xCA,  // Decrement Index Register X Implied
  DEY_IMP = 0x88,  // Decrement Index Register Y Implied

  // Branching operations
  BCC_REL = 0x90,  // Branch on carry clear Relative
  BCS_REL = 0xB0,  // Branch on carry set Relative
  BEQ_REL = 0xF0,  // Branch on result zero Relative
  BMI_REL = 0x30,  // Branch on result minus Relative
  BNE_REL = 0xD0,  // Branch on result not zero Relative
  BPL_REL = 0x10,  // Branch on result plus Relative
  BVS_REL = 0x70,  // Branch on overflow clear Relative
  BVC_REL = 0x50,  // Branch on overflow set Relative

  // Control-Flow operations
  JMP_ABS = 0x4C,  // Jump Absolute
  JMP_IND = 0x6C,  // Jump Indirect
  BRK_IMP = 0x00,  // Break Implied
  JSR_ABS = 0x20,  // Jump to subroutine Implied
  RTI_IMP = 0x40,  // Return from interrupt
  RTS_IMP = 0x60,  // Return from subroutine

  // Flag operations (implied addressing)
  SEC_IMP = 0x38,  // Set Carry Flag
  SED_IMP = 0xF8,  // Set Decimal Flag
  SEI_IMP = 0x78,  // Set Interrupt Disable Flag
  CLC_IMP = 0x18,  // Clear Carry Flag
  CLD_IMP = 0xD8,  // Clear Decimal Flag
  CLI_IMP = 0x58,  // Clear Interrupt Disable Flag
  CLV_IMP = 0xB8,  // Clear Overflow  Flag

  // NOP
  NOP_IMP = 0xEA,  // No operation
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
