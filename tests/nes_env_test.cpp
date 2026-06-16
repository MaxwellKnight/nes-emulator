// Tests for the headless C ABI (nes_env.h). Uses a tiny synthetic NROM image so
// there is no dependency on any commercial ROM: the program just loops forever,
// which is enough to exercise loading, stepping, and determinism.
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <vector>

#include "nes_env.h"

namespace {

// A minimal valid iNES image: 1x16KB PRG, 1x8KB CHR, mapper 0. PRG starts with
// JMP $C000 (an infinite loop) and the reset vector points at it.
std::vector<uint8_t> synthetic_rom() {
  std::vector<uint8_t> rom(16 + 16384 + 8192, 0);
  rom[0] = 'N'; rom[1] = 'E'; rom[2] = 'S'; rom[3] = 0x1A;
  rom[4] = 1;  // 16KB PRG banks
  rom[5] = 1;  // 8KB CHR banks
  uint8_t* prg = rom.data() + 16;
  prg[0x0000] = 0x4C; prg[0x0001] = 0x00; prg[0x0002] = 0xC0;  // JMP $C000
  prg[0x3FFC] = 0x00; prg[0x3FFD] = 0xC0;                      // reset vector -> $C000
  return rom;
}

uint32_t fnv1a(const uint8_t* data, int n) {
  uint32_t h = 2166136261u;
  for (int i = 0; i < n; i++) h = (h ^ data[i]) * 16777619u;
  return h;
}

}  // namespace

TEST(NesEnv, CreateLoadStep) {
  NesEnv* e = nes_create();
  ASSERT_NE(e, nullptr);
  auto rom = synthetic_rom();
  EXPECT_EQ(nes_load(e, rom.data(), static_cast<int>(rom.size())), 0);

  EXPECT_EQ(nes_frame_count(e), 0u);
  for (int i = 0; i < 10; i++) nes_step(e, 0);
  EXPECT_EQ(nes_frame_count(e), 10u);

  EXPECT_EQ(nes_framebuffer_size(e), 256 * 240 * 4);
  EXPECT_NE(nes_framebuffer(e), nullptr);
  nes_destroy(e);
}

TEST(NesEnv, RejectsJunk) {
  NesEnv* e = nes_create();
  std::vector<uint8_t> junk(64, 0xFF);
  EXPECT_NE(nes_load(e, junk.data(), static_cast<int>(junk.size())), 0);
  nes_destroy(e);
}

TEST(NesEnv, ResetIsDeterministic) {
  NesEnv* e = nes_create();
  auto rom = synthetic_rom();
  nes_load(e, rom.data(), static_cast<int>(rom.size()));

  const uint8_t seq[] = {0, 0x80, 0x01, 0x08, 0x10, 0, 0x80, 0x40};
  uint8_t ram_a[2048], ram_b[2048];
  for (int i = 0; i < 50; i++) nes_step(e, seq[i % 8]);
  nes_get_ram(e, ram_a);

  nes_reset(e);
  for (int i = 0; i < 50; i++) nes_step(e, seq[i % 8]);
  nes_get_ram(e, ram_b);

  EXPECT_EQ(0, std::memcmp(ram_a, ram_b, 2048)) << "reset + same inputs must replay identically";
  nes_destroy(e);
}

TEST(NesEnv, InstancesAreIndependentAndIdentical) {
  auto rom = synthetic_rom();
  NesEnv* a = nes_create();
  NesEnv* b = nes_create();
  nes_load(a, rom.data(), static_cast<int>(rom.size()));
  nes_load(b, rom.data(), static_cast<int>(rom.size()));

  const uint8_t seq[] = {0, 0x80, 0x81, 0x08, 0, 0x40};
  for (int f = 0; f < 120; f++) {
    nes_step(a, seq[f % 6]);
    nes_step(b, seq[f % 6]);
    const int n = nes_framebuffer_size(a);
    ASSERT_EQ(fnv1a(nes_framebuffer(a), n), fnv1a(nes_framebuffer(b), n))
        << "two handles with identical inputs diverged at frame " << f;
  }
  nes_destroy(a);
  nes_destroy(b);
}
