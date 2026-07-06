# How it works

Notes on how the emulator is put together, the timing shortcuts I took and why, and a
debugging story that makes the case for testing against real ROMs better than any
amount of arguing would.

## The pieces

```
                         +--------------------------------------+
   .nes file --load_rom->| Cartridge  -->  Mapper (0/1/2/3/4)   |
                         +---------------+----------------------+
                                         | PRG / CHR / mirroring
        +-----------+   reads/writes     v
        |    CPU    |<------------+--- Bus ---+------------> PPU --> framebuffer
        |  (6502)   |             |           |                 (256x240 RGBA)
        +-----+-----+             |           +------------> APU --> sample ring
              | NMI / IRQ         |
              +-------------------+           Controllers ($4016/$4017)
```

Everything hangs off the **`Bus`** (`src/bus.cpp`). It owns the CPU, PPU, APU,
controllers, and the 2 KB of work RAM, and it's the thing that decides where a given
CPU read or write actually goes. One step of the bus clocks the CPU once and the PPU
three times. That 1:3 ratio is the real hardware, and getting it right is what keeps
video and audio in sync with the program.

The **`CPU`** (`src/cpu.cpp`) is a table-driven 6502. A 256-entry table maps each
opcode byte to an addressing mode plus an operation, which keeps the decode loop tiny.
It does the official instruction set and the illegal opcodes worth caring about (more
on those below).

The **`PPU`** (`src/ppu.cpp`) is the picture chip: the `$2000`-`$2007` registers, the
"loopy" `v`/`t`/`x`/`w` scroll registers, background and sprite drawing, sprite-0 hit,
and the NMI it kicks off at the start of vblank.

The **`APU`** (`src/apu.cpp`) runs two pulse channels, a triangle, and a noise channel
through a frame sequencer and a non-linear mixer, and produces 44.1 kHz samples that
the web layer drains once a frame and hands to WebAudio.

The **cartridge and mappers** (`src/cartridge.cpp`, `src/mapper_*.cpp`) read the iNES
header and handle bank switching, mirroring, and for MMC3 the scanline IRQ that games
like SMB3 use to split the screen.

The whole core builds into one static library that gets linked two ways: into the
native gtest binaries, and into a WASM module (`src/wasm_main.cpp`) whose C exports a
typed TypeScript layer wraps for the browser.

## Timing, and the shortcuts I took

The CPU runs an instruction at a time. `CPU::clock()` decodes a full instruction,
figures out how many cycles it should take (plus the page-cross penalty where it
applies), and then reports "busy" for that many bus steps. It doesn't split a read and
a write across separate cycles the way real silicon does. The upside is that the cycle
*count* is exact. It matches the `nestest` log to the cycle, even though the work
happens in one go.

The PPU is the bigger shortcut. It keeps proper dot/scanline/frame counters, so
vblank, NMI, the sprite-0 flag clearing, the scroll updates, and the MMC3 IRQ all fire
on the right dots. But when it's time to draw a visible line it draws the whole line
at once from the current scroll position. Real hardware produces the line pixel by
pixel.

Here's the honest version of what that buys and costs:

| | This emulator | Fully dot-accurate |
|---|---|---|
| CPU correctness | cycle-exact per instruction | same |
| CPU read/write timing within an instruction | atomic | split across cycles |
| PPU rendering | per scanline | per dot |
| Mid-scanline raster tricks | not modelled | modelled |
| Running commercial games | works well | works |
| Dot-exact PPU test ROMs | fail | pass |

I picked this point on purpose. It's enough to run real games correctly, sprite-0
status-bar splits and MMC3 raster splits included, without the per-dot PPU rewrite. A
dot-based PPU is the obvious next step, and the conformance scoreboard already has
`ppu_vbl_nmi` sitting there as a reminder.

## Illegal opcodes

The 6502 has a pile of undocumented opcodes, and real cartridges lean on them, so I
implemented the stable ones: the unofficial `NOP`s, `LAX`, `SAX`, `SBC #imm`, and the
read-modify-write combos `SLO`, `RLA`, `SRE`, `RRA`, `DCP`, `ISC`, plus
`ANC`/`ALR`/`ARR`/`AXS`/`LAS`. The genuinely cursed ones (`XAA`, `AHX`, `SHX`, `SHY`,
`TAS`, and the `KIL` jams that lock up a real chip) I register with the right length
and cycle count but don't try to reproduce their analog-dependent behaviour. The point
of that is robustness: the core should never hit an opcode it doesn't know and fall
over. A test (`all_256_opcodes_registered`) keeps me honest by checking every single
byte value is mapped.

## Testing

Three layers, fastest to slowest:

1. **Unit tests** (`tests/*.cpp`): the per-instruction CPU stuff, PPU registers and
   rendering, mappers, APU, controllers. Quick, deterministic, run constantly.
2. **Conformance ROMs** (`tests/conformance_test.cpp`): blargg's and kevtris' test
   ROMs run headless, with the harness reading back each ROM's own pass/fail report.
   This is the layer that catches the timing bugs unit tests sail right past, and it's
   the public scoreboard in the [README](../README.md#conformance).
3. **Golden trace**: on top of the pass/fail, `nestest` gets compared line by line
   against the published reference log (PC, A/X/Y/P/SP, and cycle count) for all 5003
   official-opcode instructions.

## A one-dot bug that froze Mario

This is the bug that convinced me the conformance ROMs earn their keep.

Super Mario Bros. booted, showed its title, and then after you pressed Start it loaded
the "WORLD 1-1" screen and just sat there. No Mario, blank TIME, controller did
nothing.

Running the core headless, the CPU was stuck forever at `$8150`: SMB's sprite-0-hit
wait, the loop that times the split between the fixed status bar at the top and the
scrolling level below it. Sprite 0 was on screen (`Y=24`, tile `$FF`), rendering was
on, and the hit flag still never fired. When I dumped the sprite pass pixel by pixel,
the reason showed up. The sprite's solid pixels (tile rows 5 and 6, scanlines 30 and
31) and the status bar's solid background pixels were never on the same scanline. They
were off by exactly one.

The cause was a single misplaced line. In `PPU::clock()` I was drawing the visible
scanline at dot 257, but `inc_y()`, the thing that advances the vertical scroll, had
already run at dot 256. So scanline N was getting drawn with scanline N+1's scroll
value, nudging the whole background up a pixel and breaking the sprite/background
overlap that sprite-0 hit depends on. The title screen hid it because the tile under
sprite 0 there is a solid block, so a one-pixel shift looks identical. Only the thin
glyphs in the gameplay status bar gave it away.

The fix was to draw the line before `inc_y()` runs, so it uses its own scroll
position. There's a regression test (`BackgroundUsesOwnScanlineFineY`) that pins it
down: a tile that's only opaque on its top row has to show up on scanline 0, not
scanline 1.

What sticks with me about this one is that a one-dot timing error is completely
invisible to "yeah, that looks like Mario," but it's fatal to the game's actual frame
logic. That's the exact kind of thing the test ROMs are built to catch on their own,
which is why they run in CI now instead of living in my head.
