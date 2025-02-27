#include <gtest/gtest.h>
#include "../include/bus.h"
#include "../include/cpu.h"
#include "types.h"

class CPUTestBase : public ::testing::Test {
 protected:
  void SetUp() override { cpu.reset(); }

  void execute_cycles(int cycles) {
    for (int i = 0; i < cycles; i++) {
      cpu.clock();
    }
  }

  nes::Bus bus;
  nes::CPU cpu{bus};
};
