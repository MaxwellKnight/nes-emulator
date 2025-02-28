#include "../include/bus.h"
#include "../include/cpu.h"
#include "../include/debugger.h"

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
// Only define the macro if it's not already defined
#ifndef EMSCRIPTEN_EXPORT
#define EMSCRIPTEN_EXPORT EMSCRIPTEN_KEEPALIVE extern "C"
#endif
#else
// For testing without Emscripten
#ifndef EMSCRIPTEN_KEEPALIVE
#define EMSCRIPTEN_KEEPALIVE
#endif
#ifndef EMSCRIPTEN_EXPORT
#define EMSCRIPTEN_EXPORT extern "C"
#endif
#endif

nes::Bus g_bus;
nes::CPU g_cpu(g_bus);
nes::Debugger g_debugger(g_cpu, g_bus);

EMSCRIPTEN_EXPORT int init() {
  g_cpu.reset();
  return 1;
}

// Empty implementation for debugging
EMSCRIPTEN_EXPORT void main_loop() {
  // Empty implementation to debug signature issues
  // Once this works, you can restore the original functionality:
  // if (g_debugger.is_running()) {
  //   g_debugger.step();
  // }
}

// Set up the main loop to be called from JavaScript
int main() {
  // Initialize everything
  init();
  // Not setting up an animation frame callback here
  // Instead, JavaScript will call main_loop() when needed
  return 0;
}
