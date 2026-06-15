#pragma once
#include <array>
#include <cstddef>
#include <memory>
#include "cpu.h"
#include "ppu.h"
#include "types.h"

namespace nes {
class Bus {
 public:
  Bus();
  ~Bus() = default;

 public:
  void cpu_write(u16 address, u8 data);
  u8 cpu_read(u16 address) const;

 public:
  void clock();
  void reset();
  CPU& get_cpu();
  PPU& get_ppu();
  void insert_cartridge(const std::shared_ptr<Cartridge>& cartridge);

 private:
  static constexpr size_t _CPU_RAM_SIZE = 2 * 1024;  // 2KB
  u32 _sys_clock = 0;
  int _dma_stall = 0;  // CPU cycles remaining stalled by an OAM ($4014) DMA
  CPU _cpu;
  PPU _ppu;
  std::shared_ptr<Cartridge> _cartridge;
  std::array<u8, _CPU_RAM_SIZE> _ram{0};
};
}  // namespace nes
