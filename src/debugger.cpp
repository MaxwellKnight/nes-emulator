#include "debugger.h"
#include <iomanip>
#include <iostream>
#include <sstream>
#include "types.h"

namespace nes {
// Global instance for WASM exports
static Debugger* g_debugger = nullptr;

void print_disassembled_instruction(const DisassembledInstruction& instruction);
Debugger::Debugger(CPU& cpu, Bus& bus)
  : _cpu(cpu)
  , _bus(bus)
  , _running(false)
  , _instruction_count(0)
  , _cycle_count(0) {
  g_debugger = this;

  init_addressing_mode_table();
}

void Debugger::init_addressing_mode_table() {
  _addressing_mode_table.fill("IMP");

  // Load/Store operations
  // LDA
  _addressing_mode_table[(u8)Opcode::LDA_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::LDA_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::LDA_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::LDA_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::LDA_ABX] = "ABX";
  _addressing_mode_table[(u8)Opcode::LDA_ABY] = "ABY";
  _addressing_mode_table[(u8)Opcode::LDA_IZX] = "IZX";
  _addressing_mode_table[(u8)Opcode::LDA_IZY] = "IZY";

  // LDX
  _addressing_mode_table[(u8)Opcode::LDX_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::LDX_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::LDX_ZPY] = "ZPY";
  _addressing_mode_table[(u8)Opcode::LDX_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::LDX_ABY] = "ABY";

  // LDY
  _addressing_mode_table[(u8)Opcode::LDY_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::LDY_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::LDY_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::LDY_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::LDY_ABX] = "ABX";

  // STA
  _addressing_mode_table[(u8)Opcode::STA_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::STA_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::STA_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::STA_ABX] = "ABX";
  _addressing_mode_table[(u8)Opcode::STA_ABY] = "ABY";
  _addressing_mode_table[(u8)Opcode::STA_IZX] = "IZX";
  _addressing_mode_table[(u8)Opcode::STA_IZY] = "IZY";

  // STX
  _addressing_mode_table[(u8)Opcode::STX_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::STX_ZPY] = "ZPY";
  _addressing_mode_table[(u8)Opcode::STX_ABS] = "ABS";

  // STY
  _addressing_mode_table[(u8)Opcode::STY_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::STY_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::STY_ABS] = "ABS";

  // Arithmetic operations
  // ADC
  _addressing_mode_table[(u8)Opcode::ADC_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::ADC_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::ADC_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::ADC_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::ADC_ABX] = "ABX";
  _addressing_mode_table[(u8)Opcode::ADC_ABY] = "ABY";
  _addressing_mode_table[(u8)Opcode::ADC_IZX] = "IZX";
  _addressing_mode_table[(u8)Opcode::ADC_IZY] = "IZY";

  // SBC
  _addressing_mode_table[(u8)Opcode::SBC_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::SBC_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::SBC_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::SBC_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::SBC_ABX] = "ABX";
  _addressing_mode_table[(u8)Opcode::SBC_ABY] = "ABY";
  _addressing_mode_table[(u8)Opcode::SBC_IZX] = "IZX";
  _addressing_mode_table[(u8)Opcode::SBC_IZY] = "IZY";

  // CMP
  _addressing_mode_table[(u8)Opcode::CMP_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::CMP_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::CMP_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::CMP_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::CMP_ABX] = "ABX";
  _addressing_mode_table[(u8)Opcode::CMP_ABY] = "ABY";
  _addressing_mode_table[(u8)Opcode::CMP_IZX] = "IZX";
  _addressing_mode_table[(u8)Opcode::CMP_IZY] = "IZY";

  // CPX
  _addressing_mode_table[(u8)Opcode::CPX_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::CPX_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::CPX_ABS] = "ABS";

  // CPY
  _addressing_mode_table[(u8)Opcode::CPY_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::CPY_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::CPY_ABS] = "ABS";

  // Logical operations
  // AND
  _addressing_mode_table[(u8)Opcode::AND_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::AND_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::AND_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::AND_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::AND_ABX] = "ABX";
  _addressing_mode_table[(u8)Opcode::AND_ABY] = "ABY";
  _addressing_mode_table[(u8)Opcode::AND_IZX] = "IZX";
  _addressing_mode_table[(u8)Opcode::AND_IZY] = "IZY";

  // ORA
  _addressing_mode_table[(u8)Opcode::ORA_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::ORA_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::ORA_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::ORA_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::ORA_ABX] = "ABX";
  _addressing_mode_table[(u8)Opcode::ORA_ABY] = "ABY";
  _addressing_mode_table[(u8)Opcode::ORA_IZX] = "IZX";
  _addressing_mode_table[(u8)Opcode::ORA_IZY] = "IZY";

  // EOR
  _addressing_mode_table[(u8)Opcode::EOR_IMM] = "IMM";
  _addressing_mode_table[(u8)Opcode::EOR_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::EOR_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::EOR_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::EOR_ABX] = "ABX";
  _addressing_mode_table[(u8)Opcode::EOR_ABY] = "ABY";
  _addressing_mode_table[(u8)Opcode::EOR_IZX] = "IZX";
  _addressing_mode_table[(u8)Opcode::EOR_IZY] = "IZY";

  // BIT
  _addressing_mode_table[(u8)Opcode::BIT_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::BIT_ABS] = "ABS";

  // Shifts and rotates
  // ASL
  _addressing_mode_table[(u8)Opcode::ASL_ACC] = "ACC";
  _addressing_mode_table[(u8)Opcode::ASL_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::ASL_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::ASL_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::ASL_ABX] = "ABX";

  // LSR
  _addressing_mode_table[(u8)Opcode::LSR_ACC] = "ACC";
  _addressing_mode_table[(u8)Opcode::LSR_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::LSR_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::LSR_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::LSR_ABX] = "ABX";

  // ROL
  _addressing_mode_table[(u8)Opcode::ROL_ACC] = "ACC";
  _addressing_mode_table[(u8)Opcode::ROL_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::ROL_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::ROL_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::ROL_ABX] = "ABX";

  // ROR
  _addressing_mode_table[(u8)Opcode::ROR_ACC] = "ACC";
  _addressing_mode_table[(u8)Opcode::ROR_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::ROR_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::ROR_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::ROR_ABX] = "ABX";

  // Increments and decrement
  // INC
  _addressing_mode_table[(u8)Opcode::INC_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::INC_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::INC_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::INC_ABX] = "ABX";

  // DEC
  _addressing_mode_table[(u8)Opcode::DEC_ZPG] = "ZPG";
  _addressing_mode_table[(u8)Opcode::DEC_ZPX] = "ZPX";
  _addressing_mode_table[(u8)Opcode::DEC_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::DEC_ABX] = "ABX";

  // INX, DEX, INY, DEY are iplied

  // Control flow
  // JMP
  _addressing_mode_table[(u8)Opcode::JMP_ABS] = "ABS";
  _addressing_mode_table[(u8)Opcode::JMP_IND] = "IND";

  // JSR
  _addressing_mode_table[(u8)Opcode::JSR_ABS] = "ABS";

  // Branches
  _addressing_mode_table[(u8)Opcode::BCC_REL] = "REL";
  _addressing_mode_table[(u8)Opcode::BCS_REL] = "REL";
  _addressing_mode_table[(u8)Opcode::BEQ_REL] = "REL";
  _addressing_mode_table[(u8)Opcode::BMI_REL] = "REL";
  _addressing_mode_table[(u8)Opcode::BNE_REL] = "REL";
  _addressing_mode_table[(u8)Opcode::BPL_REL] = "REL";
  _addressing_mode_table[(u8)Opcode::BVC_REL] = "REL";
  _addressing_mode_table[(u8)Opcode::BVS_REL] = "REL";
}

// Execute one instruction
void Debugger::step() {
  // Get current PC before execution
  u16 current_pc = _cpu.get_pc();

  // Check if current instruction is BRK (0x00)
  u8 opcode = _bus.read(current_pc);

  do {
    _cpu.clock();
    _cycle_count++;
  } while (_cpu.get_remaining_cycles() > 0);

  _instruction_count++;

  // Stop if we executed a BRK instruction
  if (opcode == 0x00) {
    stop();

// Notify JavaScript
#ifdef __EMSCRIPTEN__
    EM_ASM({ window.dispatchEvent(new CustomEvent('nes-brk-encountered')); });
#endif
  }

  check_breakpoints();
}

void Debugger::run() { _running = true; }

void Debugger::stop() { _running = false; }

void Debugger::reset() {
  _cpu.reset();
  _instruction_count = 0;
  _cycle_count = 0;
  _running = false;
}

bool Debugger::is_running() const { return _running; }

// Breakpoint methods
void Debugger::add_breakpoint(u16 address) { _breakpoints[address] = true; }
void Debugger::remove_breakpoint(u16 address) { _breakpoints.erase(address); }
void Debugger::clear_breakpoints() { _breakpoints.clear(); }
bool Debugger::has_breakpoint(u16 address) const { return _breakpoints.find(address) != _breakpoints.end(); }

std::vector<u16> Debugger::get_breakpoints() const {
  std::vector<u16> breakpoints;
  for (auto& bp : _breakpoints) {
    breakpoints.push_back(bp.first);
  }
  return breakpoints;
}

// State inspection methods
u8 Debugger::get_register_a() const { return _cpu.get_accumulator(); }
u8 Debugger::get_register_x() const { return _cpu.get_x(); }
u8 Debugger::get_register_y() const { return _cpu.get_y(); }
u8 Debugger::get_register_sp() const { return _cpu.get_sp(); }
u16 Debugger::get_register_pc() const { return _cpu.get_pc(); }
u8 Debugger::get_register_status() const { return _cpu.get_status(); }
u8 Debugger::get_status_flag(Flag flag) const { return _cpu.get_flag(flag) ? 1 : 0; }
void Debugger::set_pc(u16 address) { _cpu.set_pc(address); }

// Memory access methods
u8 Debugger::read_memory(u16 address) const { return _bus.read(address); }
void Debugger::write_memory(u16 address, u8 value) { _bus.write(address, value); }

std::vector<u8> Debugger::read_memory_range(u16 start, u16 end) const {
  std::vector<u8> memory;
  for (u16 addr = start; addr <= end; addr++) {
    memory.push_back(read_memory(addr));
  }
  return memory;
}

std::vector<u8> Debugger::get_stack() const {
  u8 sp = get_register_sp();
  return read_memory_range(0x0100 + sp + 1, 0x01FF);  // Stack is from 0x0100 to 0x01FF
}

u64 Debugger::get_instruction_count() const { return _instruction_count; }
u64 Debugger::get_cycle_count() const { return _cycle_count; }

// Get the number of bytes for a specific opcode
u8 Debugger::get_instruction_bytes(u8 opcode) const {
  std::string addr_mode = _addressing_mode_table[opcode];

  if (addr_mode == "IMP" || addr_mode == "ACC") {
    return 1;  // Just the opcode
  } else if (addr_mode == "IMM" || addr_mode == "ZPG" || addr_mode == "ZPX" || addr_mode == "ZPY" || addr_mode == "IZX" ||
             addr_mode == "IZY" || addr_mode == "REL") {
    return 2;  // Opcode + 1 byte operand
  } else if (addr_mode == "ABS" || addr_mode == "ABX" || addr_mode == "ABY" || addr_mode == "IND") {
    return 3;  // Opcode + 2 byte operand
  }

  // Default/unknown
  return 1;
}

DisassembledInstruction Debugger::disassemble_instruction(u16 address) const {
  DisassembledInstruction result;
  result.address = address;

  u8 opcode = read_memory(address);
  result.opcode = opcode;

  const auto& instruction = _cpu.get_instruction((Opcode)opcode);
  const char* name = instruction.name;
  result.mnemonic = (name != nullptr && name[0] != '\0') ? name : "???";
  result.cycles = instruction.cycles;

  std::string addr_mode = _addressing_mode_table[opcode];
  u8 bytes = get_instruction_bytes(opcode);
  result.bytes = bytes;

  u16 operand = 0;
  if (bytes >= 2) {
    operand = read_memory(address + 1);
    if (bytes == 3) {
      operand |= (static_cast<u16>(read_memory(address + 2)) << 8);
    }
  }
  result.operand = operand;
  result.formatted = format_instruction(opcode, operand, bytes, address);

  if (result.formatted.empty()) {
    std::stringstream ss;
    ss << result.mnemonic;

    if (bytes > 1) {
      ss << " ";
      if (bytes == 2) {
        ss << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (operand & 0xFF);
      } else {
        ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << operand;
      }
    }

    result.formatted = ss.str();
  }

  return result;
}

std::vector<DisassembledInstruction> Debugger::disassemble_range(u16 start, u16 end) const {
  std::vector<DisassembledInstruction> instructions;
  u16 addr = start;

  while (addr <= end) {
    DisassembledInstruction instr = disassemble_instruction(addr);
    instructions.push_back(instr);
    addr += instr.bytes;

    // Safety check to prevent infinite loops with invalid opcodes
    if (instr.bytes == 0) {
      break;
    }
  }

  return instructions;
}

std::vector<DisassembledInstruction> Debugger::disassemble_around_pc(int instructions_before, int instructions_after) const {
  u16 pc = get_register_pc();
  std::unordered_map<u16, bool> valid_starts;

  // Try to find valid instruction starts before PC
  // We'll scan a reasonable range before PC (up to 3*instructions_before bytes)
  int max_scan_distance = instructions_before * 3;
  u16 scan_start = (pc > max_scan_distance) ? (pc - max_scan_distance) : 0;

  for (u16 addr = scan_start; addr < pc; addr++) {
    valid_starts[addr] = true;
  }

  // Invalidate addresses that would be in the middle of other instructions
  for (u16 addr = scan_start; addr < pc; addr++) {
    if (valid_starts[addr]) {
      u8 opcode = read_memory(addr);

      // Skip invalid opcodes by treating them as 1-byte
      const auto& instruction = _cpu.get_instruction((Opcode)opcode);
      if (instruction.name == nullptr || instruction.name[0] == '\0' || instruction.cycles == 0) {
        continue;  // Skip to next address
      }

      u8 instr_bytes = get_instruction_bytes(opcode);

      // Mark addresses inside this instruction as invalid starts
      for (u8 i = 1; i < instr_bytes && (addr + i) < pc; i++) {
        valid_starts[addr + i] = false;
      }
    }
  }

  std::vector<u16> before_addresses;
  u16 current = pc;
  while (before_addresses.size() < instructions_before && current > scan_start) {
    bool found = false;

    for (u16 addr = current - 3; addr < current; addr++) {
      if (addr >= scan_start && valid_starts[addr]) {
        u8 opcode = read_memory(addr);

        const auto& instruction = _cpu.get_instruction((Opcode)opcode);
        if (instruction.name == nullptr || instruction.name[0] == '\0' || instruction.cycles == 0) {
          continue;  // Skip invalid opcodes
        }

        u8 instr_bytes = get_instruction_bytes(opcode);

        if (addr + instr_bytes == current) {
          before_addresses.insert(before_addresses.begin(), addr);
          current = addr;
          found = true;
          break;
        }
      }
    }

    if (!found) {
      // If we can't find a valid instruction ending at current,
      // just step back by one byte and try again
      current--;
    }
  }

  std::vector<DisassembledInstruction> result;
  for (u16 addr : before_addresses) {
    result.push_back(disassemble_instruction(addr));
  }
  result.push_back(disassemble_instruction(pc));

  u16 next_addr = pc;
  DisassembledInstruction pc_instr = disassemble_instruction(pc);
  next_addr += pc_instr.bytes;

  for (int i = 0; i < instructions_after && next_addr < 0xFFFF; i++) {
    DisassembledInstruction instr = disassemble_instruction(next_addr);
    result.push_back(instr);

    // Prevent infinite loops with invalid opcodes
    if (instr.bytes == 0) {
      next_addr++;  // Treat as 1-byte instruction
    } else {
      next_addr += instr.bytes;
    }
  }

  return result;
}

void Debugger::check_breakpoints() {
  u16 pc = get_register_pc();
  if (has_breakpoint(pc)) {
    stop();
    // Might want to emit an event or callback to JS here
  }
}

std::string Debugger::format_instruction(u8 opcode, u16 operand, u8 bytes, u16 instruction_addr) const {
  std::stringstream ss;
  const auto& instruction = _cpu.get_instruction((Opcode)opcode);
  std::string mnemonic = (instruction.name != nullptr && instruction.name[0] != '\0') ? instruction.name : "???";

  // Start with the mnemonic - NO SPACE for immediate mode
  if (opcode == 0xA2 || opcode == 0xA9) {
    ss << mnemonic;
  } else {
    ss << mnemonic << " ";
  }

  // Force immediate addressing mode for specific opcodes
  std::string addr_mode;
  if (opcode == 0xA2) {  // LDX #imm
    addr_mode = "IMM";
  } else if (opcode == 0xA9) {  // LDA #imm
    addr_mode = "IMM";
  } else {
    addr_mode = _addressing_mode_table[opcode];
  }

  // For modes other than implied and immediate opcodes that we've already handled,
  // add a space before the operand
  if (addr_mode != "IMP" && opcode != 0xA2 && opcode != 0xA9) {
    ss << " ";
  }

  if (addr_mode == "IMM") {
    ss << "#$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand & 0xFF);
  } else if (addr_mode == "ZPG") {
    ss << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand & 0xFF);
  } else if (addr_mode == "ZPX") {
    ss << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand & 0xFF) << ",X";
  } else if (addr_mode == "ZPY") {
    ss << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand & 0xFF) << ",Y";
  } else if (addr_mode == "ABS") {
    ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(operand);
  } else if (addr_mode == "ABX") {
    ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(operand) << ",X";
  } else if (addr_mode == "ABY") {
    ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(operand) << ",Y";
  } else if (addr_mode == "IND") {
    ss << "($" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(operand) << ")";
  } else if (addr_mode == "IZX") {
    ss << "($" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand & 0xFF) << ",X)";
  } else if (addr_mode == "IZY") {
    ss << "($" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand & 0xFF) << "),Y";
  } else if (addr_mode == "REL") {
    // Calculate relative address for branch instructions
    int8_t offset = static_cast<int8_t>(operand & 0xFF);
    u16 target = instruction_addr + bytes + offset;
    ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(target);
  } else if (addr_mode == "ACC") {
    ss << "A";
  }

  return ss.str();
}

std::string Debugger::address_mode_string(u8 opcode) const { return _addressing_mode_table[opcode]; }

void print_disassembled_instruction(const DisassembledInstruction& instruction) {
  std::cout << "Address:   0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << instruction.address << std::dec
            << "\n";
  std::cout << "Opcode:    0x" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(instruction.opcode)
            << std::dec << "\n";
  std::cout << "Mnemonic:  " << instruction.mnemonic << "\n";
  std::cout << "Operand:   0x" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << instruction.operand << std::dec
            << "\n";
  std::cout << "Formatted: " << instruction.formatted << "\n";
  std::cout << "Bytes:     " << static_cast<int>(instruction.bytes) << "\n";
  std::cout << "Cycles:    " << static_cast<int>(instruction.cycles) << "\n";
  std::cout << "------------------------------" << std::endl;
}

#ifdef __EMSCRIPTEN__

// WASM exported functions
EMSCRIPTEN_EXPORT void debugger_step() {
  if (g_debugger) {
    g_debugger->step();
  }
}

EMSCRIPTEN_EXPORT void debugger_run() {
  if (g_debugger) {
    g_debugger->run();
  }
}

EMSCRIPTEN_EXPORT void debugger_stop() {
  if (g_debugger) {
    g_debugger->stop();
  }
}

EMSCRIPTEN_EXPORT void debugger_reset() {
  if (g_debugger) {
    g_debugger->reset();
  }
}

EMSCRIPTEN_EXPORT int debugger_is_running() {
  if (g_debugger) {
    return g_debugger->is_running() ? 1 : 0;
  }
  return 0;
}

EMSCRIPTEN_EXPORT void debugger_add_breakpoint(u16 address) {
  if (g_debugger) {
    g_debugger->add_breakpoint(address);
  }
}

EMSCRIPTEN_EXPORT void debugger_remove_breakpoint(u16 address) {
  if (g_debugger) {
    g_debugger->remove_breakpoint(address);
  }
}

EMSCRIPTEN_EXPORT void debugger_clear_breakpoints() {
  if (g_debugger) {
    g_debugger->clear_breakpoints();
  }
}

EMSCRIPTEN_EXPORT u8 debugger_get_register_a() {
  if (g_debugger) {
    return g_debugger->get_register_a();
  }
  return 0;
}

EMSCRIPTEN_EXPORT u8 debugger_get_register_x() {
  if (g_debugger) {
    return g_debugger->get_register_x();
  }
  return 0;
}

EMSCRIPTEN_EXPORT u8 debugger_get_register_y() {
  if (g_debugger) {
    return g_debugger->get_register_y();
  }
  return 0;
}

EMSCRIPTEN_EXPORT u8 debugger_get_register_sp() {
  if (g_debugger) {
    return g_debugger->get_register_sp();
  }
  return 0;
}

EMSCRIPTEN_EXPORT u16 debugger_get_register_pc() {
  if (g_debugger) {
    return g_debugger->get_register_pc();
  }
  return 0;
}

EMSCRIPTEN_EXPORT u8 debugger_get_register_status() {
  if (g_debugger) {
    return g_debugger->get_register_status();
  }
  return 0;
}

EMSCRIPTEN_EXPORT u8 debugger_get_status_flag(int flag) {
  if (g_debugger) {
    return g_debugger->get_status_flag(static_cast<Flag>(flag));
  }
  return 0;
}

EMSCRIPTEN_EXPORT u8 debugger_read_memory(u16 address) {
  if (g_debugger) {
    return g_debugger->read_memory(address);
  }
  return 0;
}

EMSCRIPTEN_EXPORT void debugger_write_memory(u16 address, u8 value) {
  if (g_debugger) {
    g_debugger->write_memory(address, value);
  }
}

EMSCRIPTEN_EXPORT u64 debugger_get_instruction_count() {
  if (g_debugger) {
    return g_debugger->get_instruction_count();
  }
  return 0;
}

EMSCRIPTEN_EXPORT u64 debugger_get_cycle_count() {
  if (g_debugger) {
    return g_debugger->get_cycle_count();
  }
  return 0;
}

EMSCRIPTEN_EXPORT void debugger_set_pc(u16 address) {
  if (g_debugger) {
    g_debugger->set_pc(address);
  }
}

EMSCRIPTEN_EXPORT int debugger_is_at_brk() {
  if (g_debugger) {
    u16 pc = g_debugger->get_register_pc();
    u8 opcode = g_debugger->read_memory(pc);
    return (opcode == 0x00) ? 1 : 0;
  }
  return 0;
}

EMSCRIPTEN_EXPORT char* debugger_disassemble_around_pc(int before, int after) {
  static std::string result;
  result.clear();

  if (!g_debugger) {
    std::cerr << "Error: g_debugger is null!\n";
    return nullptr;
  }

  u16 pc = g_debugger->get_register_pc();
  auto instructions = g_debugger->disassemble_around_pc(before, after);
  std::stringstream ss;

  // Format: address|opcode|mnemonic|operand|formatted|bytes|cycles#address|...
  for (const auto& instr : instructions) {
    // Ensure all values are explicitly formatted as decimal
    ss << std::dec << instr.address << "|" << std::dec << static_cast<int>(instr.opcode) << "|"
       << (instr.mnemonic.empty() ? ".byte" : instr.mnemonic) << "|" << std::dec << static_cast<int>(instr.operand) << "|";

    // Ensure formatted string is never empty
    if (instr.formatted.empty()) {
      // Default formatting
      ss << (instr.mnemonic.empty() ? ".byte" : instr.mnemonic);

      // Add operand if applicable
      if (instr.bytes > 1) {
        ss << " ";
        if (instr.bytes == 2) {
          ss << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(instr.operand & 0xFF);
        } else {
          ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(instr.operand);
        }
      }
    } else {
      ss << instr.formatted;
    }

    // Make sure bytes and cycles are always valid numbers
    int bytes = (instr.bytes > 0) ? static_cast<int>(instr.bytes) : 1;
    int cycles = (instr.cycles > 0) ? static_cast<int>(instr.cycles) : 1;

    ss << "|" << std::dec << bytes << "|" << std::dec << cycles << "#";
  }

  result = ss.str();
  if (!result.empty()) {
    result.pop_back();  // Remove the last '#'
  }

  return const_cast<char*>(result.c_str());
}

EMSCRIPTEN_EXPORT char* debugger_disassemble_range(u16 start, u16 end) {
  static std::string result;
  result.clear();

  if (!g_debugger) {
    return nullptr;
  }

  auto instructions = g_debugger->disassemble_range(start, end);
  std::stringstream ss;

  // Format: address|opcode|mnemonic|operand|formatted|bytes|cycles#address|...
  for (const auto& instr : instructions) {
    // Ensure all values are explicitly formatted as decimal
    ss << std::dec << instr.address << "|" << std::dec << static_cast<int>(instr.opcode) << "|"
       << (instr.mnemonic.empty() ? ".byte" : instr.mnemonic) << "|" << std::dec << static_cast<int>(instr.operand) << "|";

    if (instr.formatted.empty()) {
      // Default formatting
      ss << (instr.mnemonic.empty() ? ".byte" : instr.mnemonic);

      // Add operand if applicable
      if (instr.bytes > 1) {
        ss << " ";
        if (instr.bytes == 2) {
          ss << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(instr.operand & 0xFF);
        } else {
          ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(instr.operand);
        }
      }
    } else {
      ss << instr.formatted;
    }

    int bytes = (instr.bytes > 0) ? static_cast<int>(instr.bytes) : 1;
    int cycles = (instr.cycles > 0) ? static_cast<int>(instr.cycles) : 1;

    ss << "|" << std::dec << bytes << "|" << std::dec << cycles << "#";
  }

  result = ss.str();
  if (!result.empty()) {
    result.pop_back();  // Remove the last '#'
  }

  return const_cast<char*>(result.c_str());
}

EMSCRIPTEN_EXPORT void debugger_print_state() {
  if (!g_debugger) {
    std::cerr << "Error: g_debugger is null!\n";
    return;
  }

  u16 pc = g_debugger->get_register_pc();
  u8 a = g_debugger->get_register_a();
  u8 x = g_debugger->get_register_x();
  u8 y = g_debugger->get_register_y();
  u8 sp = g_debugger->get_register_sp();
  u8 status = g_debugger->get_register_status();

  printf("CPU State:\n");
  printf("  PC = $%04X\n", pc);
  printf("  A  = $%02X\n", a);
  printf("  X  = $%02X\n", x);
  printf("  Y  = $%02X\n", y);
  printf("  SP = $%02X\n", sp);
  printf("  P  = $%02X (", status);

  if (g_debugger->get_status_flag(static_cast<Flag>(7)))
    printf("N");
  else
    printf("-");
  if (g_debugger->get_status_flag(static_cast<Flag>(6)))
    printf("V");
  else
    printf("-");
  if (g_debugger->get_status_flag(static_cast<Flag>(5)))
    printf("U");
  else
    printf("-");
  if (g_debugger->get_status_flag(static_cast<Flag>(4)))
    printf("B");
  else
    printf("-");
  if (g_debugger->get_status_flag(static_cast<Flag>(3)))
    printf("D");
  else
    printf("-");
  if (g_debugger->get_status_flag(static_cast<Flag>(2)))
    printf("I");
  else
    printf("-");
  if (g_debugger->get_status_flag(static_cast<Flag>(1)))
    printf("Z");
  else
    printf("-");
  if (g_debugger->get_status_flag(static_cast<Flag>(0)))
    printf("C");
  else
    printf("-");
  printf(")\n");

  printf("Reset vector: $%02X%02X\n", g_debugger->read_memory(0xFFFD), g_debugger->read_memory(0xFFFC));

  printf("Memory at PC ($%04X):\n  ", pc);
  for (int i = 0; i < 8; i++) {
    printf("%02X ", g_debugger->read_memory(pc + i));
  }
  printf("\n");
}

#endif  // __EMSCRIPTEN__

}  // namespace nes
