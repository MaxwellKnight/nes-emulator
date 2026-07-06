// nes_env.h - a small, stable C ABI over the emulator core.
//
// One opaque NesEnv handle is one independent NES. Handles share no global state,
// so you can run many of them in parallel (one per thread or process) for RL
// throughput. The ABI is plain C so it loads cleanly from Python via ctypes, or
// from any other language. No C++ exception is allowed to cross the boundary.
//
// Button bit layout matches the controller hardware:
//   d0 A, d1 B, d2 Select, d3 Start, d4 Up, d5 Down, d6 Left, d7 Right
#ifndef NES_ENV_H
#define NES_ENV_H

#include <stdint.h>

#ifdef _WIN32
#define NES_API __declspec(dllexport)
#else
#define NES_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef struct NesEnv NesEnv;

// Lifetime.
NES_API NesEnv* nes_create(void);
NES_API void    nes_destroy(NesEnv* e);

// Load an iNES image. Returns 0 on success, or the non-zero from_ines status
// (1 bad header, 2 unsupported mapper). The bytes are copied, so the caller may
// free them immediately.
NES_API int nes_load(NesEnv* e, const uint8_t* rom, int len);

// Power-on reboot of the loaded ROM. Fully deterministic: the same ROM always
// resets to the same state.
NES_API void nes_reset(NesEnv* e);

// Set player 1's buttons and advance exactly one video frame. Returns the frame
// reason from the core (0 = normal frame).
NES_API int nes_step(NesEnv* e, uint8_t p1_buttons);

// Set a controller without stepping (port 0 = player 1, 1 = player 2).
NES_API void nes_set_controller(NesEnv* e, int port, uint8_t buttons);

// The current frame as 256*240 RGBA bytes. The pointer is valid until the next
// step/reset. Use nes_framebuffer_size() for the length (245760).
NES_API const uint8_t* nes_framebuffer(NesEnv* e);
NES_API int            nes_framebuffer_size(NesEnv* e);

// Copy the 2 KB of CPU work RAM ($0000-$07FF) into out (must hold 2048 bytes).
NES_API void nes_get_ram(NesEnv* e, uint8_t* out_2048);

// Read a single byte from CPU address space.
NES_API uint8_t nes_peek(NesEnv* e, uint16_t addr);

// Frames emitted since load/reset.
NES_API uint32_t nes_frame_count(NesEnv* e);

// Library version string, e.g. "nes_env 1".
NES_API const char* nes_version(void);

#ifdef __cplusplus
}
#endif

#endif  // NES_ENV_H
