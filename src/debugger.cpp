#include "debugger.h"
#include <iomanip>
#include <sstream>
#include "types.h"

namespace nes {
// Global instance for WASM exports
static Debugger* g_debugger = nullptr;

Debugger::Debugger(CPU& cpu, Bus& bus)
  : _cpu(cpu)
  , _bus(bus)
  , _running(false)
  , _instruction_count(0)
  , _cycle_count(0) {
  // Set global instance for WASM exports
  g_debugger = this;
}

// Execute one instruction
void Debugger::step() {
  do {
    _cpu.clock();
    _cycle_count++;
  } while (_cpu.get_remaining_cycles() > 0);

  _instruction_count++;
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

DisassembledInstruction Debugger::disassemble_instruction(u16 address) const {
  DisassembledInstruction result;
  result.address = address;

  u8 opcode = read_memory(address);
  result.opcode = opcode;

  const auto& instruction = _cpu.get_instruction((Opcode)opcode);
  result.mnemonic = instruction.name;
  result.cycles = instruction.cycles;

  u16 operand = 0;
  u8 bytes = 1;  // All instructions have at least 1 byte (opcode)

  if (instruction.is_implied) {
    // Implied addressing has no operand
    result.bytes = 1;
  } else {
    // Determine addressing mode and operand size
    std::string addr_mode = address_mode_string(opcode);

    if (addr_mode == "IMM" || addr_mode == "ZPG" || addr_mode == "ZPX" || addr_mode == "ZPY" || addr_mode == "IZX" || addr_mode == "IZY" ||
        addr_mode == "REL") {
      // 1-byte operand
      operand = read_memory(address + 1);
      bytes = 2;
    } else if (addr_mode == "ABS" || addr_mode == "ABX" || addr_mode == "ABY" || addr_mode == "IND") {
      // 2-byte operand
      u8 low = read_memory(address + 1);
      u8 high = read_memory(address + 2);
      operand = (high << 8) | low;
      bytes = 3;
    }
  }

  result.operand = operand;
  result.bytes = bytes;
  result.formatted = format_instruction(opcode, operand, bytes);

  return result;
}

std::vector<DisassembledInstruction> Debugger::disassemble_range(u16 start, u16 end) const {
  std::vector<DisassembledInstruction> instructions;
  u16 addr = start;

  while (addr <= end) {
    DisassembledInstruction instr = disassemble_instruction(addr);
    instructions.push_back(instr);
    addr += instr.bytes;
  }

  return instructions;
}

std::vector<DisassembledInstruction> Debugger::disassemble_around_pc(int instructions_before, int instructions_after) const {
  u16 pc = get_register_pc();
  std::vector<u16> before_addresses;
  u16 current = pc;

  // As a simple approach, I'll try disassembling from various points before PC
  // and keep ones that end exactly at PC
  for (int i = 1; i <= instructions_before * 3; i++) {
    if (current <= i) break;

    u16 test_addr = current - i;
    std::vector<DisassembledInstruction> test_instrs = disassemble_range(test_addr, current - 1);

    u16 end_addr = test_addr;
    for (const auto& instr : test_instrs) {
      end_addr += instr.bytes;
    }

    if (end_addr == current) {
      // We found a valid instruction that ends exactly at PC
      if (before_addresses.size() < instructions_before) {
        before_addresses.insert(before_addresses.begin(), test_addr);
      } else {
        before_addresses.erase(before_addresses.begin());
        before_addresses.insert(before_addresses.begin(), test_addr);
      }
    }
  }

  u16 start_addr = before_addresses.empty() ? pc : before_addresses[0];

  u16 end_addr = pc;
  for (int i = 0; i < instructions_after; i++) {
    DisassembledInstruction instr = disassemble_instruction(end_addr);
    end_addr += instr.bytes;
  }

  std::vector<DisassembledInstruction> all_instructions = disassemble_range(start_addr, end_addr);
  std::vector<DisassembledInstruction> result;
  int pc_index = -1;

  for (size_t i = 0; i < all_instructions.size(); i++) {
    if (all_instructions[i].address == pc) {
      pc_index = i;
      break;
    }
  }

  if (pc_index == -1) {
    return all_instructions;
  }

  int start_index = std::max(0, pc_index - instructions_before);
  int end_index = std::min(static_cast<int>(all_instructions.size()) - 1, pc_index + instructions_after);

  for (int i = start_index; i <= end_index; i++) {
    result.push_back(all_instructions[i]);
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

std::string Debugger::format_instruction(u8 opcode, u16 operand, u8 bytes) const {
  std::stringstream ss;
  const auto& instruction = _cpu.get_instruction((Opcode)opcode);
  std::string mnemonic = instruction.name;

  ss << mnemonic << " ";

  std::string addr_mode = address_mode_string(opcode);

  if (addr_mode == "IMM") {
    ss << "#$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand);
  } else if (addr_mode == "ZPG") {
    ss << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand);
  } else if (addr_mode == "ZPX") {
    ss << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand) << ",X";
  } else if (addr_mode == "ZPY") {
    ss << "$" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand) << ",Y";
  } else if (addr_mode == "ABS") {
    ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(operand);
  } else if (addr_mode == "ABX") {
    ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(operand) << ",X";
  } else if (addr_mode == "ABY") {
    ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(operand) << ",Y";
  } else if (addr_mode == "IND") {
    ss << "($" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(operand) << ")";
  } else if (addr_mode == "IZX") {
    ss << "($" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand) << ",X)";
  } else if (addr_mode == "IZY") {
    ss << "($" << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(operand) << "),Y";
  } else if (addr_mode == "REL") {
    // Calculate relative address for branch instructions
    int8_t offset = static_cast<int8_t>(operand);
    u16 target = get_register_pc() + 2 + offset;
    ss << "$" << std::hex << std::uppercase << std::setw(4) << std::setfill('0') << static_cast<int>(target);
  } else if (addr_mode == "ACC") {
    ss << "A";
  } else {
    // Implied or unknown
    ss << "         ";
  }

  return ss.str();
}

std::string Debugger::address_mode_string(u8 opcode) const {
  const auto& instruction = _cpu.get_instruction((Opcode)opcode);

  if (instruction.is_implied) {
    if (opcode == 0x0A || opcode == 0x4A || opcode == 0x2A || opcode == 0x6A) {
      return "ACC";
    }
    return "IMP";
  }

  u8 bbb = (opcode >> 2) & 0x07;
  u8 cc = opcode & 0x03;

  // Branch instructions
  if ((opcode & 0x1F) == 0x10) {
    return "REL";
  }

  std::string name = instruction.name;

  if (cc == 0x01) {
    // (zp,X) or (zp),Y or zp or immediate or absolute
    if (bbb == 0x00) return "IZX";
    if (bbb == 0x01) return "ZPG";
    if (bbb == 0x02) return "IMM";
    if (bbb == 0x03) return "ABS";
    if (bbb == 0x04) return "IZY";
    if (bbb == 0x05) return "ZPX";
    if (bbb == 0x06) return "ABY";
    if (bbb == 0x07) return "ABX";
  }

  if (opcode == 0x6C) return "IND";

  return "ABS";
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
    printf("PC manually set to $%04X\n", address);
  }
}

EMSCRIPTEN_EXPORT char* debugger_disassemble_around_pc(int before, int after) {
  static std::string result;
  result.clear();

  if (!g_debugger) {
    printf("Error: g_debugger is null!\n");
    return nullptr;
  }

  u16 pc = g_debugger->get_register_pc();
  printf("Disassembling around PC: %04X, before: %d, after: %d\n", g_debugger->get_register_pc(), before, after);

  auto instructions = g_debugger->disassemble_around_pc(before, after);
  std::stringstream ss;

  printf("Got %zu instructions\n", instructions.size());

  // Format: address|opcode|mnemonic|operand|formatted|bytes|cycles#address|...
  for (const auto& instr : instructions) {
    ss << instr.address << "|" << static_cast<int>(instr.opcode) << "|" << instr.mnemonic << "|" << instr.operand << "|" << instr.formatted
       << "|" << static_cast<int>(instr.bytes) << "|" << static_cast<int>(instr.cycles) << "#";

    printf("Instruction: %04X %02X %s\n", instr.address, instr.opcode, instr.formatted.c_str());
  }

  result = ss.str();
  if (!result.empty()) {
    result.pop_back();  // Remove the last '#'
  }

  printf("Final string length: %zu\n", result.length());
  return const_cast<char*>(result.c_str());
}

EMSCRIPTEN_EXPORT char* debugger_disassemble_range(u16 start, u16 end) {
  static std::string result;

  if (!g_debugger) {
    return nullptr;
  }

  auto instructions = g_debugger->disassemble_range(start, end);
  std::stringstream ss;

  // Format: address|opcode|mnemonic|operand|formatted|bytes|cycles#address|...
  for (const auto& instr : instructions) {
    ss << instr.address << "|" << static_cast<int>(instr.opcode) << "|" << instr.mnemonic << "|" << instr.operand << "|" << instr.formatted
       << "|" << static_cast<int>(instr.bytes) << "|" << static_cast<int>(instr.cycles) << "#";
  }

  result = ss.str();
  if (!result.empty()) {
    result.pop_back();  // Remove the last '#'
  }

  return const_cast<char*>(result.c_str());
}

EMSCRIPTEN_EXPORT void debugger_test() {
  printf("======= DEBUGGER TEST =======\n");
  if (g_debugger) {
    u16 pc = g_debugger->get_register_pc();
    printf("Current PC: %04X\n", pc);

    // Print some memory values around PC
    printf("Memory at PC:\n");
    for (int i = 0; i < 10; i++) {
      printf("%02X ", g_debugger->read_memory(pc + i));
    }
    printf("\n");

    // Try disassembling
    auto instructions = g_debugger->disassemble_around_pc(0, 5);
    printf("Disassembly result has %zu instructions\n", instructions.size());
  } else {
    printf("Debugger instance is null!\n");
  }
  printf("============================\n");
}

EMSCRIPTEN_EXPORT void debugger_init_test() {
  if (!g_debugger) return;

  // Set up the test program at address 0x8000
  u16 addr = 0x8000;

  // LDA #$42 (Load 0x42 into A register)
  g_debugger->write_memory(addr++, 0xA9);
  g_debugger->write_memory(addr++, 0x42);

  // STA $0200 (Store A at memory address 0x0200)
  g_debugger->write_memory(addr++, 0x8D);
  g_debugger->write_memory(addr++, 0x00);
  g_debugger->write_memory(addr++, 0x02);

  // Set reset vector to point to our program
  g_debugger->write_memory(0xFFFC, 0x00);
  g_debugger->write_memory(0xFFFD, 0x80);

  // Reset CPU (which might not set PC correctly)
  g_debugger->reset();

  // Force PC to the correct address
  g_debugger->set_pc(0x8000);

  printf("Test program initialized. PC set to $8000\n");
  printf("Memory at 0x8000: $%02X $%02X $%02X $%02X $%02X\n", g_debugger->read_memory(0x8000), g_debugger->read_memory(0x8001),
         g_debugger->read_memory(0x8002), g_debugger->read_memory(0x8003), g_debugger->read_memory(0x8004));
}

EMSCRIPTEN_EXPORT void debugger_print_state() {
  if (!g_debugger) {
    printf("Error: g_debugger is null!\n");
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

  // Use the proper enum values for Flag instead of integers
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

  // Print reset vector
  printf("Reset vector: $%02X%02X\n", g_debugger->read_memory(0xFFFD), g_debugger->read_memory(0xFFFC));

  // Print memory around PC
  printf("Memory at PC ($%04X):\n  ", pc);
  for (int i = 0; i < 8; i++) {
    printf("%02X ", g_debugger->read_memory(pc + i));
  }
  printf("\n");
}

#endif  // __EMSCRIPTEN__

}  // namespace nes
