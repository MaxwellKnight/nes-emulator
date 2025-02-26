#include "nes.h"
#include <iomanip>
#include <iostream>

NES::NES(bool debug_mode)
  : _cpu(_bus)
  , _debug_mode(debug_mode) {
  _cpu.reset();
}

void NES::load_program(const std::vector<uint8_t>& program, uint16_t address) {
  for (size_t i = 0; i < program.size(); i++) {
    _bus.write(address + i, program[i]);
  }
  _cpu.set_pc(address);

  if (_debug_mode) {
    std::cout << "Program loaded at address 0x" << std::hex << address << std::endl;
  }
}

void NES::execute(int max_steps) {
  int step = 0;

  std::cout << "Starting execution at PC: 0x" << std::hex << _cpu.get_pc() << std::endl;

  try {
    while (step < max_steps) {
      // Print debug information
      if (_debug_mode && step < 10) {
        print_cpu_state(step);
      }

      // Execute one instruction
      uint16_t old_pc = _cpu.get_pc();

      // Clock the CPU until a complete instruction is executed
      while (_cpu.get_remaining_cycles() == 0) {
        _cpu.clock();
      }

      while (_cpu.get_remaining_cycles() > 0) {
        _cpu.clock();
      }

      step++;

      // Check for termination conditions
      if (_cpu.get_pc() >= 0x0100 + 0xFF || _cpu.read_byte(_cpu.get_pc()) == 0xEA) {
        std::cout << "Program execution complete!" << std::endl;
        break;
      }

      // Check for infinite loop
      if (old_pc == _cpu.get_pc()) {
        std::cout << "Warning: Possible infinite loop at PC: 0x" << std::hex << _cpu.get_pc() << std::endl;
        break;
      }
    }

    if (step >= max_steps) {
      std::cout << "Warning: Maximum step count reached (" << max_steps << ")" << std::endl;
    }

  } catch (const std::exception& e) {
    std::cerr << "Error during execution at PC 0x" << std::hex << _cpu.get_pc() << ": " << e.what() << std::endl;
  }
}

void NES::print_cpu_state(int step) const {
  if (step >= 0) {
    std::cout << "Step " << std::dec << step << " - ";
  }

  std::cout << "PC: 0x" << std::hex << _cpu.get_pc() << " A: 0x" << static_cast<int>(_cpu.get_accumulator()) << " X: 0x"
            << static_cast<int>(_cpu.get_x()) << " Y: 0x" << static_cast<int>(_cpu.get_y()) << " Status: 0x"
            << static_cast<int>(_cpu.get_status()) << std::endl;
}

void NES::print_results() const {
  // Print CPU state
  std::cout << "\nFinal CPU state:" << std::endl;
  print_cpu_state();

  // If Y register is used to count primes, indicate this
  std::cout << "Y: 0x" << std::hex << static_cast<int>(_cpu.get_y()) << " (Number of primes: " << std::dec << static_cast<int>(_cpu.get_y())
            << ")" << std::endl;
}

void NES::print_zero_page(uint8_t start, uint8_t end) const {
  std::cout << "\nZero Page Memory (0x" << std::hex << static_cast<int>(start) << " - 0x" << static_cast<int>(end) << "):" << std::endl;

  for (uint8_t addr = start; addr <= end; addr++) {
    uint8_t value = _bus.read(addr);
    std::cout << "0x" << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(addr) << ": 0x" << std::setw(2)
              << static_cast<int>(value) << " (" << std::dec << static_cast<int>(value) << ")" << std::endl;
  }
}
