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
