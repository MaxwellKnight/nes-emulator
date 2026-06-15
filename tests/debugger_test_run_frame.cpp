#include <gtest/gtest.h>
#include "bus.h"
#include "debugger.h"
#include "test_cartridge.h"

using namespace nes;

class DebuggerRunFrameTest : public ::testing::Test {
 protected:
  void SetUp() override {
    auto cart = std::make_shared<MockCartridge>();
    bus.insert_cartridge(cart);
    bus.reset();
  }

  // Write a byte into CPU address space. RAM $0000-$1FFF is writable through
  // the Bus (the cartridge mapper only claims $8000-$FFFF).
  void poke(u16 addr, u8 value) { bus.cpu_write(addr, value); }

  Bus bus;
  Debugger dbg{bus.get_cpu(), bus};
};

// An infinite JMP loop runs until the PPU completes a frame -> reason 0.
TEST_F(DebuggerRunFrameTest, ReturnsZeroWhenFrameCompletes) {
  // JMP $0000 at $0000.
  poke(0x0000, 0x4C);
  poke(0x0001, 0x00);
  poke(0x0002, 0x00);

  dbg.set_pc(0x0000);

  u32 start_frame = bus.get_ppu().frame_count();
  int reason = dbg.run_frame();

  EXPECT_EQ(reason, 0);
  EXPECT_EQ(bus.get_ppu().frame_count(), start_frame + 1);
}

// A breakpoint inside the loop stops run_frame early -> reason 1.
TEST_F(DebuggerRunFrameTest, ReturnsOneWhenBreakpointHit) {
  // JMP $0000 at $0000 (loops back onto itself).
  poke(0x0000, 0x4C);
  poke(0x0001, 0x00);
  poke(0x0002, 0x00);

  dbg.set_pc(0x0000);
  dbg.add_breakpoint(0x0000);

  int reason = dbg.run_frame();

  EXPECT_EQ(reason, 1);
  EXPECT_EQ(dbg.get_register_pc(), 0x0000);
}

// A BRK stops run_frame -> reason 2.
TEST_F(DebuggerRunFrameTest, ReturnsTwoOnBrk) {
  // BRK at $0000.
  poke(0x0000, 0x00);

  dbg.set_pc(0x0000);

  int reason = dbg.run_frame();

  EXPECT_EQ(reason, 2);
}
