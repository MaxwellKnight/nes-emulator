#pragma once
#include "memory.h"
#include "types.h"

namespace nes {

class Bus {
public:
  Bus();

  void write(u16 address, u8 data);
  [[nodiscard]] u8 read(u16 address) const;

private:
  Memory memory;
};
} // namespace nes
