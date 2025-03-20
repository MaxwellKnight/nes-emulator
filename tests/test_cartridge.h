#pragma once
#include "cartridge.h"
#include "mapper_zero.h"

namespace nes {

class MockCartridge : public nes::Cartridge {
 public:
  MockCartridge()
    : Cartridge("") {  // Empty string instead of "test"
    // Explicitly initialize vectors if needed
    _prg_memory.resize(0x4000);  // Minimum PRG ROM size
    _chr_memory.resize(0x2000);  // Minimum CHR ROM size
    _mapper = std::make_shared<MapperZero>(_prg_banks, _chr_banks);
  }

  bool cpu_read(u16 address, u8& data) const {
    // Provide a predictable read behavior
    data = 0x00;
    return true;
  }

  bool cpu_write(u16 address, u8 value) { return true; }
};
};  // namespace nes
