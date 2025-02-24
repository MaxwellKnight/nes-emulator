#include "../include/bus.h"
#include "../include/cpu.h"
#include "types.h"
#include <gtest/gtest.h>

class CPUTestInit : public ::testing::Test {
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

TEST_F(CPUTestInit, initialization) {
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_EQ(cpu.get_sp(), 0xFF);
  EXPECT_EQ(cpu.get_pc(), 0xFFFC);
  EXPECT_EQ(cpu.get_status() & 0x30, 0x30); // UNUSED and BREAK flags
  EXPECT_EQ(cpu.get_remaining_cycles(), 0); // Should start with 0 cycles
}
