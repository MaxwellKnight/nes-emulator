#pragma once
#include "mapper.h"

namespace nes {

// MMC3 (mapper 4): eight bank registers driving 8KB PRG and 1/2KB CHR windows,
// software mirroring, and a scanline IRQ counter clocked once per rendered
// line. PRG window modes and CHR A12 inversion are selectable.
class MapperMMC3 : public Mapper {
 public:
  MapperMMC3(u8 prg_banks, u8 chr_banks);
  ~MapperMMC3() override = default;

  bool cpu_read(u16 address, u32& mapped) const override;
  bool cpu_write(u16 address, u8 value, u32& mapped) override;
  bool ppu_read(u16 address, u32& mapped) const override;
  bool ppu_write(u16 address, u32& mapped) override;
  int mirror() const override;

  void scanline() override;
  bool irq_pending() const override;
  void irq_clear() override;

 private:
  u32 chr_bank_1k(int region) const;  // resolve a 1KB CHR region (0..7)

  u8 _bank_select = 0;  // $8000: target reg + PRG/CHR modes
  u8 _regs[8] = {0};    // R0..R7
  int _mirror = 0;      // 0 vertical, 1 horizontal
  u8 _irq_latch = 0;
  u8 _irq_counter = 0;
  bool _irq_reload = false;
  bool _irq_enabled = false;
  bool _irq_pending = false;
};

}  // namespace nes
