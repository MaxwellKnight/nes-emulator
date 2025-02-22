#include "../include/bus.h"

namespace nes {

Bus::Bus() {
}

void Bus::write(u16 address, u8 data) {
    memory.write(address, data);
}

u8 Bus::read(u16 address) const {
    return memory.read(address);
}

} // namespace nes
