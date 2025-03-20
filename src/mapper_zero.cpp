#pragma once
#include "mapper_zero.h"

namespace nes {
MapperZero::MapperZero(u8 prg_banks, u8 chr_banks)
  : Mapper(prg_banks, chr_banks) {}
bool MapperZero::cpu_read(u16 address, u32& mapped) const {
  if (address >= 0x8000 && address <= 0xFFFF) {
    mapped = address & (_prg_banks > 1 ? 0x7FFF : 0x3FFF);
    return true;
  }
  return false;
}
bool MapperZero::ppu_read(u16 address, u32& mapped) const {
  if (address >= 0x0000 && address <= 0x1FFF) {
    mapped = address;
    return true;
  }
  return false;
};
bool MapperZero::cpu_write(u16 address, u32& mapped) {
  if (address >= 0x8000 && address <= 0xFFFF) {
    mapped = address & (_prg_banks > 1 ? 0x7FFF : 0x3FFF);
    return true;
  }
  return false;
};
bool MapperZero::ppu_write(u16 address, u32& mapped) {
  // Should not write to chr ROM
  return false;
};
};  // namespace nes
