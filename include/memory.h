#pragma once
#include "types.h"
#include <array>

namespace nes {
using u8 = std::uint8_t;
using u16 = std::uint16_t;

class Memory {
public:
  static constexpr size_t MEMORY_SIZE = 65536; // 64KB
  void write(u16 address, u8 value);
  [[nodiscard]] u8 read(u16 address) const;
  [[nodiscard]] u16 read_word(u16 address) const;
  void write_word(u16 address, u16 value);

private:
  std::array<u8, MEMORY_SIZE> memory{};
};
} // namespace nes
