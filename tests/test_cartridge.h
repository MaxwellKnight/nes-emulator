#pragma once
#include <array>
#include "cartridge.h"
#include "mapper_zero.h"

namespace nes {

// Test double. CPU-space IO is backed by a flat 64 KB memory so unit tests can
// poke a program/data anywhere via the bus and read it straight back (the Bus
// consults the cartridge first for every address). PPU-space reads fall through
// to the base Cartridge/MapperZero so CHR seeded into _chr_memory is visible to
// the PPU. The base Cartridge IO is virtual, so these override correctly.
class MockCartridge : public nes::Cartridge {
 public:
  MockCartridge()
    : Cartridge("") {
    _prg_memory.resize(0x4000);  // Minimum PRG ROM size
    _chr_memory.resize(0x2000);  // Minimum CHR ROM size
    _mapper = std::make_shared<MapperZero>(_prg_banks, _chr_banks);
    _mem.fill(0);
  }

  bool cpu_read(u16 address, u8& data) const override {
    if (address >= 0x8000) {
      data = _mem[address];
      return true;
    }
    return false;
  }

  bool cpu_write(u16 address, u8 value) override {
    if (address >= 0x8000) {
      _mem[address] = value;
      return true;
    }
    return false;
  }

 private:
  std::array<u8, 0x10000> _mem{};
};
};  // namespace nes
