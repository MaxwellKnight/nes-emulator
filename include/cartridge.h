#include <memory>
#include <string>
#include <vector>
#include "mapper.h"
#include "types.h"

namespace nes {
class Cartridge {
 protected:
  std::vector<u8> _prg_memory;
  std::vector<u8> _chr_memory;
  std::shared_ptr<Mapper> _mapper;

  u8 _mapper_id = 0;
  u8 _prg_banks = 0;
  u8 _chr_banks = 0;

 public:
  Cartridge(const std::string& file);
  ~Cartridge() = default;

 public:
  bool cpu_read(u16 address, u8& data) const;
  bool ppu_read(u16 address, u8& data) const;
  bool cpu_write(u16 address, u8 value);
  bool ppu_write(u16 address, u8 value);
};
};  // namespace nes
