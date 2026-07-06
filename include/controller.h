#pragma once
#include "types.h"

namespace nes {

// Standard NES controller (4021 shift register). Button bit layout (matches the
// serial read order, A reported first):
//   d0 A, d1 B, d2 Select, d3 Start, d4 Up, d5 Down, d6 Left, d7 Right
class Controller {
 public:
  // Latch the current button state (called by the front-end each frame).
  void set_buttons(u8 buttons) { _state = buttons; }

  // $4016 write: while the strobe bit (d0) is high the shift register is
  // continuously reloaded; the falling edge latches the state for shifting.
  void write(u8 value) {
    _strobe = value & 0x01;
    if (_strobe) _shift = _state;
  }

  // $4016/$4017 read: report the next button bit (d0). While the strobe is high
  // the controller keeps reporting button A. After 8 reads the open bus shifts
  // in 1s (real hardware reads as 1).
  u8 read() {
    if (_strobe) {
      _shift = _state;
      return _state & 0x01;
    }
    u8 bit = _shift & 0x01;
    _shift = static_cast<u8>((_shift >> 1) | 0x80);
    return bit;
  }

  void reset() {
    _state = 0;
    _shift = 0;
    _strobe = 0;
  }

 private:
  u8 _state = 0;
  u8 _shift = 0;
  u8 _strobe = 0;
};

}  // namespace nes
