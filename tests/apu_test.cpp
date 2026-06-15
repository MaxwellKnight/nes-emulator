#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "apu.h"

using namespace nes;

namespace {
void run(APU& a, int cycles) {
  for (int i = 0; i < cycles; i++) a.clock();
}
}  // namespace

// An enabled pulse channel produces a non-silent, oscillating sample stream.
TEST(APUTest, PulseProducesAudio) {
  APU apu;
  apu.write(0x4015, 0x01);  // enable pulse 1
  apu.write(0x4000, 0x98);  // duty 2, constant volume 8
  apu.write(0x4002, 100);   // timer low
  apu.write(0x4003, 0x08);  // timer high 0 + length index 1 -> length 254

  run(apu, 100000);

  std::vector<float> buf(4096);
  int n = apu.drain(buf.data(), 4096);
  ASSERT_GT(n, 100);
  bool any_pos = false, any_neg = false;
  for (int i = 0; i < n; i++) {
    if (buf[i] > 0.001f) any_pos = true;
    if (buf[i] < -0.001f) any_neg = true;
  }
  EXPECT_TRUE(any_pos && any_neg) << "expected an oscillating (non-DC) waveform";
}

// With no channels enabled the output settles to silence.
TEST(APUTest, SilenceWhenDisabled) {
  APU apu;
  apu.write(0x4015, 0x00);
  run(apu, 50000);
  std::vector<float> buf(4096);
  int n = apu.drain(buf.data(), 4096);
  float peak = 0.0f;
  for (int i = 0; i < n; i++) peak = std::max(peak, std::fabs(buf[i]));
  EXPECT_LT(peak, 0.01f);
}

// $4015 reports which channels still have length remaining; disabling clears it.
TEST(APUTest, StatusReflectsLengthCounters) {
  APU apu;
  apu.write(0x4015, 0x0F);  // enable all
  apu.write(0x4003, 0x08);  // pulse1 length
  apu.write(0x400F, 0x08);  // noise length
  EXPECT_TRUE(apu.read_status() & 0x01);  // pulse1 has length
  EXPECT_TRUE(apu.read_status() & 0x08);  // noise has length
  apu.write(0x4015, 0x00);  // disable all -> lengths cleared
  EXPECT_EQ(apu.read_status() & 0x0F, 0x00);
}

// The triangle channel emits audio when its timer/length/linear are set.
TEST(APUTest, TriangleProducesAudio) {
  APU apu;
  apu.write(0x4015, 0x04);  // enable triangle
  apu.write(0x4008, 0x81);  // control set, linear reload value
  apu.write(0x400A, 80);    // timer low
  apu.write(0x400B, 0x08);  // timer high + length
  run(apu, 100000);
  std::vector<float> buf(4096);
  int n = apu.drain(buf.data(), 4096);
  float peak = 0.0f;
  for (int i = 0; i < n; i++) peak = std::max(peak, std::fabs(buf[i]));
  EXPECT_GT(peak, 0.001f);
}
