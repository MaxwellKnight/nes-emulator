# Spec: headless library + RL environment

Status: in progress. The C ABI (`include/nes_env.h`), the self-contained shared
library (`libnesenv`), the `.nesmovie` record/replay format, the `record_demo`
recorder, and the browser "Watch Movie" player are built and tested. The Python
ctypes wrapper, the Gymnasium environment, and the training examples are still to do.

## Goal

Turn the emulator core into a clean, deterministic, headless library you can drive
from outside the browser, and build a reinforcement-learning environment on top so an
agent can learn to play. The valuable, distinctive part is the environment
infrastructure: a stable C ABI, deterministic stepping, and a Gymnasium-compatible
Python API that can run many independent machines in one process. Actually training a
good agent is an application built on this, not the deliverable.

## Why bother (the systems angle)

- Clean FFI/ABI design across a C++ to C to Python boundary.
- Determinism as a hard, tested guarantee, not a hope.
- Multiple independent machines per process, so the env is vectorizable for throughput.
- Reuses the existing deterministic core. No Emscripten dependency for native use.

## Scope

In:

- A C ABI (`extern "C"`) over the core: create, destroy, load, reset, step, and
  observation/state accessors.
- A self-contained native shared library (`.so` / `.dylib` / `.dll`) built via CMake.
- A Python package that loads the library with `ctypes` and exposes a low-level
  handle, a generic `NesEnv`, a game-specific `SuperMarioBrosEnv`, and an optional
  Gymnasium adapter.
- A determinism test on both the C++ and Python sides.
- Runnable example agents (random and scripted) that exercise the env and reward.
- Docs.

Out (non-goals for v1):

- Training a strong agent or shipping trained weights.
- A pybind11 or numpy-heavy interface. numpy stays optional.
- Audio in the observation.
- Save-state / record-replay. That is a separate feature.
- Reward logic for more than one reference game (SMB).

## Architecture

Four layers:

1. Core (already exists): Bus + CPU + PPU + APU + Cartridge. Deterministic, stepped at
   1 CPU to 3 PPU.
2. C ABI (`include/nes_env.h`, `src/nes_env.cpp`): one opaque handle equals one
   independent machine (its own Bus, Debugger, Cartridge, and a copy of the ROM bytes
   so reset can reboot deterministically). No global state, so handles are isolated.
3. Shared library `libnesenv`: self-contained, built from the core sources plus the C
   ABI, compiled position-independent.
4. Python package `python/nesenv/`:
   - `_native.py`: finds and loads the shared library, declares the ctypes signatures.
   - `core.py`: a `Nes` handle class that owns the pointer and frees it on `__del__`.
   - `env.py`: the generic `NesEnv` (game-agnostic step/reset/observation).
   - `smb.py`: `SuperMarioBrosEnv` (reward, done, episode start for Super Mario Bros).
   - `gymnasium_env.py`: an optional `gymnasium.Env` adapter.
   - `examples/`: `random_agent.py`, `scripted_agent.py`.

## C ABI surface (draft)

```c
typedef struct NesEnv NesEnv;

NesEnv*        nes_create(void);
void           nes_destroy(NesEnv* e);
int            nes_load(NesEnv* e, const unsigned char* rom, int len); // 0 ok, else iNES error
void           nes_reset(NesEnv* e);                    // power-on reboot of the loaded ROM
int            nes_step(NesEnv* e, unsigned char p1);   // advance one frame; returns frame reason
void           nes_set_controller(NesEnv* e, int port, unsigned char buttons);
const unsigned char* nes_framebuffer(NesEnv* e);        // 256*240*4 RGBA, valid until next step
int            nes_framebuffer_size(NesEnv* e);         // 245760
void           nes_get_ram(NesEnv* e, unsigned char* out_2048); // copy $0000-$07FF
unsigned char  nes_peek(NesEnv* e, unsigned short addr);
unsigned int   nes_frame_count(NesEnv* e);
const char*    nes_version(void);
```

Button bit layout matches the controller hardware: d0 A, d1 B, d2 Select, d3 Start,
d4 Up, d5 Down, d6 Left, d7 Right.

Threading: a single handle is single-threaded, but separate handles share nothing, so
they can run on different threads or processes in parallel. That is the multi-env
throughput story.

## Observation, action, reward (Super Mario Bros reference)

Observation (configurable):

- `ram`: the 2 KB work RAM as a byte vector. Fast, compact, the sensible default for SMB.
- `rgb`: the 256x240 framebuffer for pixel-based agents.

Action space: a small discrete set of button combinations mapped to controller
bitmasks, for example NOOP, Right, Right+A, Right+B, Right+A+B, Left, A, Down. The full
256-value space is available but off by default.

Reward (SMB): horizontal progress plus score, minus time, minus death.

- progress = Player_PageLoc * 256 + Player_X, using the documented SMB addresses
  ($006D and $0086). reward is the clamped change in progress between frames.
- a small per-frame penalty to encourage speed.
- a large penalty and episode end on death (lives at $075A decreasing).

Episode start: reset reboots the ROM, then auto-advances through the title and the
WORLD 1-1 card (press Start, run until OperMode at $0770 is 1 and the level is live), so
every episode begins in gameplay, deterministically.

Frame skip / action repeat: configurable, default 4 frames per action, matching common
NES RL practice.

## Determinism guarantee

Same ROM, same reset, same action sequence produces identical observations and rewards,
byte for byte, across runs, machines, and handle instances.

Tests:

- C++: two handles, the same scripted inputs for K frames, assert identical framebuffer
  and RAM hashes every frame.
- Python: the same action list twice yields identical observation and reward streams,
  and N handles stepped in lockstep stay identical.

## Build and packaging

- CMake: `add_library(nesenv SHARED src/nes_env.cpp ${SOURCES})` with
  POSITION_INDEPENDENT_CODE, behind an option (default ON for native builds, OFF under
  Emscripten).
- A native gtest target `nes_env_test` for the C ABI and determinism.
- The Python loader searches the build directories and a `NESENV_LIB` env var, with a
  clear error if the library has not been built.
- Python packaging via `pyproject.toml` so `pip install -e python/` works. numpy and
  gymnasium are optional extras; the base env needs neither.

## Testing plan

- C ABI (gtest): load returns 0 for valid iNES and nonzero for junk, step advances the
  frame count, framebuffer size is correct, two instances stay identical.
- Python (pytest): library loads, reset returns an observation of the declared shape,
  step returns (obs, reward, terminated, truncated, info), determinism holds, SMB reward
  is positive when moving right from the start, and death ends the episode.
- CI: build the shared library in the native job and run the C ABI test under ctest.
  Python tests live in an optional job since they need a Python toolchain.

## Examples

- `random_agent.py`: random actions for one episode, prints cumulative reward and final
  progress, optionally dumps frames to PNG.
- `scripted_agent.py`: a simple "hold Right, jump on a rule" policy that shows the reward
  actually rewards progress, which is the sanity proof that the env is learnable.
- A doc snippet for plugging the Gymnasium env into stable-baselines3 PPO. Not run in
  CI, just the path to real training.

## Milestones

1. C ABI, shared library, and the gtest determinism test. (foundation)
2. Python ctypes wrapper, base `NesEnv`, determinism pytest.
3. `SuperMarioBrosEnv` with observation, action, reward, and episode start, plus the
   random and scripted examples.
4. Optional Gymnasium adapter and the stable-baselines3 usage note.
5. Docs (README section and `python/README.md`) and CI wiring.

## Decisions to confirm

- Default observation: RAM (recommended) or pixels.
- Action set: small SMB set (recommended) or full 256.
- Frame skip default: 4 (recommended).
- Python deps: keep numpy and gymnasium optional (recommended) or make them required.
- Vectorized-env support in v1 (multiple handles plus a simple VecEnv) or defer to later.

## Risks

- SMB reward shaping depends on the right RAM addresses. Mitigate by checking against
  the known SMB RAM map and a scripted "move right" sanity test.
- Without a dot-accurate PPU, pixel observations are fine but some frame-exact behavior
  differs from hardware. RL does not care, but it is worth noting.
- Python environments vary (library path, installed deps). Mitigate with a robust loader,
  clear error messages, and optional deps.
