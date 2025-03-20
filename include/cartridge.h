#include <string>
#include <vector>
#include "types.h"

namespace nes {
class Cartridge : Addressable {
 private:
  std::vector<u8> _prg_memory;
  std::vector<u8> _chr_memory;

  u8 _mapper_id = 0;
  u8 _prg_banks = 0;
  u8 _chr_banks = 0;

 public:
  Cartridge(const std::string& file);
  ~Cartridge() = default;

 public:
  u8 cpu_read(u16 address) const override;
  u8 ppu_read(u16 address) const override;
  void cpu_write(u16 address, u8 value) override;
  void ppu_write(u16 address, u8 value) override;
  bool handles_address(u16 address) const override;
};
};  // namespace nes
