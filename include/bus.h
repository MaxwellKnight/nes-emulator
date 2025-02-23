#pragma once
#include "memory.h"
#include "types.h"
#include <array>

namespace nes {
class Bus : public Addressable {
public:
  Bus();
  void write(u16 address, u8 data) override;
  void write_word(u16 address, u16 value);
  [[nodiscard]] u8 read(u16 address) const override;
  [[nodiscard]] u16 read_word(u16 address) const;
  [[nodiscard]] bool handles_address(u16 address) const override;

private:
  static constexpr size_t CPU_RAM_SIZE = 2 * 1024; // 2KB
  static constexpr size_t RESET_VECTOR_SIZE = 4;

  std::array<u8, CPU_RAM_SIZE> ram{};
  std::array<u8, RESET_VECTOR_SIZE> reset_vector{};
};
} // namespace nes
