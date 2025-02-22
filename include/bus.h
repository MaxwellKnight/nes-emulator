#pragma once
#include "memory.h"
#include "types.h"
#include <array>

namespace nes {
class Bus {
public:
  Bus();
  static constexpr size_t MEMORY_SIZE = 64 * 1024; // 64KB
  void write(u16 address, u8 data);
  void write_word(u16 address, u16 value);
  [[nodiscard]] u8 read(u16 address) const;
  [[nodiscard]] u16 read_word(u16 address) const;

private:
  std::array<u8, MEMORY_SIZE> ram{};
};
} // namespace nes
