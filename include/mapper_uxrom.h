#pragma once
#include "mapper.h"

namespace nes {

// UxROM (mapper 2): a single switchable 16KB PRG bank at $8000 with the last
// bank fixed at $C000. CHR is 8KB RAM. Mirroring is fixed by the header.
class MapperUxROM : public Mapper {
 public:
  MapperUxROM(u8 prg_banks, u8 chr_banks);
  ~MapperUxROM() override = default;

  bool cpu_read(u16 address, u32& mapped) const override;
  bool cpu_write(u16 address, u8 value, u32& mapped) override;
  bool ppu_read(u16 address, u32& mapped) const override;
  bool ppu_write(u16 address, u32& mapped) override;

 private:
  u8 _bank = 0;
};

}  // namespace nes
