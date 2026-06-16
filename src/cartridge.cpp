#include "cartridge.h"
#include <fstream>
#include <memory>
#include "mapper_cnrom.h"
#include "mapper_mmc1.h"
#include "mapper_mmc3.h"
#include "mapper_uxrom.h"
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
      _mirror = (header.mapper1 & 0x01) ? MirrorMode::VERTICAL
                                        : MirrorMode::HORIZONTAL;

      _prg_banks = header.prg_rom_chunks;
      _prg_memory.resize(_prg_banks * 16384);
      ifs.read((char*)_prg_memory.data(), _prg_memory.size());

      _chr_banks = header.chr_rom_chunks;
      if (_chr_banks == 0) {  // CHR-RAM
        _chr_memory.assign(8192, 0);
        _chr_is_ram = true;
      } else {
        _chr_memory.resize(_chr_banks * 8192);
        ifs.read((char*)_chr_memory.data(), _chr_memory.size());
      }
    }

    if (file_type == 2) {
    }
  }

  // Provide 8KB of work/save RAM at $6000-$7FFF. MMC1/MMC3 boards always carry
  // it; many NROM boards (and most blargg/conformance test ROMs) expect it too,
  // so we back the whole region unconditionally — harmless for carts that ignore it.
  _prg_ram.assign(8192, 0);

  // Load the appropriate mapper (defaults to NROM for unknown ids).
  switch (_mapper_id) {
    case 1: _mapper = std::make_shared<MapperMMC1>(_prg_banks, _chr_banks); break;
    case 2: _mapper = std::make_shared<MapperUxROM>(_prg_banks, _chr_banks); break;
    case 3: _mapper = std::make_shared<MapperCNROM>(_prg_banks, _chr_banks); break;
    case 4: _mapper = std::make_shared<MapperMMC3>(_prg_banks, _chr_banks); break;
    default: _mapper = std::make_shared<MapperZero>(_prg_banks, _chr_banks); break;
  }

  ifs.close();
}

bool Cartridge::cpu_read(u16 address, u8& data) const {
  // PRG-RAM / save RAM at $6000-$7FFF for mappers that provide it.
  if (address >= 0x6000 && address <= 0x7FFF && !_prg_ram.empty()) {
    data = _prg_ram[address & 0x1FFF];
    return true;
  }
  u32 mapped_addr = 0x00;
  if (_mapper->cpu_read(address, mapped_addr) && mapped_addr < _prg_memory.size()) {
    data = _prg_memory[mapped_addr];
    return true;
  }
  return false;
}

bool Cartridge::ppu_read(u16 address, u8& data) const {
  u32 mapped_addr = 0x00;
  if (_mapper->ppu_read(address, mapped_addr) && mapped_addr < _chr_memory.size()) {
    data = _chr_memory[mapped_addr];
    return true;
  }
  return false;
}

bool Cartridge::cpu_write(u16 address, u8 value) {
  // PRG-RAM / save RAM at $6000-$7FFF.
  if (address >= 0x6000 && address <= 0x7FFF && !_prg_ram.empty()) {
    _prg_ram[address & 0x1FFF] = value;
    return true;
  }
  // $8000-$FFFF writes are usually mapper-register writes (the mapper updates
  // bank state and returns false); only writable PRG-ROM returns a mapped offset.
  u32 mapped_addr = 0x00;
  if (_mapper->cpu_write(address, value, mapped_addr)) {
    if (mapped_addr < _prg_memory.size()) _prg_memory[mapped_addr] = value;
    return true;
  }
  return false;
}

bool Cartridge::ppu_write(u16 address, u8 value) {
  // Only RAM-backed CHR is writable; use the mapper's banked offset.
  if (_chr_is_ram && address <= 0x1FFF) {
    u32 mapped_addr = address;
    if (_mapper->ppu_read(address, mapped_addr) &&
        mapped_addr < _chr_memory.size()) {
      _chr_memory[mapped_addr] = value;
      return true;
    }
  }
  return false;
}

Cartridge::MirrorMode Cartridge::mirror_mode() const {
  switch (_mapper ? _mapper->mirror() : -1) {
    case 0: return MirrorMode::HORIZONTAL;
    case 1: return MirrorMode::VERTICAL;
    case 2: return MirrorMode::SINGLE_LO;
    case 3: return MirrorMode::SINGLE_HI;
    default: return _mirror;  // -1: use the iNES header value
  }
}

void Cartridge::signal_scanline() {
  if (_mapper) _mapper->scanline();
}
bool Cartridge::irq_pending() const {
  return _mapper && _mapper->irq_pending();
}
void Cartridge::irq_clear() {
  if (_mapper) _mapper->irq_clear();
}

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

  // Supported mappers: 0 NROM, 1 MMC1, 2 UxROM, 3 CNROM, 4 MMC3.
  const u8 mapper = (flags7 & 0xF0) | (flags6 >> 4);
  if (mapper != 0 && mapper != 1 && mapper != 2 && mapper != 3 && mapper != 4) {
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

  // Back $6000-$7FFF with 8KB of work/save RAM unconditionally (see the file
  // constructor above): MMC1/MMC3 always have it and NROM conformance ROMs use it.
  cart->_prg_ram.assign(8192, 0);

  switch (mapper) {
    case 1:
      cart->_mapper = std::make_shared<MapperMMC1>(prg_banks, chr_banks);
      break;
    case 2:
      cart->_mapper = std::make_shared<MapperUxROM>(prg_banks, chr_banks);
      break;
    case 3:
      cart->_mapper = std::make_shared<MapperCNROM>(prg_banks, chr_banks);
      break;
    case 4:
      cart->_mapper = std::make_shared<MapperMMC3>(prg_banks, chr_banks);
      break;
    default:
      cart->_mapper = std::make_shared<MapperZero>(prg_banks, chr_banks);
      break;
  }
  out_status = 0;
  return cart;
}
};  // namespace nes
