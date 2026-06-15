#pragma once
#include "mapper.h"

namespace nes {

// MMC1 (mapper 1): a 5-bit serial shift register loads one of four internal
// registers (control, CHR bank 0/1, PRG bank). Supports switchable 16/32KB PRG,
// 4/8KB CHR, and software-controlled mirroring.
class MapperMMC1 : public Mapper {
 public:
  MapperMMC1(u8 prg_banks, u8 chr_banks);
  ~MapperMMC1() override = default;

  bool cpu_read(u16 address, u32& mapped) const override;
  bool cpu_write(u16 address, u8 value, u32& mapped) override;
  bool ppu_read(u16 address, u32& mapped) const override;
  bool ppu_write(u16 address, u32& mapped) override;
  int mirror() const override;

 private:
  u8 _shift = 0x10;   // serial load register (bit4 marks 5th write)
  u8 _control = 0x0C; // power-on: PRG mode 3 (fix last bank at $C000)
  u8 _chr0 = 0;
  u8 _chr1 = 0;
  u8 _prg = 0;
};

}  // namespace nes
