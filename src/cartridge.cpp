#include "cartridge.h"
#include <fstream>
#include <memory>
#include "mapper_zero.h"

namespace nes {
Cartridge::Cartridge(const std::string& file) {
  struct Header {
    char name[4];
    u8 prg_rom_chunks;
    u8 chr_rom_chunks;
    u8 mapper1;
    u8 mapper2;
    u8 tv_system1;
    u8 tv_system2;
    char unused[5];
  } header;

  std::ifstream ifs;
  ifs.open(file, std::ifstream::binary);
  if (ifs.is_open()) {
    // Reading the file header
    ifs.read((char*)&header, sizeof(Header));

    if (header.mapper1 & 0x40) ifs.seekg(512, std::ios_base::cur);

    // Extracting the mapper id used
    _mapper_id = ((header.mapper2 >> 4) << 4) | (header.mapper1 >> 4);

    // TODO: find the file format
    u8 file_type = 1;
    if (file_type == 0) {
    }

    if (file_type == 1) {
      _prg_banks = header.prg_rom_chunks;
      _prg_memory.resize(_prg_banks * 16384);
      ifs.read((char*)_prg_memory.data(), _prg_memory.size());

      _chr_banks = header.chr_rom_chunks;
      _chr_memory.resize(_chr_banks * 8192);
      ifs.read((char*)_chr_memory.data(), _chr_memory.size());
    }

    if (file_type == 2) {
    }
  }

  // Load the appropriate mapper
  switch (_mapper_id) {
    case 0:
      _mapper = std::make_shared<MapperZero>(_prg_banks, _chr_banks);
      break;
  }

  ifs.close();
}

bool Cartridge::cpu_read(u16 address, u8& data) const {
  u32 mapped_addr = 0x00;
  if (_mapper->cpu_read(address, mapped_addr)) {
    data = _prg_memory[mapped_addr];
    return true;
  }
  return false;
}

bool Cartridge::ppu_read(u16 address, u8& data) const {
  u32 mapped_addr = 0x00;
  if (_mapper->ppu_read(address, mapped_addr)) {
    data = _chr_memory[mapped_addr];
    return true;
  }
  return false;
}

bool Cartridge::cpu_write(u16 address, u8 value) {
  u32 mapped_addr = 0x00;
  if (_mapper->cpu_write(address, mapped_addr)) {
    _prg_memory[mapped_addr] = value;
    return true;
  }
  return false;
}

bool Cartridge::ppu_write(u16 address, u8 value) {
  u32 mapped_addr = 0x00;
  if (_mapper->ppu_write(address, mapped_addr)) {
    _chr_memory[mapped_addr] = value;
    return true;
  }
  return false;
}
};  // namespace nes
