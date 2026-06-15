#include "ppu.h"

namespace nes {

PPU::PPU() { reset(); }

void PPU::reset() {
  _ctrl = 0;
  _mask = 0;
  _status = 0;
  _oam_addr = 0;
  _data_buffer = 0;
  _v = 0;
  _t = 0;
  _x = 0;
  _w = 0;
  _scanline = 0;
  _dot = 0;
  _frame = 0;
  _nmi_pending = false;

  for (int t = 0; t < 2; t++)
    for (int i = 0; i < 1024; i++) _name[t][i] = 0;
  for (int i = 0; i < 32; i++) _palette[i] = 0;
  _framebuffer.fill(0);
}

void PPU::insert_cartridge(const std::shared_ptr<Cartridge>& c) { _cartridge = c; }

// --- register access (implemented in B2) ---
u8 PPU::cpu_read(u16 reg) const {
  u8 result = 0x00;
  switch (reg) {
    case 2:  // PPUSTATUS
      result = (_status & 0xE0) | (_data_buffer & 0x1F);
      _status &= ~0x80;  // clear vblank
      _w = 0;
      break;
    case 4:  // OAMDATA (Phase 2)
      result = 0x00;
      break;
    case 7: {  // PPUDATA
      if ((_v & 0x3FFF) < 0x3F00) {
        result = _data_buffer;
        _data_buffer = ppu_read(_v);
      } else {
        result = ppu_read(_v);
        _data_buffer = ppu_read(_v - 0x1000);
      }
      _v += (_ctrl & 0x04) ? 32 : 1;
      break;
    }
    default:
      break;  // $2000/$2001/$2003/$2005/$2006 are write-only (read 0)
  }
  return result;
}

void PPU::cpu_write(u16 reg, u8 value) {
  switch (reg) {
    case 0:  // PPUCTRL
      _ctrl = value;
      _t = (_t & 0xF3FF) | ((value & 0x03) << 10);
      break;
    case 1:  // PPUMASK
      _mask = value;
      break;
    case 3:  // OAMADDR
      _oam_addr = value;
      break;
    case 4:  // OAMDATA (Phase 2)
      break;
    case 5:  // PPUSCROLL
      if (_w == 0) {
        _x = value & 0x07;
        _t = (_t & 0xFFE0) | (value >> 3);
        _w = 1;
      } else {
        _t = (_t & 0x8FFF) | ((value & 0x07) << 12);
        _t = (_t & 0xFC1F) | ((value & 0xF8) << 2);
        _w = 0;
      }
      break;
    case 6:  // PPUADDR
      if (_w == 0) {
        _t = (_t & 0x80FF) | ((value & 0x3F) << 8);
        _w = 1;
      } else {
        _t = (_t & 0xFF00) | value;
        _v = _t;
        _w = 0;
      }
      break;
    case 7:  // PPUDATA
      ppu_write(_v, value);
      _v += (_ctrl & 0x04) ? 32 : 1;
      break;
    default:
      break;
  }
}

// --- PPU bus access (implemented in B3) ---
u16 PPU::nt_index(u16 addr, u16& offset) const {
  offset = addr & 0x03FF;
  Cartridge::MirrorMode mode =
      _cartridge ? _cartridge->mirror_mode() : Cartridge::MirrorMode::HORIZONTAL;
  u16 table;
  if (mode == Cartridge::MirrorMode::VERTICAL) {
    table = (addr >> 10) & 1;
  } else {  // HORIZONTAL
    table = (addr >> 11) & 1;
  }
  return table;
}

u8 PPU::ppu_read(u16 addr) const {
  addr &= 0x3FFF;
  if (addr <= 0x1FFF) {
    u8 data = 0x00;
    if (_cartridge) _cartridge->ppu_read(addr, data);
    return data;
  } else if (addr <= 0x3EFF) {
    u16 offset;
    u16 table = nt_index(addr, offset);
    return _name[table][offset];
  } else {
    u16 a = addr & 0x1F;
    if (a == 0x10 || a == 0x14 || a == 0x18 || a == 0x1C) a -= 0x10;
    u8 data = _palette[a];
    if (_mask & 0x01) data &= 0x30;  // greyscale
    return data;
  }
}

void PPU::ppu_write(u16 addr, u8 value) {
  addr &= 0x3FFF;
  if (addr <= 0x1FFF) {
    if (_cartridge) _cartridge->ppu_write(addr, value);
  } else if (addr <= 0x3EFF) {
    u16 offset;
    u16 table = nt_index(addr, offset);
    _name[table][offset] = value;
  } else {
    u16 a = addr & 0x1F;
    if (a == 0x10 || a == 0x14 || a == 0x18 || a == 0x1C) a -= 0x10;
    _palette[a] = value;
  }
}

// --- loopy helpers (implemented in B4) ---
void PPU::inc_coarse_x() {}
void PPU::inc_y() {}
void PPU::copy_x() {}
void PPU::copy_y() {}

// --- timing + rendering (implemented in C1/C2) ---
void PPU::clock() {}
void PPU::render_scanline(u16 line) { (void)line; }
void PPU::render_pattern_table(int table, int palette, u32* out) const {
  (void)table;
  (void)palette;
  (void)out;
}

bool PPU::take_nmi() {
  bool pending = _nmi_pending;
  _nmi_pending = false;
  return pending;
}

u32 PPU::frame_count() const { return _frame; }
u16 PPU::scanline() const { return _scanline; }
u16 PPU::dot() const { return _dot; }
const u32* PPU::framebuffer() const { return _framebuffer.data(); }

u8 PPU::reg_status() const { return _status; }
u8 PPU::reg_ctrl() const { return _ctrl; }
u8 PPU::reg_mask() const { return _mask; }
u16 PPU::vram_addr() const { return _v; }

const u8* PPU::nametable_ram() const { return &_name[0][0]; }
const u8* PPU::palette_ram() const { return &_palette[0]; }

};  // namespace nes
