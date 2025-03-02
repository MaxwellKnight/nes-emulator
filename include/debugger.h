#pragma once
#include <array>
#include <string>
#include <unordered_map>
#include <vector>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#define EMSCRIPTEN_EXPORT extern "C" EMSCRIPTEN_KEEPALIVE
#else
#define EMSCRIPTEN_EXPORT
#endif

#include "bus.h"
#include "cpu.h"

namespace nes {

struct DisassembledInstruction {
  u16 address;
  u8 opcode;
  std::string mnemonic;
  u16 operand;
  std::string formatted;
  u8 bytes;
  u8 cycles;
};

class Debugger {
 public:
  Debugger(CPU& cpu, Bus& bus);

  // Execution control
  void step();
  void run();
  void stop();
  void reset();
  bool is_running() const;

  // Breakpoint methods
  void add_breakpoint(u16 address);
  void remove_breakpoint(u16 address);
  void clear_breakpoints();
  bool has_breakpoint(u16 address) const;
  std::vector<u16> get_breakpoints() const;

  // State inspection methods
  u8 get_register_a() const;
  u8 get_register_x() const;
  u8 get_register_y() const;
  u8 get_register_sp() const;
  u16 get_register_pc() const;
  u8 get_register_status() const;
  u8 get_status_flag(Flag flag) const;
  void set_pc(u16 address);

  // Memory access methods
  u8 read_memory(u16 address) const;
  void write_memory(u16 address, u8 value);
  std::vector<u8> read_memory_range(u16 start, u16 end) const;
  std::vector<u8> get_stack() const;

  // Statistics
  u64 get_instruction_count() const;
  u64 get_cycle_count() const;

  // Disassembly methods
  DisassembledInstruction disassemble_instruction(u16 address) const;
  std::vector<DisassembledInstruction> disassemble_range(u16 start, u16 end) const;
  std::vector<DisassembledInstruction> disassemble_around_pc(int instructions_before, int instructions_after) const;

  // Helper methods for disassembly
  u8 get_instruction_bytes(u8 opcode) const;
  std::string address_mode_string(u8 opcode) const;
  std::string format_instruction(u8 opcode, u16 operand, u8 bytes, u16 instruction_addr = 0) const;

 private:
  void check_breakpoints();
  void init_addressing_mode_table();

  CPU& _cpu;
  Bus& _bus;
  bool _running;
  u64 _instruction_count;
  u64 _cycle_count;
  std::unordered_map<u16, bool> _breakpoints;

  // Lookup table for addressing modes by opcode
  std::array<std::string, 256> _addressing_mode_table;
};

}  // namespace nes
