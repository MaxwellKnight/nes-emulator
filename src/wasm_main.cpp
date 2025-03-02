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
// Main loop function that will be called from JavaScript
EMSCRIPTEN_KEEPALIVE extern "C" void main_loop() {
  if (g_debugger.is_running()) {
    g_debugger.step();
  }
}
#endif

int main() {
#ifdef __EMSCRIPTEN__
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
}
