#pragma once
#include <memory>
#include <string>
#include <vector>
#include "mapper.h"
#include "types.h"

namespace nes {
class Cartridge {
 public:
  enum class MirrorMode { HORIZONTAL, VERTICAL };

  std::vector<u8> _prg_memory;
  std::vector<u8> _chr_memory;
  std::shared_ptr<Mapper> _mapper;

  u8 _mapper_id = 0;
  u8 _prg_banks = 0;
  u8 _chr_banks = 0;

 private:
  MirrorMode _mirror = MirrorMode::HORIZONTAL;
  bool _chr_is_ram = false;
  Cartridge() = default;  // used by from_ines

 public:
  Cartridge(const std::string& file);
  virtual ~Cartridge() = default;

  // In-memory iNES factory (mapper 0). out_status: 0 ok, 1 bad-header,
  // 2 unsupported-mapper, 3 four-screen-unsupported. Returns nullptr on error.
  static std::shared_ptr<Cartridge> from_ines(const std::vector<u8>& bytes,
                                              int& out_status);

  MirrorMode mirror_mode() const;

  virtual bool cpu_read(u16 address, u8& data) const;
  virtual bool ppu_read(u16 address, u8& data) const;
  virtual bool cpu_write(u16 address, u8 value);
  virtual bool ppu_write(u16 address, u8 value);
};
};  // namespace nes
