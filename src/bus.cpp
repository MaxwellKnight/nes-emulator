#include "bus.h"

namespace nes {
Bus::Bus()
  : _sys_clock(0)
  , _cpu(*this)
  , _cartridge(nullptr) {}

void Bus::clock() {
  _cpu.clock();
  _sys_clock++;
}
void Bus::reset() {
  _sys_clock = 0;
  _cpu.reset();
}

CPU& Bus::get_cpu() { return _cpu; }
void Bus::insert_cartridge(const std::shared_ptr<Cartridge>& cartridge) {
  _cartridge = cartridge;
  _ppu.insert_cartridge(cartridge);
}

void Bus::cpu_write(u16 address, u8 value) {
  if (_cartridge && _cartridge->cpu_write(address, value)) {
  } else if (address >= 0x0000 && address <= 0x1FFF) {
    _ram[address & 0x07FF] = value;
  } else if (address >= 0x2000 && address <= 0x3FFF) {
    _ppu.cpu_write(address & 0x0007, value);
  }
}

u8 Bus::cpu_read(u16 address) const {
  u8 data = 0x00;
  if (_cartridge && _cartridge->cpu_read(address, data)) {
  } else if (address >= 0x0000 && address <= 0x1FFF) {
    data = _ram[address & 0x07FF];
  } else if (address >= 0x2000 && address <= 0x3FFF) {
    data = _ppu.cpu_read(address & 0x0007);
  }

  return data;
}
}  // namespace nes
