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
  (void)reg;
  return 0;
}
void PPU::cpu_write(u16 reg, u8 value) {
  (void)reg;
  (void)value;
}

// --- PPU bus access (implemented in B3) ---
u16 PPU::nt_index(u16 addr, u16& offset) const {
  offset = addr & 0x03FF;
  return 0;
}
u8 PPU::ppu_read(u16 addr) const {
  (void)addr;
  return 0;
}
void PPU::ppu_write(u16 addr, u8 value) {
  (void)addr;
  (void)value;
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
