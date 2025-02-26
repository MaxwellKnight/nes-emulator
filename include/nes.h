#pragma once
#include <vector>
#include "../include/bus.h"
#include "../include/cpu.h"

class NES {
 private:
  nes::Bus _bus;
  nes::CPU _cpu;
  bool _debug_mode;

 public:
  NES(bool debug_mode = false);

  void load_program(const std::vector<uint8_t>& program, uint16_t address = 0x0100);
  void execute(int max_steps = 1000);
  void print_cpu_state(int step = -1) const;
  void print_results() const;
  void print_zero_page(uint8_t start, uint8_t end) const;
};
