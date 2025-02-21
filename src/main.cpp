#include "../include/cpu.h"
#include <iomanip>
#include <iostream>

void print_cpu_state(const nes::CPU &cpu) {
  std::cout << "CPU State:\n";
  std::cout << "A:  0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cpu.get_accumulator()) << '\n';
  std::cout << "X:  0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cpu.get_x()) << '\n';
  std::cout << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0')
            << static_cast<int>(cpu.get_pc()) << '\n';
  std::cout << "Status: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cpu.get_status()) << "\n\n";
}

int main() {
  using namespace nes;
  CPU cpu;
  Memory mem;

  try {
    cpu.reset();
    std::cout << "Initial CPU State:\n";
    print_cpu_state(cpu);

    mem.write(0xFFFC, 0xA9); // LDA Immediate opcode
    mem.write(0xFFFD, 0x42); // Value to load
    mem.write(0xFFFE, 0xAA); // TAX opcode
    mem.write(0xFFFF, 0x85); // STA Zero Page opcode
    mem.write(0x0000, 0x10); // Zero Page address
    mem.write(0x0001, 0xA5); // LDA Zero Page opcode
    mem.write(0x0002, 0x10); // Zero Page address

    std::cout << "Executing :\n";
    cpu.run(mem);
    print_cpu_state(cpu);
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
