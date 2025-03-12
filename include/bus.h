#pragma once
#include <array>
#include <cstddef>
#include "cpu.h"
#include "types.h"

namespace nes {
class Bus : public Addressable {
 public:
  Bus();
  ~Bus() = default;
  void cpu_write(u16 address, u8 data) override;
  u8 cpu_read(u16 address) const override;
  bool handles_address(u16 address) const override;

 public:
  void clock();
  void reset();
  CPU& get_cpu();

 private:
  static constexpr size_t _CPU_RAM_SIZE = 2 * 1024;  // 2KB
  static constexpr size_t _RESET_VECTOR_SIZE = 4;
  u32 _sys_clock = 0;
  CPU _cpu;

  std::array<u8, _CPU_RAM_SIZE> _ram{0};
  std::array<u8, _RESET_VECTOR_SIZE> _reset_vector{0};
};
}  // namespace nes
