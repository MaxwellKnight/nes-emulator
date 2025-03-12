#include <gtest/gtest.h>
#include "../include/bus.h"
#include "../include/cpu.h"

class CPUTestBase : public ::testing::Test {
 protected:
  void SetUp() override { bus.reset(); }

  void execute_cycles(int cycles) {
    for (int i = 0; i < cycles; i++) {
      bus.clock();
    }
  }

  nes::Bus bus;
  nes::CPU& cpu = bus.get_cpu();
};
