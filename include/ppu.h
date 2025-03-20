#include <memory>
#include "cartridge.h"
#include "types.h"

namespace nes {
class PPU : Addressable {
 private:
  std::shared_ptr<Cartridge> _cartridge;

 private:
  u8 _name[2][1024];  // VRAM
  u8 _palette[32];    // RAM
 public:
  PPU();
  ~PPU() = default;

 public:
  void insert_cartridge(const std::shared_ptr<Cartridge>& cartridge);
  void clock();

 public:
  u8 cpu_read(u16 address) const override;
  u8 ppu_read(u16 address) const override;
  void cpu_write(u16 address, u8 value) override;
  void ppu_write(u16 address, u8 value) override;
  bool handles_address(u16 address) const override;
};
};  // namespace nes
