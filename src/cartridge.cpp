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
  // CHR-RAM: the mapper declines CHR writes, but RAM-backed CHR must accept
  // writes to the pattern-table region ($0000-$1FFF).
  if (_chr_is_ram && address <= 0x1FFF && address < _chr_memory.size()) {
    _chr_memory[address] = value;
    return true;
  }
  return false;
}

Cartridge::MirrorMode Cartridge::mirror_mode() const { return _mirror; }

std::shared_ptr<Cartridge> Cartridge::from_ines(const std::vector<u8>& bytes,
                                                int& out_status) {
  // Header is the first 16 bytes; validate magic "NES\x1A".
  if (bytes.size() < 16 || bytes[0] != 'N' || bytes[1] != 'E' ||
      bytes[2] != 'S' || bytes[3] != 0x1A) {
    out_status = 1;  // bad header
    return nullptr;
  }

  const u8 prg_banks = bytes[4];
  const u8 chr_banks = bytes[5];
  const u8 flags6 = bytes[6];
  const u8 flags7 = bytes[7];

  // Four-screen mirroring is unsupported in Phase 1.
  if (flags6 & 0x08) {
    out_status = 3;
    return nullptr;
  }

  // Mapper number must be 0 (NROM).
  const u8 mapper = (flags7 & 0xF0) | (flags6 >> 4);
  if (mapper != 0) {
    out_status = 2;
    return nullptr;
  }

  const size_t prg_size = (size_t)prg_banks * 16384;
  const size_t chr_size = (size_t)chr_banks * 8192;

  // Trainer (bit2) occupies 512 bytes before the PRG data.
  size_t offset = 16;
  if (flags6 & 0x04) offset += 512;

  // Validate the payload is large enough.
  if (bytes.size() < offset + prg_size + chr_size) {
    out_status = 1;  // truncated / bad header
    return nullptr;
  }

  auto cart = std::shared_ptr<Cartridge>(new Cartridge());
  cart->_mapper_id = mapper;
  cart->_prg_banks = prg_banks;
  cart->_chr_banks = chr_banks;
  cart->_mirror =
      (flags6 & 0x01) ? MirrorMode::VERTICAL : MirrorMode::HORIZONTAL;

  // Slice PRG.
  cart->_prg_memory.assign(bytes.begin() + offset,
                           bytes.begin() + offset + prg_size);
  offset += prg_size;

  // Slice CHR, or allocate 8KB CHR-RAM when no CHR-ROM is present.
  if (chr_size == 0) {
    cart->_chr_memory.assign(8192, 0);
    cart->_chr_is_ram = true;
  } else {
    cart->_chr_memory.assign(bytes.begin() + offset,
                             bytes.begin() + offset + chr_size);
    cart->_chr_is_ram = false;
  }

  cart->_mapper = std::make_shared<MapperZero>(cart->_prg_banks,
                                               cart->_chr_banks);
  out_status = 0;
  return cart;
}
};  // namespace nes
