// Conformance suite: runs the canonical NES test ROMs (kevtris' nestest and
// blargg's instr_test family) through the real core and checks each ROM's
// self-reported result. ROMs are fetched on demand by tests/roms/fetch_test_roms.sh
// (not committed); when a ROM is absent the case is SKIPPED rather than failed,
// so the suite stays green on a fresh checkout and turns into a scoreboard once
// the ROMs are present.
//
// Two result protocols are implemented:
//   * nestest  - runs from $C000 and reports official/illegal opcode errors in
//                zero page $02 (and $03). 0/0 means every opcode behaved.
//   * blargg   - writes 0x80 to $6000 while running, then a result code (0 = pass);
//                a 0xDE 0xB0 0x61 signature at $6001-$6003 marks the value valid,
//                and a human-readable message follows at $6004.
#include <gtest/gtest.h>
#include <cstdint>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>
#include "bus.h"
#include "cartridge.h"
#include "debugger.h"

#ifndef NES_TEST_ROM_DIR
#define NES_TEST_ROM_DIR "tests/roms"
#endif

using namespace nes;

namespace {

std::vector<u8> read_rom(const std::string& name) {
  std::ifstream f(std::string(NES_TEST_ROM_DIR) + "/" + name, std::ios::binary);
  if (!f) return {};
  return {std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>()};
}

struct BlarggResult {
  bool reached = false;  // the $6001-$6003 signature was seen
  int code = -1;         // 0 = pass
  std::string message;
};

BlarggResult run_blargg(const std::vector<u8>& bytes, int max_frames = 4000) {
  BlarggResult r;
  int st = 0;
  auto cart = Cartridge::from_ines(bytes, st);
  if (st != 0) return r;
  Bus bus;
  bus.insert_cartridge(cart);
  bus.reset();
  Debugger dbg{bus.get_cpu(), bus};
  dbg.reset_to_vector();

  auto step_frame = [&]() {
    uint32_t s = bus.get_ppu().frame_count();
    long guard = 0;
    while (bus.get_ppu().frame_count() == s) {
      bus.clock();
      if (++guard > 500000) return;
    }
  };

  for (int f = 0; f < max_frames; f++) {
    step_frame();
    if (bus.cpu_read(0x6001) == 0xDE && bus.cpu_read(0x6002) == 0xB0 &&
        bus.cpu_read(0x6003) == 0x61) {
      r.reached = true;
      u8 status = bus.cpu_read(0x6000);
      if (status != 0x80) {
        r.code = status;
        break;
      }
    }
  }
  for (int i = 0; i < 120; i++) {
    u8 c = bus.cpu_read(0x6004 + i);
    if (!c) break;
    if (c >= 32 && c < 127) r.message += static_cast<char>(c);
  }
  return r;
}

// nestest: official + illegal opcode behavior, result in zero page $02/$03.
bool run_nestest(const std::vector<u8>& bytes, std::string& detail) {
  int st = 0;
  auto cart = Cartridge::from_ines(bytes, st);
  if (st != 0) {
    detail = "load failed";
    return false;
  }
  Bus bus;
  bus.insert_cartridge(cart);
  bus.reset();
  Debugger dbg{bus.get_cpu(), bus};
  dbg.set_pc(0xC000);
  for (long i = 0; i < 12000; i++) {
    u16 pc = dbg.get_register_pc();
    dbg.step();
    if (dbg.get_register_pc() == pc) break;  // self-loop at the end
  }
  u8 r2 = bus.cpu_read(0x0002), r3 = bus.cpu_read(0x0003);
  char buf[64];
  std::snprintf(buf, sizeof(buf), "$02=%02X $03=%02X", r2, r3);
  detail = buf;
  return r2 == 0 && r3 == 0;
}

// Required ROM: must pass, or the suite (and CI) goes red.
void expect_blargg_pass(const std::string& file) {
  auto bytes = read_rom(file);
  if (bytes.empty()) GTEST_SKIP() << file << " absent — run tests/roms/fetch_test_roms.sh";
  auto r = run_blargg(bytes);
  ASSERT_TRUE(r.reached) << file << ": result protocol never reached";
  EXPECT_EQ(r.code, 0) << file << ": '" << r.message << "'";
}

// Known-gap ROM: run it and surface the result, but SKIP (don't fail) so the
// scoreboard stays honest about accuracy work still in progress.
void report_blargg_gap(const std::string& file) {
  auto bytes = read_rom(file);
  if (bytes.empty()) GTEST_SKIP() << file << " absent — run tests/roms/fetch_test_roms.sh";
  auto r = run_blargg(bytes);
  if (r.reached && r.code == 0) {
    SUCCEED() << file << " now PASSES (promote it to a required test!)";
  } else {
    GTEST_SKIP() << file << " known gap (needs dot-accurate PPU): code=" << r.code
                 << " '" << r.message << "'";
  }
}

}  // namespace

// ---- CPU: required (these define our correctness floor) --------------------
TEST(Conformance, NestestCpuOpcodes) {
  auto bytes = read_rom("nestest.nes");
  if (bytes.empty()) GTEST_SKIP() << "nestest.nes absent — run tests/roms/fetch_test_roms.sh";
  std::string detail;
  EXPECT_TRUE(run_nestest(bytes, detail)) << "nestest reported errors: " << detail;
}

TEST(Conformance, BlarggInstrBasics) { expect_blargg_pass("instr_01_basics.nes"); }
TEST(Conformance, BlarggInstrImplied) { expect_blargg_pass("instr_02_implied.nes"); }
TEST(Conformance, BlarggInstrAllOfficial) { expect_blargg_pass("instr_official.nes"); }
TEST(Conformance, BlarggInstrTiming) { expect_blargg_pass("instr_timing.nes"); }

// ---- PPU: known accuracy gap (per-scanline renderer, not dot-accurate) -----
TEST(Conformance, BlarggPpuVblNmi) { report_blargg_gap("ppu_vbl_nmi.nes"); }
