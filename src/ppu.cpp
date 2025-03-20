#include "ppu.h"

namespace nes {
PPU::PPU() {}
u8 PPU::cpu_read(u16 address) const {
  u8 data = 0x00;
  switch (address) {
    case 0x0000:  // Control
      break;
    case 0x0001:  // Mask
      break;
    case 0x0002:  // Status
      break;
    case 0x0003:  // OAM Address
      break;
    case 0x0004:  // OAM Data
      break;
    case 0x0005:  // Scroll
      break;
    case 0x0006:  // PPU Address
      break;
    case 0x0007:  // PPU Data
      break;
  }

  return data;
}
void PPU::cpu_write(u16 address, u8 value) {
  switch (address) {
    case 0x0000:  // Control
      break;
    case 0x0001:  // Mask
      break;
    case 0x0002:  // Status
      break;
    case 0x0003:  // OAM Address
      break;
    case 0x0004:  // OAM Data
      break;
    case 0x0005:  // Scroll
      break;
    case 0x0006:  // PPU Address
      break;
    case 0x0007:  // PPU Data
      break;
  }
}

u8 PPU::ppu_read(u16 address) const {
  u8 data = 0x00;
  address &= 0x3FFF;
  if (_cartridge->ppu_read(address, data)) {
  }

  return data;
}
void PPU::ppu_write(u16 address, u8 value) {
  address &= 0x3FFF;
  if (_cartridge->cpu_write(address, value)) {
  }
}

void PPU::insert_cartridge(const std::shared_ptr<Cartridge>& cartridge) { _cartridge = cartridge; }
bool PPU::handles_address(u16 address) const { return true; }
};  // namespace nes
