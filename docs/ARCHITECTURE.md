# Architecture

This document explains how the emulator is put together, the timing model it uses
(and the trade-offs behind it), and a debugging case study that shows the
conformance-first mindset in action.

## Component overview

```
                         ┌──────────────────────────────────────┐
   .nes file ──load_rom─▶│ Cartridge  ──▶  Mapper (0/1/2/3/4)    │
                         └───────────────┬──────────────────────┘
                                         │ PRG / CHR / mirroring
        ┌───────────┐   reads/writes     ▼
        │    CPU    │◀────────────┬─── Bus ───┬────────────▶ PPU ──▶ framebuffer
        │  (6502)   │             │           │                 (256×240 RGBA)
        └─────┬─────┘             │           └────────────▶ APU ──▶ sample ring
              │ NMI / IRQ         │
              └───────────────────┘           Controllers ($4016/$4017)
```

- **`Bus`** (`src/bus.cpp`) owns the CPU, PPU, APU, controllers, and 2 KB of work
  RAM, and routes every CPU read/write to the right device. One "system step"
  clocks the CPU once and the PPU three times (the NES's 1:3 ratio), then the APU,
  then delivers any pending NMI/IRQ.
- **`CPU`** (`src/cpu.cpp`) is a table-driven 6502: a 256-entry instruction table
  maps each opcode to an addressing-mode handler + an operation. It implements the
  official opcodes and the documented unofficial ones (see [below](#unofficial-opcodes)).
- **`PPU`** (`src/ppu.cpp`) implements the `$2000`–`$2007` registers, the loopy
  `v`/`t`/`x`/`w` scroll model, background + sprite rendering, sprite-0 hit, and NMI
  generation.
- **`APU`** (`src/apu.cpp`) runs two pulse channels, triangle, and noise through a
  frame sequencer and a non-linear mixer, producing 44.1 kHz samples drained each frame.
- **`Cartridge` + mappers** (`src/cartridge.cpp`, `src/mapper_*.cpp`) parse the iNES
  header and bank-switch PRG/CHR, set mirroring, and (for MMC3) raise scanline IRQs.

The whole core compiles to a static library that is linked twice: into the native
gtest binaries, and into a WebAssembly module (`src/wasm_main.cpp`) whose C exports
are wrapped by a typed TypeScript bridge in `web/src/wasm/`.

## Execution & timing model

The CPU is **instruction-stepped with a cycle budget**: `CPU::clock()` decodes a
whole instruction, sets `_cycles` to its documented length (plus a page-cross
penalty where applicable), and then reports "busy" for that many system steps. The
`Bus` clocks the PPU 3× per CPU step, so PPU time advances at the correct ratio even
though the CPU executes an instruction atomically.

The PPU is **per-scanline**: it tracks dot/scanline/frame counters every step (so
VBlank, NMI, sprite-0-hit clearing, loopy `v` updates, and the MMC3 IRQ clock all
fire at the right dots), but it rasterises an entire visible line in one shot using
the current scroll address.

### Trade-offs

| Property | This emulator | Fully cycle-accurate |
|----------|---------------|----------------------|
| CPU correctness | Cycle-exact per instruction (matches `nestest` to the cycle) | identical |
| CPU mid-instruction bus timing | atomic per instruction | each read/write on its own cycle |
| PPU rendering | per-scanline | per-dot |
| Mid-scanline raster effects | not modelled | modelled |
| Commercial-game compatibility | high (sprite-0 splits, scrolling, IRQs work) | highest |
| `ppu_vbl_nmi` / dot-exact PPU ROMs | fail | pass |

This is a deliberate point on the accuracy/complexity curve: it is more than enough
to run commercial games correctly — including Super Mario Bros.'s sprite-0 status-bar
split and MMC3 scanline-IRQ raster splits — while keeping the renderer simple. The
documented next step is a dot-based PPU, which the conformance scoreboard
(`ppu_vbl_nmi`) already tracks.

## Unofficial opcodes

Real cartridges (and blargg's `instr_test`) use the 6502's "illegal" opcodes, so the
core implements the stable ones — the unofficial `NOP`s, `LAX`, `SAX`, `SBC #imm`,
and the read-modify-write/ALU combos `SLO`, `RLA`, `SRE`, `RRA`, `DCP`, `ISC`, plus
`ANC`/`ALR`/`ARR`/`AXS`/`LAS`. The genuinely unstable opcodes (`XAA`, `AHX`, `SHX`,
`SHY`, `TAS`, the `KIL` jams) are registered with the correct length and cycle count
so the program counter stays aligned and **the core never aborts on an undefined
opcode** — a `all_256_opcodes_registered` test enforces full coverage.

## Verification strategy

Three layers, fastest first:

1. **Unit tests** (`tests/*.cpp`, gtest) — per-instruction CPU behaviour, PPU
   registers/rendering/sprites, mappers, APU, controllers. Fast and deterministic.
2. **Conformance ROMs** (`tests/conformance_test.cpp`) — the industry-standard
   `nestest`/`blargg` test ROMs, run headless and checked against each ROM's
   self-reported pass/fail. This is what catches subtle timing bugs that unit tests
   miss; it doubles as the public [scoreboard](../README.md#conformance).
3. **Differential / golden traces** — `nestest` is additionally compared
   line-by-line against the published golden log (PC + A/X/Y/P/SP + cycle count) for
   all 5003 official-opcode instructions.

## Case study: the one-dot offset that hung Super Mario Bros.

A good illustration of why the conformance mindset matters.

**Symptom.** SMB booted, showed the title, and — after pressing Start — loaded the
"WORLD 1-1" screen but froze: no player sprite, a blank TIME, and a dead controller.

**Investigation.** Driving the core headless showed the CPU spinning forever at
`$8150`, SMB's *sprite-0-hit wait* — the loop that times the split between the fixed
status bar and the scrolling playfield. Sprite 0 was on-screen (`Y=24`, tile `$FF`),
rendering was enabled, yet the hit flag never set. Instrumenting the sprite pass
revealed why: the sprite's opaque pixels (tile rows 5–6, scanlines 30–31) and the
status bar's opaque background pixels never coincided — they were misaligned by
exactly **one scanline**.

**Root cause.** In `PPU::clock()`, the visible line was rasterised at dot 257 — but
`inc_y()` had already advanced the loopy `v` register at dot 256. So scanline *N* was
being drawn with scanline *N+1*'s vertical scroll, shifting the whole background up
one line and breaking the sprite/background overlap that sprite-0 hit depends on.
(At the title screen the block under sprite 0 is solid, so the off-by-one was
invisible there — only gameplay's thin status-bar glyphs exposed it.)

**Fix.** Rasterise each line *before* `inc_y()` advances the scroll, so it uses its
own vertical position ([`src/ppu.cpp`](../src/ppu.cpp)). A regression test
(`BackgroundUsesOwnScanlineFineY`) pins the behaviour: a tile opaque only on its top
row must render on scanline 0, not scanline 1.

**Takeaway.** A one-dot timing error is invisible to "does it look like Mario?" but
fatal to a real game's frame logic. The same class of bug is exactly what the
conformance ROMs exist to catch automatically — which is why they run in CI.
