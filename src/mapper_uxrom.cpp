#include "mapper_uxrom.h"

namespace nes {
MapperUxROM::MapperUxROM(u8 prg_banks, u8 chr_banks)
  : Mapper(prg_banks, chr_banks) {}

bool MapperUxROM::cpu_read(u16 address, u32& mapped) const {
  if (address < 0x8000) return false;
  if (address < 0xC000) {
    mapped = _bank * 0x4000u + (address & 0x3FFF);  // switchable low bank
  } else {
    mapped = (_prg_banks - 1) * 0x4000u + (address & 0x3FFF);  // fixed last bank
  }
  return true;
}

bool MapperUxROM::cpu_write(u16 address, u8 value, u32& /*mapped*/) {
  if (address >= 0x8000) _bank = value & 0x0F;  // bank select (low nibble)
  return false;
}

bool MapperUxROM::ppu_read(u16 address, u32& mapped) const {
  if (address > 0x1FFF) return false;
  mapped = address;  // 8KB CHR-RAM, unbanked
  return true;
}

bool MapperUxROM::ppu_write(u16 address, u32& mapped) {
  if (address > 0x1FFF) return false;
  mapped = address;
  return true;
}
}  // namespace nes
