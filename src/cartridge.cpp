#include "cartridge.h"
#include <fstream>

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

  ifs.close();
}
u8 Cartridge::cpu_read(u16 address) const {
  u8 data = 0x00;
  return data;
}
u8 Cartridge::ppu_read(u16 address) const { return 0x00; }
void Cartridge::cpu_write(u16 address, u8 value) {}
void Cartridge::ppu_write(u16 address, u8 value) {}
bool Cartridge::handles_address(u16 address) const {
  if (address >= 0x8000 && address <= 0xFFFF) {
    return true;
  }
  return false;
}
};  // namespace nes
