#include "cpu_test_base.h"

class CPUTestInit : public CPUTestBase {};

TEST_F(CPUTestInit, initialization) {
  EXPECT_EQ(cpu.get_accumulator(), 0x00);
  EXPECT_EQ(cpu.get_x(), 0x00);
  EXPECT_EQ(cpu.get_y(), 0x00);
  EXPECT_EQ(cpu.get_sp(), 0xFF);
  EXPECT_EQ(cpu.get_pc(), 0xFFFC);
  EXPECT_EQ(cpu.get_status() & 0x30, 0x30); // UNUSED and BREAK flags
  EXPECT_EQ(cpu.get_remaining_cycles(), 0); // Should start with 0 cycles
}
