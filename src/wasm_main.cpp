#include <cstdint>
#include <memory>
#include <vector>
#include "../include/bus.h"
#include "../include/cartridge.h"
#include "../include/cpu.h"
#include "../include/debugger.h"
#include "../include/ppu.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

nes::Bus g_bus;
nes::Debugger g_debugger(g_bus.get_cpu(), g_bus);

#ifdef __EMSCRIPTEN__
// Main loop function that will be called from JavaScript
EMSCRIPTEN_KEEPALIVE extern "C" void main_loop() {
  if (g_debugger.is_running()) {
    g_debugger.step();
  }
}

// ---- Phase 1: ROM loading + PPU framebuffer/debug exports ----------------

// Build a cartridge from iNES bytes, insert it, and reset the CPU.
// Returns the from_ines status code (0 ok, 1 bad-header, 2 unsupported-mapper,
// 3 four-screen-unsupported).
EMSCRIPTEN_KEEPALIVE extern "C" int load_rom(const uint8_t* data, int len) {
  std::vector<nes::u8> bytes(data, data + len);
  int status = 0;
  auto cart = nes::Cartridge::from_ines(bytes, status);
  if (status != 0 || !cart) {
    return status;
  }
  g_bus.insert_cartridge(cart);
  g_bus.reset();
  // Boot the cartridge: load PC from the reset vector ($FFFC/$FFFD) so the ROM
  // starts at its own entry point. (reset() alone leaves PC at $FFFC, which on
  // a real ROM is the low byte of the vector and would execute as a stray BRK.)
  g_debugger.reset_to_vector();
  return 0;
}

// Pointer to the PPU's 256*240 RGBA framebuffer (read by JS as a HEAPU8 view).
EMSCRIPTEN_KEEPALIVE extern "C" uint8_t* get_framebuffer_ptr() {
  return reinterpret_cast<uint8_t*>(const_cast<nes::u32*>(g_bus.get_ppu().framebuffer()));
}

// Length of the framebuffer in bytes: 256 * 240 * 4.
EMSCRIPTEN_KEEPALIVE extern "C" int get_framebuffer_len() { return 256 * 240 * 4; }

// Current PPU frame counter.
EMSCRIPTEN_KEEPALIVE extern "C" uint32_t get_frame_count() {
  return static_cast<uint32_t>(g_bus.get_ppu().frame_count());
}

// Emulate up to the next VBlank, honoring breakpoints / BRK. Returns the stop
// reason (0 = frame completed, 1 = breakpoint hit, 2 = BRK) so the JS run loop
// knows whether to keep going or halt — discarding it made every frame look
// like a halt.
EMSCRIPTEN_KEEPALIVE extern "C" int run_frame() { return g_debugger.run_frame(); }

// Render a 128x128 RGBA pattern table (table 0/1, palette 0..7) into out.
EMSCRIPTEN_KEEPALIVE extern "C" void ppu_render_pattern_table(int table, int palette, uint8_t* out) {
  g_bus.get_ppu().render_pattern_table(table, palette, reinterpret_cast<nes::u32*>(out));
}

// Pointer to the 2048-byte nametable VRAM.
EMSCRIPTEN_KEEPALIVE extern "C" uint8_t* get_nametable_ptr() {
  return const_cast<uint8_t*>(g_bus.get_ppu().nametable_ram());
}

// Pointer to the 32-byte palette RAM.
EMSCRIPTEN_KEEPALIVE extern "C" uint8_t* get_palette_ram_ptr() {
  return const_cast<uint8_t*>(g_bus.get_ppu().palette_ram());
}

// Pointer to the 256-byte OAM (64 sprites x 4: Y, tile, attr, X).
EMSCRIPTEN_KEEPALIVE extern "C" uint8_t* get_oam_ptr() {
  return const_cast<uint8_t*>(g_bus.get_ppu().oam_data());
}

// Latch controller buttons. port 0 = player 1, 1 = player 2. Bit layout:
// d0 A, d1 B, d2 Select, d3 Start, d4 Up, d5 Down, d6 Left, d7 Right.
EMSCRIPTEN_KEEPALIVE extern "C" void set_controller(int port, int buttons) {
  g_bus.set_controller(port, static_cast<nes::u8>(buttons));
}

// PPU register / scanline debug getters (no side effects).
EMSCRIPTEN_KEEPALIVE extern "C" uint8_t ppu_get_ctrl() { return g_bus.get_ppu().reg_ctrl(); }
EMSCRIPTEN_KEEPALIVE extern "C" uint8_t ppu_get_mask() { return g_bus.get_ppu().reg_mask(); }
EMSCRIPTEN_KEEPALIVE extern "C" uint8_t ppu_get_status() { return g_bus.get_ppu().reg_status(); }
EMSCRIPTEN_KEEPALIVE extern "C" int ppu_get_scanline() { return static_cast<int>(g_bus.get_ppu().scanline()); }
#endif
