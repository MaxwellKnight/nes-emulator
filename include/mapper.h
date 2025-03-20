#include "types.h"

namespace nes {
class Mapper {
 protected:
  u8 _prg_banks = 0;
  u8 _chr_banks = 0;

 public:
  Mapper(u8 prg_banks, u8 chr_banks);
  ~Mapper() = default;

 public:
  virtual bool cpu_read(u16 address, u32& mapped) const = 0;
  virtual bool ppu_read(u16 address, u32& mapped) const = 0;
  virtual bool cpu_write(u16 address, u32& mapped) = 0;
  virtual bool ppu_write(u16 address, u32& mapped) = 0;
  virtual bool handles_address(u16 address) const = 0;
};
};  // namespace nes
