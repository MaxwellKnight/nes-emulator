#include "../include/bus.h"
#include "../include/cpu.h"
#include <cassert>
#include <iomanip>
#include <iostream>

namespace {
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

void execute_instruction(nes::CPU &cpu) {
  int initial_cycles = cpu.get_remaining_cycles();
  std::cout << "Executing instruction at PC: 0x" << std::hex << cpu.get_pc()
            << " with " << std::dec << initial_cycles << " cycles\n";
  do {
    cpu.clock();
  } while (cpu.get_remaining_cycles() > 0);
  std::cout << "Instruction complete\n\n";
}
} // namespace

class CPUTester {
public:
  void test_initialization() {
    std::cout << "Testing CPU initialization...\n";
    print_cpu_state(cpu, "Initial state");

    // Verify initial state
    assert(cpu.get_accumulator() == 0x00 && "A should be 0");
    assert(cpu.get_x() == 0x00 && "X should be 0");
    assert(cpu.get_y() == 0x00 && "Y should be 0");
    assert(cpu.get_sp() == 0xFF && "SP should be 0xFF");
    assert(cpu.get_pc() == 0xFFFC && "PC should be 0xFFFC");
    assert((cpu.get_status() & 0x30) == 0x30 &&
           "UNUSED and BREAK flags should be set");
  }

  void test_lda_immediate() {
    std::cout << "Testing LDA Immediate...\n";

    // Setup test program
    bus.write(0xFFFC,
              static_cast<nes::u8>(nes::Opcode::LDA_IM)); // LDA Immediate
    bus.write(0xFFFD, 0x42);                              // Value to load

    print_cpu_state(cpu, "Before LDA #$42");
    execute_instruction(cpu);
    print_cpu_state(cpu, "After LDA #$42");

    // Verify results
    assert(cpu.get_accumulator() == 0x42 && "A should be 0x42");
    assert(cpu.get_pc() == 0xFFFE && "PC should be 0xFFFE");
    assert(!cpu.get_flag(nes::Flag::ZERO) && "Zero flag should be clear");
    assert(!cpu.get_flag(nes::Flag::NEGATIVE) &&
           "Negative flag should be clear");
  }

  void test_tax() {
    std::cout << "Testing TAX...\n";

    // Setup: First load a value into A
    bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_IM));
    bus.write(0xFFFD, 0x42);
    execute_instruction(cpu);

    // Then do TAX
    bus.write(0xFFFE, static_cast<nes::u8>(nes::Opcode::TAX));
    print_cpu_state(cpu, "Before TAX");
    execute_instruction(cpu);
    print_cpu_state(cpu, "After TAX");

    // Verify results
    assert(cpu.get_x() == 0x42 && "X should be 0x42");
    assert(cpu.get_accumulator() == 0x42 && "A should still be 0x42");
    assert(!cpu.get_flag(nes::Flag::ZERO) && "Zero flag should be clear");
    assert(!cpu.get_flag(nes::Flag::NEGATIVE) &&
           "Negative flag should be clear");
  }

  void test_lda_zero_page() {
    std::cout << "Testing LDA Zero Page...\n";

    // Setup: Store value in zero page first
    bus.write(0x0042, 0x37); // Value in zero page

    // Then load it using zero page addressing
    bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_ZP));
    bus.write(0xFFFD, 0x42); // Zero page address

    print_cpu_state(cpu, "Before LDA $42");
    execute_instruction(cpu);
    print_cpu_state(cpu, "After LDA $42");

    // Verify results
    assert(cpu.get_accumulator() == 0x37 && "A should be 0x37");
    assert(!cpu.get_flag(nes::Flag::ZERO) && "Zero flag should be clear");
    assert(!cpu.get_flag(nes::Flag::NEGATIVE) &&
           "Negative flag should be clear");
  }

  void test_sta_zero_page() {
    std::cout << "Testing STA Zero Page...\n";

    // Setup: First load a value into A
    bus.write(0xFFFC, static_cast<nes::u8>(nes::Opcode::LDA_IM));
    bus.write(0xFFFD, 0x42);
    execute_instruction(cpu);

    // Then store it to zero page
    bus.write(0xFFFE, static_cast<nes::u8>(nes::Opcode::STA_ZP));
    bus.write(0xFFFF, 0x20);

    print_cpu_state(cpu, "Before STA $20");
    execute_instruction(cpu);
    print_cpu_state(cpu, "After STA $20");

    // Verify results
    assert(bus.read(0x20) == 0x42 && "Memory at $20 should be 0x42");
    assert(cpu.get_accumulator() == 0x42 && "A should still be 0x42");
  }

  void run_all_tests() {
    test_initialization();
    cpu.reset();
    test_lda_immediate();
    cpu.reset();
    test_tax();
    cpu.reset();
    test_lda_zero_page();
    cpu.reset();
    test_sta_zero_page();

    std::cout << "\nAll tests completed successfully!\n";
  }

private:
  nes::Bus bus;
  nes::CPU cpu;
};

int main() {
  try {
    CPUTester tester;
    tester.run_all_tests();
  } catch (const std::exception &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
