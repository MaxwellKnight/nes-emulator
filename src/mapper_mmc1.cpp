#include "mapper_mmc1.h"

namespace nes {
MapperMMC1::MapperMMC1(u8 prg_banks, u8 chr_banks)
  : Mapper(prg_banks, chr_banks) {}

bool MapperMMC1::cpu_read(u16 address, u32& mapped) const {
  if (address < 0x8000) return false;
  const u8 prg_mode = (_control >> 2) & 0x03;
  if (prg_mode == 0 || prg_mode == 1) {
    // 32KB switch (low bit of the bank register ignored).
    const u32 bank = (_prg & 0x0E) >> 1;
    mapped = bank * 0x8000u + (address & 0x7FFF);
  } else if (prg_mode == 2) {
    // Fix first bank at $8000, switch 16KB at $C000.
    if (address < 0xC000) {
      mapped = address & 0x3FFF;
    } else {
      mapped = (_prg & 0x0F) * 0x4000u + (address & 0x3FFF);
    }
  } else {  // prg_mode == 3
    // Switch 16KB at $8000, fix last bank at $C000.
    if (address < 0xC000) {
      mapped = (_prg & 0x0F) * 0x4000u + (address & 0x3FFF);
    } else {
      mapped = (_prg_banks - 1) * 0x4000u + (address & 0x3FFF);
    }
  }
  return true;
}

bool MapperMMC1::cpu_write(u16 address, u8 value, u32& /*mapped*/) {
  if (address < 0x8000) return false;

  if (value & 0x80) {
    // Reset: clear the shift register and lock PRG mode 3.
    _shift = 0x10;
    _control |= 0x0C;
    return false;
  }

  const bool complete = (_shift & 0x01) != 0;  // 5th write fills the register
  _shift = static_cast<u8>((_shift >> 1) | ((value & 0x01) << 4));
  if (!complete) return false;

  const u8 reg = _shift & 0x1F;
  switch ((address >> 13) & 0x03) {  // which $2000-window the address falls in
    case 0: _control = reg; break;   // $8000-$9FFF
    case 1: _chr0 = reg; break;      // $A000-$BFFF
    case 2: _chr1 = reg; break;      // $C000-$DFFF
    default: _prg = reg; break;      // $E000-$FFFF
  }
  _shift = 0x10;  // reset for the next 5-write sequence
  return false;
}

bool MapperMMC1::ppu_read(u16 address, u32& mapped) const {
  if (address > 0x1FFF) return false;
  if (_control & 0x10) {  // 4KB CHR banks
    if (address < 0x1000) {
      mapped = _chr0 * 0x1000u + (address & 0x0FFF);
    } else {
      mapped = _chr1 * 0x1000u + (address & 0x0FFF);
    }
  } else {  // 8KB CHR bank (low bit ignored)
    mapped = (_chr0 & 0x1E) * 0x1000u + (address & 0x1FFF);
  }
  return true;
}

bool MapperMMC1::ppu_write(u16 address, u32& mapped) {
  // CHR-RAM uses the same banked mapping for writes.
  return ppu_read(address, mapped);
}

int MapperMMC1::mirror() const {
  // MMC1 control bits 0-1: 0 single-lo, 1 single-hi, 2 vertical, 3 horizontal.
  switch (_control & 0x03) {
    case 0: return 2;  // single-screen low
    case 1: return 3;  // single-screen high
    case 2: return 1;  // vertical
    default: return 0; // horizontal
  }
}
}  // namespace nes
