# Conformance test ROMs

These are standard, freely-redistributable NES test ROMs used by
`tests/conformance_test.cpp`. They are **not committed**; fetch them with:

```bash
./tests/roms/fetch_test_roms.sh
```

| ROM | Author | Tests |
|-----|--------|-------|
| `nestest.nes` | kevtris | All official + illegal CPU opcodes |
| `instr_01_basics.nes`, `instr_02_implied.nes`, `instr_official.nes` | blargg | CPU instruction behaviour (incl. unofficial) |
| `instr_timing.nes` | blargg | CPU instruction cycle timing |
| `ppu_vbl_nmi.nes` | blargg | PPU VBlank/NMI timing (accuracy roadmap) |

Source: <https://github.com/christopherpow/nes-test-roms>. ROMs remain the
property of their respective authors and are redistributable for testing.
