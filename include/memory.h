#pragma once
#include <array>
#include <cstdint>

namespace cpu6502 {
using u8 = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;

struct Memory {
  static constexpr std::size_t MEMORY_SIZE = 0x10000; // 64KB (65536 bytes)
  std::array<u8, MEMORY_SIZE> memory{};

  [[nodiscard]] u8 read(u16 address) const;
  void write(u16 address, u8 value);
  [[nodiscard]] u16 read_word(u16 address) const;
  void write_word(u16 address, u16 value);
};
} // namespace cpu6502
