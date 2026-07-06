#include <gtest/gtest.h>
#include "bus.h"
#include "test_cartridge.h"

class CPUTestBase : public ::testing::Test {
 protected:
  void SetUp() override {
    auto cart = std::make_shared<nes::MockCartridge>();
    bus.insert_cartridge(cart);
    bus.reset();
  }

  void execute_cycles(int cycles) {
    for (int i = 0; i < cycles; i++) {
      bus.clock();
    }
  }

  nes::Bus bus;
  nes::CPU& cpu = bus.get_cpu();
};
