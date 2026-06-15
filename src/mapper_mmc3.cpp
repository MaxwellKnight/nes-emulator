#include "mapper_mmc3.h"

namespace nes {
MapperMMC3::MapperMMC3(u8 prg_banks, u8 chr_banks)
  : Mapper(prg_banks, chr_banks) {}

bool MapperMMC3::cpu_read(u16 address, u32& mapped) const {
  if (address < 0x8000) return false;
  const int total_8k = _prg_banks * 2;  // 8KB PRG bank count
  const int last = total_8k - 1;
  const int second_last = total_8k - 2;
  const int region = (address - 0x8000) / 0x2000;  // 0..3
  const bool prg_mode = (_bank_select & 0x40) != 0;

  int bank;
  if (!prg_mode) {  // $8000 swappable, $C000 fixed to second-last
    const int map[4] = {_regs[6], _regs[7], second_last, last};
    bank = map[region];
  } else {  // $C000 swappable, $8000 fixed to second-last
    const int map[4] = {second_last, _regs[7], _regs[6], last};
    bank = map[region];
  }
  bank %= (total_8k > 0 ? total_8k : 1);
  mapped = static_cast<u32>(bank) * 0x2000u + (address & 0x1FFF);
  return true;
}

bool MapperMMC3::cpu_write(u16 address, u8 value, u32& /*mapped*/) {
  if (address < 0x8000) return false;
  const bool odd = (address & 0x01) != 0;
  if (address <= 0x9FFF) {
    if (!odd) {
      _bank_select = value;  // target register + mode bits
    } else {
      _regs[_bank_select & 0x07] = value;  // bank data
    }
  } else if (address <= 0xBFFF) {
    if (!odd) {
      _mirror = value & 0x01;  // 0 vertical, 1 horizontal
    }
    // odd ($A001) = PRG-RAM protect: not emulated (RAM always enabled).
  } else if (address <= 0xDFFF) {
    if (!odd) {
      _irq_latch = value;  // IRQ reload value
    } else {
      _irq_counter = 0;
      _irq_reload = true;  // force reload on next clock
    }
  } else {  // $E000-$FFFF
    if (!odd) {
      _irq_enabled = false;
      _irq_pending = false;  // disable + acknowledge
    } else {
      _irq_enabled = true;
    }
  }
  return false;
}

u32 MapperMMC3::chr_bank_1k(int region) const {
  const bool inv = (_bank_select & 0x80) != 0;  // CHR A12 inversion
  const u8 r0 = _regs[0] & 0xFE, r1 = _regs[1] & 0xFE;
  // region order without inversion: [R0,R0+1, R1,R1+1, R2,R3,R4,R5]
  const int normal[8] = {r0, r0 + 1, r1, r1 + 1,
                         _regs[2], _regs[3], _regs[4], _regs[5]};
  // with inversion the 1KB banks move to $0000 and the 2KB banks to $1000.
  const int inverted[8] = {_regs[2], _regs[3], _regs[4], _regs[5],
                           r0, r0 + 1, r1, r1 + 1};
  return static_cast<u32>((inv ? inverted : normal)[region]);
}

bool MapperMMC3::ppu_read(u16 address, u32& mapped) const {
  if (address > 0x1FFF) return false;
  const int region = address / 0x0400;  // 1KB region 0..7
  mapped = chr_bank_1k(region) * 0x0400u + (address & 0x03FF);
  return true;
}

bool MapperMMC3::ppu_write(u16 address, u32& mapped) {
  // CHR-RAM carts reuse the banked mapping for writes.
  return ppu_read(address, mapped);
}

int MapperMMC3::mirror() const {
  return _mirror == 0 ? 1 /*vertical*/ : 0 /*horizontal*/;
}

void MapperMMC3::scanline() {
  if (_irq_counter == 0 || _irq_reload) {
    _irq_counter = _irq_latch;
    _irq_reload = false;
  } else {
    _irq_counter--;
  }
  if (_irq_counter == 0 && _irq_enabled) _irq_pending = true;
}

bool MapperMMC3::irq_pending() const { return _irq_pending; }
void MapperMMC3::irq_clear() { _irq_pending = false; }
}  // namespace nes
