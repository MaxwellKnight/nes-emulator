#include "../include/bus.h"

namespace nes {
Bus::Bus() {}

bool Bus::handles_address(u16 address) const { return true; }

void Bus::write(u16 address, u8 value) {
  if (address >= 0x0000 && address <= 0x1FFF) {
    _ram[address & 0x07FF] = value;
  } else if (address >= 0xFFFC && address <= 0xFFFF) {
    _reset_vector[address - 0xFFFC] = value;
  }
}

u8 Bus::read(u16 address) const {
  u8 addr = 0x00;
  if (address >= 0x0000 && address <= 0x1FFF) {
    addr = _ram[address & 0x07FF];
  } else if (address >= 0xFFFC && address <= 0xFFFF) {
    addr = _reset_vector[address - 0xFFFC];
  }

  return addr;
}

u16 Bus::read_word(u16 address) const {
  u8 low = read(address);
  u8 high = read(address + 1);
  return (u16)low | ((u16)high << 8);
}

void Bus::write_word(u16 address, u16 value) {
  write(address, value & 0xFF);
  write(address + 1, (value >> 8) & 0xFF);
}
}  // namespace nes
