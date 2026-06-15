#pragma once
#include "types.h"

namespace nes {

// NES 2A03 audio processing unit: two pulse channels, triangle, noise, and a
// frame sequencer. Clocked once per CPU cycle; produces mono float samples into
// an internal ring buffer that the front-end drains for WebAudio playback.
// (DMC is not synthesized; its registers are accepted as no-ops.)
class APU {
 public:
  APU();

  void reset();
  void clock();                       // advance one CPU cycle
  void write(u16 address, u8 value);  // $4000-$4017 register write
  u8 read_status() const;             // $4015 read

  // Drain up to `max` queued samples into `out`; returns the count written.
  int drain(float* out, int max);
  int available() const;

 private:
  // --- envelope (pulse + noise) ---
  struct Envelope {
    bool start = false, loop = false, constant = false;
    u8 volume = 0, decay = 0, divider = 0;
    void clock();
    u8 output() const { return constant ? volume : decay; }
  };

  // --- pulse channel ---
  struct Pulse {
    bool enabled = false;
    u8 duty = 0, duty_step = 0;
    u16 timer = 0, timer_period = 0;
    u8 length = 0;
    bool length_halt = false;
    Envelope env;
    // sweep
    bool sweep_enabled = false, sweep_negate = false, sweep_reload = false;
    u8 sweep_period = 0, sweep_shift = 0, sweep_divider = 0;
    int channel = 0;  // 1 or 2 (affects sweep negate behavior)
    void clock_timer();
    void clock_sweep();
    u16 target_period() const;
    bool muted() const;
    u8 output() const;
  };

  // --- triangle ---
  struct Triangle {
    bool enabled = false;
    u16 timer = 0, timer_period = 0;
    u8 length = 0, linear = 0, linear_reload_val = 0;
    bool control = false, linear_reload = false;
    u8 step = 0;
    void clock_timer();
    u8 output() const;
  };

  // --- noise ---
  struct Noise {
    bool enabled = false, mode = false, length_halt = false;
    u16 timer = 0, timer_period = 0, shift = 1;
    u8 length = 0;
    Envelope env;
    void clock_timer();
    u8 output() const;
  };

  void clock_quarter_frame();  // envelopes + triangle linear counter
  void clock_half_frame();     // length counters + sweeps
  float mix() const;

  Pulse _pulse1, _pulse2;
  Triangle _triangle;
  Noise _noise;

  u32 _frame_cycles = 0;  // CPU cycles into the current frame sequence
  u8 _frame_mode = 0;     // 0 = 4-step, 1 = 5-step
  bool _apu_cycle = false;  // pulse/noise timers tick every other CPU cycle

  // sample generation
  double _sample_accum = 0.0;
  float _hp_prev_in = 0.0f, _hp_prev_out = 0.0f;  // DC-blocking high-pass

  static constexpr int RING = 8192;
  float _ring[RING] = {0};
  int _ring_w = 0, _ring_r = 0;
  void push_sample(float s);
};

}  // namespace nes
