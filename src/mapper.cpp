#include "mapper.h"

namespace nes {
Mapper::Mapper(u8 prg_banks, u8 chr_banks) {
  _prg_banks = prg_banks;
  _chr_banks = chr_banks;
}
};  // namespace nes
