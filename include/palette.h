#pragma once
#include "types.h"

namespace nes {

// 64-entry 2C02 master palette, each entry packed as a little-endian RGBA
// u32 so the bytes appear in memory as R,G,B,A:
//   pack(r,g,b) = u32(r) | (u32(g)<<8) | (u32(b)<<16) | (0xFFu<<24)
extern const u32 NES_PALETTE[64];

// Returns the RGBA color for a 6-bit NES color index (index masked to 0..63).
u32 palette_rgba(u8 index);

}  // namespace nes
