#pragma once
#include <array>
#include <memory>
#include "cartridge.h"
#include "types.h"

namespace nes {
class PPU {
 public:
  PPU();
  ~PPU() = default;

 public:
  void insert_cartridge(const std::shared_ptr<Cartridge>& c);
  void reset();

  u8 cpu_read(u16 reg) const;  // reg already masked to 0..7 by the Bus
  void cpu_write(u16 reg, u8 value);
  u8 ppu_read(u16 addr) const;
  void ppu_write(u16 addr, u8 value);

  void clock();        // advance ONE dot
  bool take_nmi();     // returns _nmi_pending and clears it

  u32 frame_count() const;
  u16 scanline() const;
  u16 dot() const;
  const u32* framebuffer() const;  // 256*240 RGBA

  u8 reg_status() const;
  u8 reg_ctrl() const;
  u8 reg_mask() const;
  u16 vram_addr() const;

  void render_pattern_table(int table, int palette, u32* out) const;  // out = 128*128 RGBA
  const u8* nametable_ram() const;  // 2048 bytes (&_name[0][0])
  const u8* palette_ram() const;    // 32 bytes

 private:
  u16 nt_index(u16 addr, u16& offset) const;  // returns table; sets offset
  void render_scanline(u16 line);

  // loopy scroll helpers
  void inc_coarse_x();
  void inc_y();
  void copy_x();
  void copy_y();

 private:
  std::shared_ptr<Cartridge> _cartridge;

  u8 _name[2][1024];  // nametable VRAM
  u8 _palette[32];    // palette RAM

  u8 _ctrl = 0;
  u8 _mask = 0;
  mutable u8 _status = 0;
  u8 _oam_addr = 0;
  mutable u8 _data_buffer = 0;

  mutable u16 _v = 0;  // current VRAM address (15 bit)
  u16 _t = 0;          // temp VRAM address (15 bit)
  u8 _x = 0;           // fine X scroll (3 bit)
  mutable u8 _w = 0;   // write toggle

  u16 _scanline = 0;
  u16 _dot = 0;
  u32 _frame = 0;
  bool _nmi_pending = false;

  std::array<u32, 256 * 240> _framebuffer{};

  friend class PPUTestLoopy;
};
};  // namespace nes
