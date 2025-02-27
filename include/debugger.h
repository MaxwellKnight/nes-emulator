#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "../include/bus.h"
#include "../include/cpu.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EMSCRIPTEN_EXPORT EMSCRIPTEN_KEEPALIVE extern "C"
#else
#define EMSCRIPTEN_EXPORT
#endif

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
  ~Debugger() = default;

  // Execution control
  void step();
  void run();
  void stop();
  void reset();
  bool is_running() const;

  // Breakpoint management
  void add_breakpoint(u16 address);
  void remove_breakpoint(u16 address);
  void clear_breakpoints();
  bool has_breakpoint(u16 address) const;
  std::vector<u16> get_breakpoints() const;

  // Disassembly
  DisassembledInstruction disassemble_instruction(u16 address) const;
  std::vector<DisassembledInstruction> disassemble_range(u16 start, u16 end) const;
  std::vector<DisassembledInstruction> disassemble_around_pc(int instructions_before = 5, int instructions_after = 10) const;

  // State inspection
  u8 get_register_a() const;
  u8 get_register_x() const;
  u8 get_register_y() const;
  u8 get_register_sp() const;
  u16 get_register_pc() const;
  u8 get_register_status() const;
  u8 get_status_flag(nes::Flag flag) const;

  // Memory access
  u8 read_memory(u16 address) const;
  void write_memory(u16 address, u8 value);
  std::vector<u8> read_memory_range(u16 start, u16 end) const;

  // Stack inspection
  std::vector<u8> get_stack() const;

  // Instruction statistics
  u64 get_instruction_count() const;
  u64 get_cycle_count() const;

 private:
  CPU& _cpu;
  Bus& _bus;
  bool _running;
  std::unordered_map<u16, bool> _breakpoints;
  u64 _instruction_count;
  u64 _cycle_count;

  // Helper methods
  void check_breakpoints();
  std::string format_instruction(u8 opcode, u16 operand, u8 bytes) const;
  std::string address_mode_string(u8 opcode) const;
};

// WASM exported functions
#ifdef __EMSCRIPTEN__

EMSCRIPTEN_EXPORT void debugger_step();
EMSCRIPTEN_EXPORT void debugger_run();
EMSCRIPTEN_EXPORT void debugger_stop();
EMSCRIPTEN_EXPORT void debugger_reset();
EMSCRIPTEN_EXPORT int debugger_is_running();

EMSCRIPTEN_EXPORT void debugger_add_breakpoint(u16 address);
EMSCRIPTEN_EXPORT void debugger_remove_breakpoint(u16 address);
EMSCRIPTEN_EXPORT void debugger_clear_breakpoints();

EMSCRIPTEN_EXPORT u8 debugger_get_register_a();
EMSCRIPTEN_EXPORT u8 debugger_get_register_x();
EMSCRIPTEN_EXPORT u8 debugger_get_register_y();
EMSCRIPTEN_EXPORT u8 debugger_get_register_sp();
EMSCRIPTEN_EXPORT u16 debugger_get_register_pc();
EMSCRIPTEN_EXPORT u8 debugger_get_register_status();
EMSCRIPTEN_EXPORT u8 debugger_get_status_flag(nes::Flag flag);

EMSCRIPTEN_EXPORT u8 debugger_read_memory(u16 address);
EMSCRIPTEN_EXPORT void debugger_write_memory(u16 address, u8 value);

EMSCRIPTEN_EXPORT u64 debugger_get_instruction_count();
EMSCRIPTEN_EXPORT u64 debugger_get_cycle_count();

// Helper function to get disassembly data to JavaScript
EMSCRIPTEN_EXPORT char* debugger_disassemble_around_pc(int before, int after);
EMSCRIPTEN_EXPORT char* debugger_disassemble_range(u16 start, u16 end);

#endif  // __EMSCRIPTEN__

}  // namespace nes
