#pragma once
#include <array>
#include <cstddef>
#include "types.h"

namespace nes {
class Bus : public Addressable {
 public:
  Bus();
  ~Bus() = default;
  void write(u16 address, u8 data) override;
  void write_word(u16 address, u16 value);
  u8 read(u16 address) const override;
  u16 read_word(u16 address) const;
  bool handles_address(u16 address) const override;

 private:
  static constexpr size_t _CPU_RAM_SIZE = 2 * 1024;  // 2KB
  static constexpr size_t _RESET_VECTOR_SIZE = 4;

  std::array<u8, _CPU_RAM_SIZE> _ram{0};
  std::array<u8, _RESET_VECTOR_SIZE> _reset_vector{0};
};
}  // namespace nes
