#include <gtest/gtest.h>
#include "bus.h"
#include "controller.h"

using namespace nes;

// Button bit layout for set_buttons (see controller.h).
namespace {
constexpr u8 BTN_A = 0x01;
constexpr u8 BTN_B = 0x02;
constexpr u8 BTN_SELECT = 0x04;
constexpr u8 BTN_START = 0x08;
constexpr u8 BTN_UP = 0x10;
constexpr u8 BTN_DOWN = 0x20;
constexpr u8 BTN_LEFT = 0x40;
constexpr u8 BTN_RIGHT = 0x80;
}  // namespace

// Strobe (write 1 then 0) latches state; reads return buttons A..Right in order.
TEST(ControllerTest, SerialReadOrder) {
  Controller c;
  c.set_buttons(BTN_A | BTN_START | BTN_RIGHT);
  c.write(1);  // strobe high
  c.write(0);  // latch on falling edge

  // Order: A, B, Select, Start, Up, Down, Left, Right
  EXPECT_EQ(c.read() & 1, 1);  // A
  EXPECT_EQ(c.read() & 1, 0);  // B
  EXPECT_EQ(c.read() & 1, 0);  // Select
  EXPECT_EQ(c.read() & 1, 1);  // Start
  EXPECT_EQ(c.read() & 1, 0);  // Up
  EXPECT_EQ(c.read() & 1, 0);  // Down
  EXPECT_EQ(c.read() & 1, 0);  // Left
  EXPECT_EQ(c.read() & 1, 1);  // Right
  // After 8 reads the open bus reads as 1.
  EXPECT_EQ(c.read() & 1, 1);
  EXPECT_EQ(c.read() & 1, 1);
}

// While the strobe is held high the controller keeps reporting button A.
TEST(ControllerTest, StrobeHighReportsA) {
  Controller c;
  c.set_buttons(BTN_A);
  c.write(1);  // strobe stays high
  EXPECT_EQ(c.read() & 1, 1);
  EXPECT_EQ(c.read() & 1, 1);  // still A, no shifting
  c.set_buttons(0x00);
  EXPECT_EQ(c.read() & 1, 0);  // reflects live state while strobed
}

// $4016 write strobes; $4016 read returns player 1, $4017 read returns player 2.
TEST(ControllerTest, BusRoutesPorts) {
  Bus bus;
  bus.set_controller(0, BTN_B);      // player 1: B
  bus.set_controller(1, BTN_SELECT); // player 2: Select
  bus.cpu_write(0x4016, 1);          // strobe
  bus.cpu_write(0x4016, 0);

  // Player 1: A=0, B=1, ...
  EXPECT_EQ(bus.cpu_read(0x4016) & 1, 0);  // A
  EXPECT_EQ(bus.cpu_read(0x4016) & 1, 1);  // B
  // Player 2: A=0, B=0, Select=1
  EXPECT_EQ(bus.cpu_read(0x4017) & 1, 0);  // A
  EXPECT_EQ(bus.cpu_read(0x4017) & 1, 0);  // B
  EXPECT_EQ(bus.cpu_read(0x4017) & 1, 1);  // Select
}
