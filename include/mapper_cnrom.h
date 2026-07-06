#pragma once
#include "mapper.h"

namespace nes {

// CNROM (mapper 3): fixed PRG (NROM-style), with a switchable 8KB CHR-ROM bank
// selected by writes to $8000-$FFFF. Mirroring is fixed by the header.
class MapperCNROM : public Mapper {
 public:
  MapperCNROM(u8 prg_banks, u8 chr_banks);
  ~MapperCNROM() override = default;

  bool cpu_read(u16 address, u32& mapped) const override;
  bool cpu_write(u16 address, u8 value, u32& mapped) override;
  bool ppu_read(u16 address, u32& mapped) const override;
  bool ppu_write(u16 address, u32& mapped) override;

 private:
  u8 _chr_bank = 0;
};

}  // namespace nes
