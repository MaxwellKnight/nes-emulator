// record_demo - drive a scripted agent through Super Mario Bros and write a
// self-contained .nesmovie (ROM + one controller byte per frame). The movie
// replays frame-for-frame in the browser ("Watch Movie" in NES Studio) because
// the core is deterministic.
//
// Usage:
//   record_demo <rom.nes> [out.nesmovie]
//
// This is intentionally a hand-written policy, not a trained agent: it is the
// stand-in that proves the record -> replay pipeline end to end. Swap in a policy
// from the Python RL env and the movie format and browser player stay the same.
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <vector>

#include "nes_env.h"

namespace {

std::vector<uint8_t> read_file(const char* path) {
  std::ifstream f(path, std::ios::binary);
  return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

// Super Mario Bros RAM locations we read to drive and score the agent.
constexpr uint16_t kOperMode = 0x0770;  // 1 once a level is running
constexpr uint16_t kPlayerX = 0x0086;   // on-screen X
constexpr uint16_t kPlayerPage = 0x006D; // horizontal page
constexpr uint16_t kLives = 0x075A;

constexpr uint8_t kA = 0x01;
constexpr uint8_t kStart = 0x08;
constexpr uint8_t kRight = 0x80;

}  // namespace

int main(int argc, char** argv) {
  if (argc < 2) {
    std::fprintf(stderr, "usage: %s <rom.nes> [out.nesmovie]\n", argv[0]);
    return 2;
  }
  const char* rom_path = argv[1];
  const char* out_path = argc > 2 ? argv[2] : "smb_demo.nesmovie";

  std::vector<uint8_t> rom = read_file(rom_path);
  if (rom.empty()) {
    std::fprintf(stderr, "could not read ROM: %s\n", rom_path);
    return 1;
  }

  NesEnv* e = nes_create();
  if (nes_load(e, rom.data(), static_cast<int>(rom.size())) != 0) {
    std::fprintf(stderr, "not a valid iNES ROM: %s\n", rom_path);
    nes_destroy(e);
    return 1;
  }

  std::vector<uint8_t> inputs;
  int lives0 = -1, gameplay_start = -1, max_progress = 0, stall = 0;

  for (int f = 0; f < 1100; f++) {
    uint8_t in = 0;
    if (f < 80) {
      in = 0;  // let the title screen come up
    } else if (f < 90) {
      in = kStart;  // press Start
    } else {
      const int oper = nes_peek(e, kOperMode);
      const int progress = nes_peek(e, kPlayerPage) * 256 + nes_peek(e, kPlayerX);
      const bool playing = oper == 1 && f > 250;  // past the WORLD 1-1 card
      if (playing) {
        if (gameplay_start < 0) {
          gameplay_start = f;
          lives0 = nes_peek(e, kLives);
        }
        in = kRight;
        stall = progress <= max_progress ? stall + 1 : 0;
        if ((f % 24) < 13 || stall > 4) in |= kA;  // jump on a rhythm and when stuck
        if (progress > max_progress) max_progress = progress;
      }
    }

    inputs.push_back(in);
    nes_step(e, in);

    if (gameplay_start > 0 && nes_peek(e, kLives) < lives0) {
      std::printf("agent died at frame %d\n", f);
      break;
    }
  }

  const int progress = nes_peek(e, kPlayerPage) * 256 + nes_peek(e, kPlayerX);
  std::printf("recorded %zu frames, final progress %d (gameplay started at %d)\n",
              inputs.size(), progress, gameplay_start);

  std::ofstream o(out_path, std::ios::binary);
  o.write("NESMOVIE", 8);
  const uint32_t version = 1, rom_len = static_cast<uint32_t>(rom.size()),
                 frame_count = static_cast<uint32_t>(inputs.size());
  o.write(reinterpret_cast<const char*>(&version), 4);
  o.write(reinterpret_cast<const char*>(&rom_len), 4);
  o.write(reinterpret_cast<const char*>(&frame_count), 4);
  o.write(reinterpret_cast<const char*>(rom.data()), rom.size());
  o.write(reinterpret_cast<const char*>(inputs.data()), inputs.size());
  std::printf("wrote %s\n", out_path);

  nes_destroy(e);
  return 0;
}
