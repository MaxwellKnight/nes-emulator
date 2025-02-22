#include "../include/bus.h"

namespace nes {

Bus::Bus() {}
void Bus::write(u16 address, u8 value) { memory[address] = value; }

u8 Bus::read(u16 address) const { return memory[address]; }

u16 Bus::read_word(u16 address) const {
  u8 low = read(address);
  u8 high = read(address + 1);
  return static_cast<u16>(low) | (static_cast<u16>(high) << 8);
}

void Bus::write_word(u16 address, u16 value) {
  write(address, value & 0xFF);
  write(address + 1, (value >> 8) & 0xFF);
}

} // namespace nes
