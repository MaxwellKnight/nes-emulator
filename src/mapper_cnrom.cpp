#include "mapper_cnrom.h"

namespace nes {
MapperCNROM::MapperCNROM(u8 prg_banks, u8 chr_banks)
  : Mapper(prg_banks, chr_banks) {}

bool MapperCNROM::cpu_read(u16 address, u32& mapped) const {
  if (address < 0x8000) return false;
  mapped = address & (_prg_banks > 1 ? 0x7FFF : 0x3FFF);  // fixed PRG
  return true;
}

bool MapperCNROM::cpu_write(u16 address, u8 value, u32& /*mapped*/) {
  if (address >= 0x8000) _chr_bank = value & 0x03;  // select 8KB CHR bank
  return false;
}

bool MapperCNROM::ppu_read(u16 address, u32& mapped) const {
  if (address > 0x1FFF) return false;
  mapped = _chr_bank * 0x2000u + address;  // switchable 8KB CHR
  return true;
}

bool MapperCNROM::ppu_write(u16 /*address*/, u32& /*mapped*/) {
  return false;  // CHR is ROM
}
}  // namespace nes
