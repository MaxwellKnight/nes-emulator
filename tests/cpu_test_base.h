#include "../include/bus.h"
#include "../include/cpu.h"
#include "types.h"
#include <gtest/gtest.h>

class CPUTestBase : public ::testing::Test {
protected:
  void SetUp() override { cpu.reset(); }

  void execute_cycles(int cycles) {
    for (int i = 0; i < cycles; i++) {
      cpu.clock();
    }
    EXPECT_EQ(cpu.get_remaining_cycles(), 0);
  }

  nes::Bus bus;
  nes::CPU cpu{bus};
};
