#include "bus.h"

namespace nes {
Bus::Bus()
  : _sys_clock(0)
  , _cpu(*this)
  , _cartridge(nullptr) {}

void Bus::clock() {
  // While an OAM DMA is in progress the CPU is halted; the PPU keeps running.
  if (_dma_stall > 0) {
    _dma_stall--;
  } else {
    _cpu.clock();
  }
  _ppu.clock();
  _ppu.clock();
  _ppu.clock();
  if (_ppu.take_nmi()) {
    _cpu.trigger_nmi();
  }
  _sys_clock++;
}
void Bus::reset() {
  _sys_clock = 0;
  _cpu.reset();
  _ppu.reset();
}

CPU& Bus::get_cpu() { return _cpu; }
PPU& Bus::get_ppu() { return _ppu; }
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
  } else if (address == 0x4014) {
    // OAMDMA: copy 256 bytes from CPU page $XX00 into PPU OAM (through OAMADDR),
    // then stall the CPU ~513 cycles while the transfer "runs".
    u16 base = static_cast<u16>(value) << 8;
    for (int i = 0; i < 256; i++) {
      _ppu.oam_write(cpu_read(base + static_cast<u16>(i)));
    }
    _dma_stall += 513;
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
