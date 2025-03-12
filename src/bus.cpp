#include "bus.h"

namespace nes {
Bus::Bus() { _cpu.connect_bus(this); }

void Bus::clock() { _cpu.clock(); }
void Bus::reset() {
  _sys_clock = 0;
  _cpu.reset();
}

CPU& Bus::get_cpu() { return _cpu; }

bool Bus::handles_address(u16 address) const { return true; }

void Bus::cpu_write(u16 address, u8 value) {
  if (address >= 0x0000 && address <= 0x1FFF) {
    _ram[address & 0x07FF] = value;
  } else if (address >= 0xFFFC && address <= 0xFFFF) {
    _reset_vector[address - 0xFFFC] = value;
  }
}

u8 Bus::cpu_read(u16 address) const {
  u8 addr = 0x00;
  if (address >= 0x0000 && address <= 0x1FFF) {
    addr = _ram[address & 0x07FF];
  } else if (address >= 0xFFFC && address <= 0xFFFF) {
    addr = _reset_vector[address - 0xFFFC];
  }

  return addr;
}
}  // namespace nes
