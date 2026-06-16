#pragma once
#include "types.h"

namespace nes {

// Address-translation + bank-switching interface implemented per cartridge
// board. Reads return a flat offset into the cartridge's PRG/CHR memory; writes
// to $8000-$FFFF are usually mapper-register writes that update bank state and
// return false (no PRG-ROM byte is written).
class Mapper {
 protected:
  u8 _prg_banks = 0;  // 16KB PRG-ROM banks
  u8 _chr_banks = 0;  // 8KB CHR-ROM banks (0 => CHR-RAM)

 public:
  Mapper(u8 prg_banks, u8 chr_banks);
  virtual ~Mapper() = default;

  // Map a CPU address to a PRG-ROM offset. Returns false if not handled.
  virtual bool cpu_read(u16 address, u32& mapped) const = 0;
  // Handle a CPU write. Register writes update bank state and return false;
  // returns true (+offset) only when the write targets writable PRG-ROM.
  virtual bool cpu_write(u16 address, u8 value, u32& mapped) = 0;
  // Map a PPU address ($0000-$1FFF) to a CHR offset.
  virtual bool ppu_read(u16 address, u32& mapped) const = 0;
  virtual bool ppu_write(u16 address, u32& mapped) = 0;

  // Dynamic nametable mirroring: -1 = use the iNES header value; otherwise
  // 0 horizontal, 1 vertical, 2 single-screen low, 3 single-screen high.
  virtual int mirror() const { return -1; }

  // Scanline IRQ (MMC3). Other mappers leave these as no-ops.
  virtual void scanline() {}  // pulse once per rendered scanline
  virtual bool irq_pending() const { return false; }
  virtual void irq_clear() {}
};

}  // namespace nes
