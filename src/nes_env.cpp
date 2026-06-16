// nes_env.cpp - implementation of the C ABI in nes_env.h.
#include "nes_env.h"

#include <cstring>
#include <memory>
#include <vector>

#include "bus.h"
#include "cartridge.h"
#include "debugger.h"

// One handle = one independent machine. Bus is declared first so the Debugger's
// references into it are valid, and the whole thing lives on the heap so those
// references never dangle.
struct NesEnv {
  nes::Bus bus;
  nes::Debugger dbg;
  std::shared_ptr<nes::Cartridge> cart;
  std::vector<uint8_t> rom;
  NesEnv() : bus(), dbg(bus.get_cpu(), bus) {}
};

extern "C" {

NES_API NesEnv* nes_create(void) {
  try {
    return new NesEnv();
  } catch (...) {
    return nullptr;
  }
}

NES_API void nes_destroy(NesEnv* e) { delete e; }

NES_API int nes_load(NesEnv* e, const uint8_t* rom, int len) {
  if (!e || !rom || len <= 0) return 1;
  try {
    e->rom.assign(rom, rom + len);
    int status = 0;
    e->cart = nes::Cartridge::from_ines(e->rom, status);
    if (status != 0) return status;
    e->bus.insert_cartridge(e->cart);
    e->bus.reset();
    e->dbg.reset_to_vector();
    return 0;
  } catch (...) {
    return 1;
  }
}

NES_API void nes_reset(NesEnv* e) {
  if (!e || e->rom.empty()) return;
  try {
    // Rebuild the cartridge so PRG-RAM, CHR-RAM, and mapper bank state start
    // clean; bus.reset() clears the CPU/PPU/APU/controllers.
    int status = 0;
    e->cart = nes::Cartridge::from_ines(e->rom, status);
    if (status != 0) return;
    e->bus.insert_cartridge(e->cart);
    e->bus.reset();
    e->dbg.reset_to_vector();
  } catch (...) {
  }
}

NES_API int nes_step(NesEnv* e, uint8_t p1_buttons) {
  if (!e) return -1;
  try {
    e->bus.set_controller(0, p1_buttons);
    return e->dbg.run_frame();
  } catch (...) {
    return -1;
  }
}

NES_API void nes_set_controller(NesEnv* e, int port, uint8_t buttons) {
  if (!e) return;
  e->bus.set_controller(port, buttons);
}

NES_API const uint8_t* nes_framebuffer(NesEnv* e) {
  if (!e) return nullptr;
  return reinterpret_cast<const uint8_t*>(e->bus.get_ppu().framebuffer());
}

NES_API int nes_framebuffer_size(NesEnv* e) {
  (void)e;
  return 256 * 240 * 4;
}

NES_API void nes_get_ram(NesEnv* e, uint8_t* out_2048) {
  if (!e || !out_2048) return;
  for (int i = 0; i < 0x0800; i++) {
    out_2048[i] = e->bus.cpu_read(static_cast<uint16_t>(i));
  }
}

NES_API uint8_t nes_peek(NesEnv* e, uint16_t addr) {
  if (!e) return 0;
  return e->bus.cpu_read(addr);
}

NES_API uint32_t nes_frame_count(NesEnv* e) {
  if (!e) return 0;
  return e->bus.get_ppu().frame_count();
}

NES_API const char* nes_version(void) { return "nes_env 1"; }

}  // extern "C"
