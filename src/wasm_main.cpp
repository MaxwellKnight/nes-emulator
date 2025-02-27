#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/debugger.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#define EMSCRIPTEN_EXPORT EMSCRIPTEN_KEEPALIVE extern "C"
#else
// For testing without Emscripten
#define EMSCRIPTEN_KEEPALIVE
#define EMSCRIPTEN_EXPORT extern "C"
#endif

// Global instances that will be used by the debugger
nes::Bus g_bus;
nes::CPU g_cpu(g_bus);
nes::Debugger g_debugger(g_cpu, g_bus);

// Main initialization function
EMSCRIPTEN_EXPORT int init() {
  g_cpu.reset();
  return 1;
}

// Main emulation loop that will be called by JavaScript
EMSCRIPTEN_EXPORT void main_loop() {
  if (g_debugger.is_running()) {
    g_debugger.step();
  }
}

// Set up the main loop to be called from JavaScript
int main() {
  // Initialize everything
  init();

  // We don't set up an animation frame callback here
  // Instead, JavaScript will call main_loop() when needed

  return 0;
}
