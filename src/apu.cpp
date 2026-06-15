#include "apu.h"

namespace nes {
namespace {
const u8 LENGTH_TABLE[32] = {10, 254, 20, 2, 40, 4, 80, 6, 160, 8, 60,
                             10, 14, 12, 26, 14, 12, 16, 24, 18, 48, 20,
                             96, 22, 192, 24, 72, 26, 16, 28, 32, 30};
const u8 DUTY[4][8] = {{0, 1, 0, 0, 0, 0, 0, 0},
                       {0, 1, 1, 0, 0, 0, 0, 0},
                       {0, 1, 1, 1, 1, 0, 0, 0},
                       {1, 0, 0, 1, 1, 1, 1, 1}};
const u16 NOISE_PERIOD[16] = {4, 8, 16, 32, 64, 96, 128, 160, 202, 254,
                              380, 508, 762, 1016, 2034, 4068};
const u8 TRI_SEQ[32] = {15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0,
                        0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
}  // namespace

// ---- Envelope -------------------------------------------------------------
void APU::Envelope::clock() {
  if (start) {
    start = false;
    decay = 15;
    divider = volume;
  } else if (divider == 0) {
    divider = volume;
    if (decay > 0) {
      decay--;
    } else if (loop) {
      decay = 15;
    }
  } else {
    divider--;
  }
}

// ---- Pulse ----------------------------------------------------------------
void APU::Pulse::clock_timer() {
  if (timer == 0) {
    timer = timer_period;
    duty_step = (duty_step + 1) & 7;
  } else {
    timer--;
  }
}

u16 APU::Pulse::target_period() const {
  int change = timer_period >> sweep_shift;
  if (sweep_negate) change = -change - (channel == 1 ? 1 : 0);
  int target = static_cast<int>(timer_period) + change;
  return target < 0 ? 0 : static_cast<u16>(target);
}

bool APU::Pulse::muted() const {
  return timer_period < 8 || target_period() > 0x7FF;
}

void APU::Pulse::clock_sweep() {
  if (sweep_divider == 0 && sweep_enabled && sweep_shift > 0 && !muted()) {
    timer_period = target_period();
  }
  if (sweep_divider == 0 || sweep_reload) {
    sweep_divider = sweep_period;
    sweep_reload = false;
  } else {
    sweep_divider--;
  }
}

u8 APU::Pulse::output() const {
  if (!enabled || length == 0 || muted() || DUTY[duty][duty_step] == 0) return 0;
  return env.output();
}

// ---- Triangle -------------------------------------------------------------
void APU::Triangle::clock_timer() {
  if (timer == 0) {
    timer = timer_period;
    if (length > 0 && linear > 0) step = (step + 1) & 31;
  } else {
    timer--;
  }
}

u8 APU::Triangle::output() const {
  if (!enabled) return 0;
  // Silence ultrasonic periods to avoid popping.
  if (timer_period < 2) return 7;
  return TRI_SEQ[step];
}

// ---- Noise ----------------------------------------------------------------
void APU::Noise::clock_timer() {
  if (timer == 0) {
    timer = timer_period;
    u16 fb = (shift & 1) ^ ((shift >> (mode ? 6 : 1)) & 1);
    shift = static_cast<u16>((shift >> 1) | (fb << 14));
  } else {
    timer--;
  }
}

u8 APU::Noise::output() const {
  if (!enabled || length == 0 || (shift & 1)) return 0;
  return env.output();
}

// ---- APU ------------------------------------------------------------------
APU::APU() { reset(); }

void APU::reset() {
  _pulse1 = Pulse();
  _pulse2 = Pulse();
  _pulse1.channel = 1;
  _pulse2.channel = 2;
  _triangle = Triangle();
  _noise = Noise();
  _noise.shift = 1;
  _frame_cycles = 0;
  _frame_mode = 0;
  _apu_cycle = false;
  _sample_accum = 0.0;
  _ring_w = _ring_r = 0;
}

void APU::clock_quarter_frame() {
  _pulse1.env.clock();
  _pulse2.env.clock();
  _noise.env.clock();
  // Triangle linear counter.
  if (_triangle.linear_reload) {
    _triangle.linear = _triangle.linear_reload_val;
  } else if (_triangle.linear > 0) {
    _triangle.linear--;
  }
  if (!_triangle.control) _triangle.linear_reload = false;
}

void APU::clock_half_frame() {
  if (!_pulse1.length_halt && _pulse1.length > 0) _pulse1.length--;
  if (!_pulse2.length_halt && _pulse2.length > 0) _pulse2.length--;
  if (!_triangle.control && _triangle.length > 0) _triangle.length--;
  if (!_noise.length_halt && _noise.length > 0) _noise.length--;
  _pulse1.clock_sweep();
  _pulse2.clock_sweep();
}

float APU::mix() const {
  float p = static_cast<float>(_pulse1.output() + _pulse2.output());
  float pulse_out = p > 0 ? 95.88f / (8128.0f / p + 100.0f) : 0.0f;
  float tnd = _triangle.output() / 8227.0f + _noise.output() / 12241.0f;
  float tnd_out = tnd > 0 ? 159.79f / (1.0f / tnd + 100.0f) : 0.0f;
  return pulse_out + tnd_out;  // ~0..1
}

void APU::clock() {
  _triangle.clock_timer();  // ticks at the CPU rate
  if (_apu_cycle) {
    _pulse1.clock_timer();
    _pulse2.clock_timer();
    _noise.clock_timer();
  }
  _apu_cycle = !_apu_cycle;

  _frame_cycles++;
  if (_frame_mode == 0) {  // 4-step
    if (_frame_cycles == 7457) {
      clock_quarter_frame();
    } else if (_frame_cycles == 14913) {
      clock_quarter_frame();
      clock_half_frame();
    } else if (_frame_cycles == 22371) {
      clock_quarter_frame();
    } else if (_frame_cycles == 29829) {
      clock_quarter_frame();
      clock_half_frame();
    } else if (_frame_cycles >= 29830) {
      _frame_cycles = 0;
    }
  } else {  // 5-step
    if (_frame_cycles == 7457 || _frame_cycles == 22371) {
      clock_quarter_frame();
    } else if (_frame_cycles == 14913 || _frame_cycles == 37281) {
      clock_quarter_frame();
      clock_half_frame();
    } else if (_frame_cycles >= 37282) {
      _frame_cycles = 0;
    }
  }

  // Downsample the ~1.79MHz CPU rate to 44.1kHz, with a DC-blocking high-pass.
  _sample_accum += 44100.0 / 1789773.0;
  if (_sample_accum >= 1.0) {
    _sample_accum -= 1.0;
    float raw = mix();
    float out = raw - _hp_prev_in + 0.995f * _hp_prev_out;
    _hp_prev_in = raw;
    _hp_prev_out = out;
    push_sample(out);
  }
}

void APU::push_sample(float s) {
  int next = (_ring_w + 1) % RING;
  if (next == _ring_r) return;  // full: drop (front-end fell behind)
  _ring[_ring_w] = s;
  _ring_w = next;
}

int APU::available() const {
  return (_ring_w - _ring_r + RING) % RING;
}

int APU::drain(float* out, int max) {
  int n = 0;
  while (n < max && _ring_r != _ring_w) {
    out[n++] = _ring[_ring_r];
    _ring_r = (_ring_r + 1) % RING;
  }
  return n;
}

void APU::write(u16 address, u8 value) {
  switch (address) {
    case 0x4000:
      _pulse1.duty = (value >> 6) & 3;
      _pulse1.length_halt = (value & 0x20) != 0;
      _pulse1.env.loop = _pulse1.length_halt;
      _pulse1.env.constant = (value & 0x10) != 0;
      _pulse1.env.volume = value & 0x0F;
      break;
    case 0x4001:
      _pulse1.sweep_enabled = (value & 0x80) != 0;
      _pulse1.sweep_period = (value >> 4) & 7;
      _pulse1.sweep_negate = (value & 0x08) != 0;
      _pulse1.sweep_shift = value & 7;
      _pulse1.sweep_reload = true;
      break;
    case 0x4002:
      _pulse1.timer_period = (_pulse1.timer_period & 0x700) | value;
      break;
    case 0x4003:
      _pulse1.timer_period = (_pulse1.timer_period & 0xFF) | ((value & 7) << 8);
      if (_pulse1.enabled) _pulse1.length = LENGTH_TABLE[(value >> 3) & 0x1F];
      _pulse1.env.start = true;
      _pulse1.duty_step = 0;
      break;

    case 0x4004:
      _pulse2.duty = (value >> 6) & 3;
      _pulse2.length_halt = (value & 0x20) != 0;
      _pulse2.env.loop = _pulse2.length_halt;
      _pulse2.env.constant = (value & 0x10) != 0;
      _pulse2.env.volume = value & 0x0F;
      break;
    case 0x4005:
      _pulse2.sweep_enabled = (value & 0x80) != 0;
      _pulse2.sweep_period = (value >> 4) & 7;
      _pulse2.sweep_negate = (value & 0x08) != 0;
      _pulse2.sweep_shift = value & 7;
      _pulse2.sweep_reload = true;
      break;
    case 0x4006:
      _pulse2.timer_period = (_pulse2.timer_period & 0x700) | value;
      break;
    case 0x4007:
      _pulse2.timer_period = (_pulse2.timer_period & 0xFF) | ((value & 7) << 8);
      if (_pulse2.enabled) _pulse2.length = LENGTH_TABLE[(value >> 3) & 0x1F];
      _pulse2.env.start = true;
      _pulse2.duty_step = 0;
      break;

    case 0x4008:
      _triangle.control = (value & 0x80) != 0;
      _triangle.linear_reload_val = value & 0x7F;
      break;
    case 0x400A:
      _triangle.timer_period = (_triangle.timer_period & 0x700) | value;
      break;
    case 0x400B:
      _triangle.timer_period =
          (_triangle.timer_period & 0xFF) | ((value & 7) << 8);
      if (_triangle.enabled) _triangle.length = LENGTH_TABLE[(value >> 3) & 0x1F];
      _triangle.linear_reload = true;
      break;

    case 0x400C:
      _noise.length_halt = (value & 0x20) != 0;
      _noise.env.loop = _noise.length_halt;
      _noise.env.constant = (value & 0x10) != 0;
      _noise.env.volume = value & 0x0F;
      break;
    case 0x400E:
      _noise.mode = (value & 0x80) != 0;
      _noise.timer_period = NOISE_PERIOD[value & 0x0F];
      break;
    case 0x400F:
      if (_noise.enabled) _noise.length = LENGTH_TABLE[(value >> 3) & 0x1F];
      _noise.env.start = true;
      break;

    case 0x4015:
      _pulse1.enabled = (value & 0x01) != 0;
      _pulse2.enabled = (value & 0x02) != 0;
      _triangle.enabled = (value & 0x04) != 0;
      _noise.enabled = (value & 0x08) != 0;
      if (!_pulse1.enabled) _pulse1.length = 0;
      if (!_pulse2.enabled) _pulse2.length = 0;
      if (!_triangle.enabled) _triangle.length = 0;
      if (!_noise.enabled) _noise.length = 0;
      break;

    case 0x4017:
      _frame_mode = (value & 0x80) ? 1 : 0;
      _frame_cycles = 0;
      if (_frame_mode == 1) {  // 5-step: immediate quarter+half clock
        clock_quarter_frame();
        clock_half_frame();
      }
      break;

    default:
      break;  // $4010-$4013 (DMC) accepted as no-ops
  }
}

u8 APU::read_status() const {
  u8 status = 0;
  if (_pulse1.length > 0) status |= 0x01;
  if (_pulse2.length > 0) status |= 0x02;
  if (_triangle.length > 0) status |= 0x04;
  if (_noise.length > 0) status |= 0x08;
  return status;
}
}  // namespace nes
