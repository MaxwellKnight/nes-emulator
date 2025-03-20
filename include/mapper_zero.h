#pragma once
#include "mapper.h"

namespace nes {
class MapperZero : public Mapper {
 public:
  MapperZero(u8 prg_banks, u8 chr_banks);
  ~MapperZero() = default;

 public:
  bool cpu_read(u16 address, u32& mapped) const override;
  bool ppu_read(u16 address, u32& mapped) const override;
  bool cpu_write(u16 address, u32& mapped) override;
  bool ppu_write(u16 address, u32& mapped) override;
};
};  // namespace nes
