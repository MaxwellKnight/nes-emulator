# NES Emulator + 6502 Studio

A cycle-aware **Nintendo Entertainment System emulator** written in C++17, compiled to
WebAssembly, and wrapped in **NES Studio** — a browser-based debugger that lets you
single-step a real 6502, watch the PPU render, and play commercial games in the same
window. The CPU passes the canonical [`nestest`](https://www.qmtpro.com/~nes/misc/nestest.txt)
and [blargg `instr_test`](https://github.com/christopherpow/nes-test-roms) suites.

![NES Studio screenshot](imgs/main-screenshot.png)

## Highlights

- **Full 6502 core** — all 151 official opcodes *and* the documented unofficial
  opcodes, validated cycle-for-cycle against the `nestest` golden log (5003/5003
  instructions match on PC, registers, **and** cycle count).
- **PPU** — background + sprite rendering, OAM/`$4014` DMA, 8×8 & 8×16 sprites,
  sprite-0 hit, loopy scrolling, and the four mirroring modes. Renders real games
  (Super Mario Bros. boots into playable 1-1).
- **APU** — two pulse channels, triangle, and noise through a non-linear mixer to WebAudio.
- **Mappers** — NROM (0), MMC1 (1), UxROM (2), CNROM (3), MMC3 (4) with scanline IRQs.
- **NES Studio debugger** — step/run/breakpoints, live disassembly, memory editor,
  CPU + PPU state, pattern/nametable/OAM viewers, and a full-screen Play mode.
- **Verified** — 32 native (gtest) + 211 web (Vitest) tests, plus a [conformance
  suite](#conformance) that runs the industry-standard test ROMs in CI.

## Conformance

The suite in [`tests/conformance_test.cpp`](tests/conformance_test.cpp) runs the
canonical NES test ROMs through the real core and checks each ROM's self-reported
result. ROMs are fetched on demand (`tests/roms/fetch_test_roms.sh`) and are not committed.

| Test ROM | Author | Area | Status |
|----------|--------|------|:------:|
| `nestest` | kevtris | All official + illegal CPU opcodes | ✅ pass |
| `instr_test` 01-basics | blargg | Basic instruction behaviour | ✅ pass |
| `instr_test` 02-implied | blargg | Implied + unofficial opcodes | ✅ pass |
| `instr_test` official_only (16 ROMs) | blargg | Every official instruction | ✅ pass |
| `instr_timing` | blargg | Instruction cycle timing | ✅ pass |
| `ppu_vbl_nmi` | blargg | PPU VBlank/NMI **dot** timing | ⏳ roadmap |

The PPU renders per-scanline (accurate enough for commercial games and sprite-0
splits) rather than dot-by-dot, so the cycle-exact PPU timing ROMs are the current
accuracy frontier — see [`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md). The harness
skips (rather than fails) ROMs that are absent or are known accuracy gaps, so the
suite is an honest scoreboard.

```bash
./tests/roms/fetch_test_roms.sh          # download the ROMs (once)
cd native_build && ctest -R conformance  # run the scoreboard
```

## Game compatibility

| Game | Mapper | Status |
|------|:------:|--------|
| Super Mario Bros. | NROM (0) | Boots to playable 1-1; movement, jump, scroll, status-bar split |
| Super Mario Bros. 3 | MMC3 (4) | Boots, attract demo runs (scanline IRQ split) |

Load any `.nes` image with **Load ROM**; press **▶ Play** for full-screen play.
Controls: arrows = D-pad, **X** = A (jump), **Z** = B, **Enter** = Start, **Shift** = Select.

## Quick start

```bash
git clone https://github.com/MaxwellKnight/nes-emulator.git
cd nes-emulator

docker compose --profile dev up web-dev    # dev server at http://localhost:5173
docker compose run --rm dev build_wasm.sh  # build the WASM core into web/src/wasm/generated/
docker compose run --rm test               # build native + run the gtest suite
```

### Front-end (without Docker)

The web app lives in `web/` (pnpm, Node 20+):

```bash
cd web
pnpm install
pnpm dev         # Vite dev server
pnpm test        # Vitest unit + component suites
pnpm typecheck   # tsc --noEmit
pnpm build       # production bundle in web/dist
```

The compiled `cpu_wasm.js` / `cpu_wasm.wasm` are emitted into `web/src/wasm/generated/`
by the Emscripten build and are gitignored — build the WASM target before running the
front-end against the real core.

### Native build + tests (without Docker)

Requires a C++17 compiler, CMake ≥ 3.14, and GoogleTest.

```bash
cmake -B native_build -DCMAKE_BUILD_TYPE=Debug
cmake --build native_build -j
./tests/roms/fetch_test_roms.sh      # optional: enables the conformance suite
cd native_build && ctest --output-on-failure
```

## Architecture

See **[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md)** for the component breakdown, the
CPU/PPU timing model and its trade-offs, and a debugging case study (how a one-dot
rendering offset hung Super Mario Bros. on its sprite-0 split, and how the conformance
mindset caught it).

At a glance:

- **Core** (`src/`, `include/`) — `Bus` ties together `CPU`, `PPU`, `APU`, and the
  `Cartridge`/mapper; clocked 1 CPU : 3 PPU dots per step.
- **WASM bridge** (`web/src/wasm/`) — a typed TypeScript layer over the Emscripten
  exports (`load_rom`, `run_frame`, `set_controller`, framebuffer/OAM pointers, …).
- **NES Studio** (`web/src/`) — Vite + React 18 + TypeScript + Tailwind v4, state in an
  `EmulatorProvider` context with hooks for the run loop, controller, and debug views.

## License

GNU GPL v3 — see [LICENSE](LICENSE).
