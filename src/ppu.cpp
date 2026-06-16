#include "ppu.h"
#include "palette.h"

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
    case 4:  // OAMDATA: read OAM at OAMADDR (no increment on read)
      result = oam_read();
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
    case 4:  // OAMDATA: write to OAM and post-increment OAMADDR
      oam_write(value);
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
  switch (mode) {
    case Cartridge::MirrorMode::VERTICAL:
      table = (addr >> 10) & 1;
      break;
    case Cartridge::MirrorMode::SINGLE_LO:
      table = 0;  // both logical nametables map to physical table 0
      break;
    case Cartridge::MirrorMode::SINGLE_HI:
      table = 1;
      break;
    case Cartridge::MirrorMode::HORIZONTAL:
    default:
      table = (addr >> 11) & 1;
      break;
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
void PPU::inc_coarse_x() {
  if ((_v & 0x001F) == 31) {
    _v &= ~0x001F;
    _v ^= 0x0400;
  } else {
    _v++;
  }
}

void PPU::inc_y() {
  if ((_v & 0x7000) != 0x7000) {
    _v += 0x1000;
  } else {
    _v &= ~0x7000;
    u16 y = (_v & 0x03E0) >> 5;
    if (y == 29) {
      y = 0;
      _v ^= 0x0800;
    } else if (y == 31) {
      y = 0;
    } else {
      y++;
    }
    _v = (_v & ~0x03E0) | (y << 5);
  }
}

void PPU::copy_x() { _v = (_v & ~0x041F) | (_t & 0x041F); }

void PPU::copy_y() { _v = (_v & ~0x7BE0) | (_t & 0x7BE0); }

// --- timing + rendering (implemented in C1/C2) ---
void PPU::clock() {
  // Advance the dot / scanline / frame counters first.
  _dot++;
  if (_dot == 341) {
    _dot = 0;
    _scanline++;
    if (_scanline == 262) {
      _scanline = 0;
      _frame++;
    }
  }

  bool rendering_enabled = (_mask & 0x18) != 0;

  // Pre-render line: clear vblank (and sprite flags) at dot 1.
  if (_scanline == 261 && _dot == 1) {
    _status &= ~0xE0;  // clear vblank + sprite 0 hit + overflow
  }

  // Start of vblank: set the flag and raise NMI when enabled.
  if (_scanline == 241 && _dot == 1) {
    _status |= 0x80;
    if (_ctrl & 0x80) _nmi_pending = true;
  }

  // Visible scanlines: render the whole line once it has been fetched.
  if (_scanline < 240 && _dot == 257) {
    render_scanline(_scanline);
  }

  // Loopy scroll updates (only while rendering is enabled).
  if (rendering_enabled) {
    if (_scanline < 240 || _scanline == 261) {
      if (_dot == 256) inc_y();
      if (_dot == 257) copy_x();
    }
    if (_scanline == 261 && _dot >= 280 && _dot <= 304) copy_y();

    // Pulse the mapper's scanline IRQ counter once per line (approximating the
    // PPU A12 rising edge that drives the MMC3 counter).
    if (_dot == 260 && (_scanline < 240 || _scanline == 261)) {
      if (_cartridge) _cartridge->signal_scanline();
    }
  }
}

// --- Scanline renderer (background + sprites) ------------------------------
void PPU::render_scanline(u16 line) {
  // Per-pixel background pixel value (0 = transparent/backdrop) for this line,
  // used by the sprite pass for priority and sprite-0 hit detection.
  u8 bg_pix[256] = {0};
  const u32 backdrop = palette_rgba(_palette[0] & 0x3F);

  if (_mask & 0x08) {  // PPUMASK d3: show background
    const bool show_left_bg = (_mask & 0x02) != 0;  // d1: show BG in leftmost 8px
    const u16 bg_base = (_ctrl & 0x10) ? 0x1000 : 0x0000;
    for (int x = 0; x < 256; x++) {
      // Fetch nametable tile id for the current coarse position.
      u8 tile = ppu_read(0x2000 | (_v & 0x0FFF));

      // Fetch attribute byte and extract the 2-bit palette for this quadrant.
      u8 attr = ppu_read(0x23C0 | (_v & 0x0C00) | ((_v >> 4) & 0x38) | ((_v >> 2) & 0x07));
      u8 palette_hi = ((attr >> (((_v >> 4) & 4) | (_v & 2))) & 3) << 2;

      // Fetch the two bitplanes for the current fine-Y row.
      u16 pat = bg_base | (static_cast<u16>(tile) << 4) | ((_v >> 12) & 7);
      u8 lo = ppu_read(pat);
      u8 hi = ppu_read(pat + 8);

      // Select the bit using fine-X within the tile.
      int bit = 7 - ((x + _x) & 7);
      u8 pixel2 = static_cast<u8>(((hi >> bit) & 1) << 1) | static_cast<u8>((lo >> bit) & 1);
      if (x < 8 && !show_left_bg) pixel2 = 0;  // mask leftmost 8px of background

      bg_pix[x] = pixel2;
      u8 color_index = (pixel2 == 0) ? _palette[0] : _palette[(palette_hi | pixel2) & 0x1F];
      _framebuffer[line * 256 + x] = palette_rgba(color_index & 0x3F);

      // Advance coarse-X every 8 rendered pixels (after the last px of a tile).
      if (((x + _x) & 7) == 7) inc_coarse_x();
    }
  } else {
    for (int x = 0; x < 256; x++) _framebuffer[line * 256 + x] = backdrop;
  }

  // Overlay sprites on top (handles 8x8/8x16, flips, priority, sprite-0 hit).
  if (_mask & 0x10) render_sprites(line, bg_pix);
}

// --- Per-scanline sprite evaluation + rendering ----------------------------
void PPU::render_sprites(u16 line, const u8* bg_pix) {
  const int sprite_height = (_ctrl & 0x20) ? 16 : 8;  // PPUCTRL d5
  const bool show_left_spr = (_mask & 0x04) != 0;     // PPUMASK d2

  // Sprite evaluation: collect up to 8 sprites whose Y range covers this line,
  // in OAM order. A 9th in-range sprite sets the overflow flag (PPUSTATUS d5).
  int found[8];
  int count = 0;
  for (int s = 0; s < 64; s++) {
    int sy = _oam[s * 4];
    int row = static_cast<int>(line) - sy - 1;  // sprites are delayed one scanline
    if (row < 0 || row >= sprite_height) continue;
    if (count < 8) {
      found[count++] = s;
    } else {
      _status |= 0x20;  // sprite overflow
      break;
    }
  }

  // Draw back-to-front so the lowest OAM index wins at each overlapping pixel.
  for (int i = count - 1; i >= 0; i--) {
    const int s = found[i];
    const int sy = _oam[s * 4];
    u8 tile = _oam[s * 4 + 1];
    const u8 attr = _oam[s * 4 + 2];
    const int sx = _oam[s * 4 + 3];

    int row = static_cast<int>(line) - sy - 1;  // sprites are delayed one scanline
    const bool flip_v = (attr & 0x80) != 0;
    const bool flip_h = (attr & 0x40) != 0;
    const bool behind = (attr & 0x20) != 0;  // priority: 1 = behind background
    const u8 pal_hi = (attr & 0x03) << 2;
    if (flip_v) row = sprite_height - 1 - row;

    u16 pat_addr;
    if (sprite_height == 16) {
      const u16 table = (tile & 0x01) ? 0x1000 : 0x0000;
      u8 t = tile & 0xFE;
      if (row >= 8) {  // bottom half uses the next tile
        t += 1;
        row -= 8;
      }
      pat_addr = table | (static_cast<u16>(t) << 4) | static_cast<u16>(row);
    } else {
      const u16 table = (_ctrl & 0x08) ? 0x1000 : 0x0000;
      pat_addr = table | (static_cast<u16>(tile) << 4) | static_cast<u16>(row);
    }
    const u8 lo = ppu_read(pat_addr);
    const u8 hi = ppu_read(pat_addr + 8);

    for (int col = 0; col < 8; col++) {
      const int x = sx + col;
      if (x >= 256) continue;
      if (x < 8 && !show_left_spr) continue;
      const int bit = flip_h ? col : (7 - col);
      const u8 pixel2 = static_cast<u8>(((hi >> bit) & 1) << 1) | static_cast<u8>((lo >> bit) & 1);
      if (pixel2 == 0) continue;  // transparent sprite pixel

      // Sprite-0 hit: set whenever an opaque sprite-0 pixel overlaps an opaque
      // background pixel (independent of priority); never at x==255.
      if (s == 0 && bg_pix[x] != 0 && x != 255) _status |= 0x40;

      // Priority: a "behind" sprite is hidden by opaque background.
      if (behind && bg_pix[x] != 0) continue;

      const u8 color_index = _palette[(0x10 + pal_hi + pixel2) & 0x1F];
      _framebuffer[line * 256 + x] = palette_rgba(color_index & 0x3F);
    }
  }
}

// --- OAM access ------------------------------------------------------------
void PPU::oam_write(u8 value) {
  _oam[_oam_addr] = value;
  _oam_addr++;  // wraps at 256
}

u8 PPU::oam_read() const { return _oam[_oam_addr]; }

const u8* PPU::oam_data() const { return _oam; }

// --- Pattern-table debug render -------------------------------------------
void PPU::render_pattern_table(int table, int palette, u32* out) const {
  u16 table_base = static_cast<u16>(table & 1) * 0x1000;
  for (int tile_y = 0; tile_y < 16; tile_y++) {
    for (int tile_x = 0; tile_x < 16; tile_x++) {
      int tile = tile_y * 16 + tile_x;
      u16 tile_base = table_base + static_cast<u16>(tile) * 16;
      for (int row = 0; row < 8; row++) {
        u8 lo = ppu_read(tile_base + row);
        u8 hi = ppu_read(tile_base + 8 + row);
        for (int col = 0; col < 8; col++) {
          int bit = 7 - col;
          u8 pixel2 = static_cast<u8>(((hi >> bit) & 1) << 1) | static_cast<u8>((lo >> bit) & 1);
          u8 color_index = _palette[((palette << 2) | pixel2) & 0x1F];
          int px = tile_x * 8 + col;
          int py = tile_y * 8 + row;
          out[py * 128 + px] = palette_rgba(color_index & 0x3F);
        }
      }
    }
  }
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
