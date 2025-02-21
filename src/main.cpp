#include "../include/cpu.h"
#include <iomanip>
#include <iostream>

void print_cpu_state(const nes::CPU &cpu, const char *label = nullptr) {
  if (label) {
    std::cout << label << ":\n";
  }
  std::cout << "CPU State:\n";
  std::cout << "A:  0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cpu.get_accumulator()) << '\n';
  std::cout << "X:  0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cpu.get_x()) << '\n';
  std::cout << "Y:  0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cpu.get_y()) << '\n';
  std::cout << "PC: 0x" << std::hex << std::setw(4) << std::setfill('0')
            << static_cast<int>(cpu.get_pc()) << '\n';
  std::cout << "SP: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cpu.get_sp()) << '\n';
  std::cout << "Status: 0x" << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(cpu.get_status()) << "\n";
  std::cout << "Cycles: " << std::dec
            << static_cast<int>(cpu.get_remaining_cycles()) << "\n\n";
}

void execute_instruction(nes::CPU &cpu, nes::Memory &mem) {
  int initial_cycles = cpu.get_remaining_cycles();
  std::cout << "Starting execution at PC: 0x" << std::hex << cpu.get_pc()
            << " with " << std::dec << initial_cycles << " cycles\n";

  do {
    cpu.clock(mem);
  } while (cpu.get_remaining_cycles() > 0);

  std::cout << "Instruction complete\n\n";
}

int main() {
  using namespace nes;
  CPU cpu;
  Memory mem;

  try {
    // Setup test program in memory first
    std::cout << "Setting up test program in memory...\n";
    mem.write(0xFFFC, 0xA9); // LDA Immediate
    mem.write(0xFFFD, 0x42); // Value 0x42
    mem.write(0xFFFE, 0xAA); // TAX
    mem.write(0xFFFF, 0x85); // STA Zero Page
    mem.write(0x0000, 0x20); // Zero Page address 0x20
    mem.write(0x0001, 0xA5); // LDA Zero Page
    mem.write(0x0002, 0x20); // Same Zero Page address

    // Initialize CPU
    cpu.reset();
    print_cpu_state(cpu, "Initial CPU State");

    // Test 1: LDA Immediate
    std::cout << "\nTest 1: LDA Immediate (0x42)\n";
    std::cout << "Reading from PC: 0x" << std::hex << cpu.get_pc()
              << " value: 0x" << static_cast<int>(mem.read(cpu.get_pc()))
              << "\n";
    execute_instruction(cpu, mem);
    print_cpu_state(cpu, "After LDA #$42");

    // Test 2: TAX
    std::cout << "\nTest 2: TAX\n";
    std::cout << "Reading from PC: 0x" << std::hex << cpu.get_pc()
              << " value: 0x" << static_cast<int>(mem.read(cpu.get_pc()))
              << "\n";
    execute_instruction(cpu, mem);
    print_cpu_state(cpu, "After TAX");

    // Test 3: STA Zero Page
    std::cout << "\nTest 3: STA Zero Page (to 0x20)\n";
    std::cout << "Reading from PC: 0x" << std::hex << cpu.get_pc()
              << " value: 0x" << static_cast<int>(mem.read(cpu.get_pc()))
              << "\n";
    execute_instruction(cpu, mem);
    print_cpu_state(cpu, "After STA $20");
    std::cout << "Value at 0x20: 0x" << std::hex
              << static_cast<int>(mem.read(0x20)) << "\n\n";

    // Test 4: LDA Zero Page
    std::cout << "\nTest 4: LDA Zero Page (from 0x20)\n";
    std::cout << "Reading from PC: 0x" << std::hex << cpu.get_pc()
              << " value: 0x" << static_cast<int>(mem.read(cpu.get_pc()))
              << "\n";
    execute_instruction(cpu, mem);
    print_cpu_state(cpu, "After LDA $20");

  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }

  return 0;
}
