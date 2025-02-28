#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/debugger.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

nes::Bus g_bus;
nes::CPU g_cpu(g_bus);
nes::Debugger g_debugger(g_cpu, g_bus);

#ifdef __EMSCRIPTEN__
// This function will be called once when the WASM module is loaded
EMSCRIPTEN_KEEPALIVE extern "C" int init() {
  // Initialize the CPU/debugger
  g_debugger.reset();

  // Set up a test program
  const uint16_t START_ADDR = 0x8000;
  uint16_t addr = START_ADDR;

  // LDA #$42 (Load 0x42 into A register)
  g_debugger.write_memory(addr++, 0xA9);
  g_debugger.write_memory(addr++, 0x42);

  // STA $0200 (Store A at memory address 0x0200)
  g_debugger.write_memory(addr++, 0x8D);
  g_debugger.write_memory(addr++, 0x00);
  g_debugger.write_memory(addr++, 0x02);

  // JMP $8000 (Loop forever)
  g_debugger.write_memory(addr++, 0x4C);
  g_debugger.write_memory(addr++, 0x00);
  g_debugger.write_memory(addr++, 0x80);

  // Set reset vector to point to our program
  g_debugger.write_memory(0xFFFC, START_ADDR & 0xFF);
  g_debugger.write_memory(0xFFFD, (START_ADDR >> 8) & 0xFF);

  // Force PC to the correct address
  g_debugger.set_pc(START_ADDR);

  return 1;
}

// Main loop function that will be called from JavaScript
EMSCRIPTEN_KEEPALIVE extern "C" void main_loop() {
  if (g_debugger.is_running()) {
    g_debugger.step();
  }
}
#endif

// Standard C/C++ entry point
int main() {
#ifdef __EMSCRIPTEN__
  // Initialize everything
  init();

  // In Emscripten, we'll let JavaScript call main_loop when needed
  // No need to set up a main loop here
#else
  // For non-Emscripten builds, we could implement testing here
  g_debugger.reset();
  // For testing, just step through a few instructions
  for (int i = 0; i < 10; i++) {
    g_debugger.step();
  }
#endif

  return 0;
}
