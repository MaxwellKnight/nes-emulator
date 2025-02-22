#include "../include/bus.h"
#include "../include/cpu.h"
#include <iomanip>
#include <iostream>

void print_cpu_state(const nes::CPU &cpu) {
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
            << static_cast<int>(cpu.get_status()) << "\n\n";
}

int main() {
  nes::Bus bus;
  nes::CPU cpu(bus);

  cpu.reset();
  print_cpu_state(cpu);
  return 0;
}
