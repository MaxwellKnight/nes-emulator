#include "../include/bus.h"
#include "../include/cpu.h"

int main() {
  nes::Bus bus;
  nes::CPU cpu{bus};

  cpu.reset();
}
