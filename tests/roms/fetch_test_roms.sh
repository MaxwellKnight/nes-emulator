#!/usr/bin/env bash
# Fetch the canonical NES test ROMs used by the conformance suite
# (tests/conformance_test.cpp). These ROMs are freely redistributable by their
# authors (kevtris' nestest, blargg's instr_test); we pull them on demand rather
# than committing binaries. Source: christopherpow/nes-test-roms.
set -euo pipefail
cd "$(dirname "$0")"
BASE="https://raw.githubusercontent.com/christopherpow/nes-test-roms/master"

# local name | remote path
ROMS=(
  "nestest.nes|other/nestest.nes"
  "instr_01_basics.nes|instr_test-v5/rom_singles/01-basics.nes"
  "instr_02_implied.nes|instr_test-v5/rom_singles/02-implied.nes"
  "instr_official.nes|instr_test-v5/official_only.nes"
  "instr_timing.nes|instr_timing/instr_timing.nes"
  "ppu_vbl_nmi.nes|ppu_vbl_nmi/ppu_vbl_nmi.nes"
)
for entry in "${ROMS[@]}"; do
  name="${entry%%|*}"; path="${entry#*|}"
  if [ -f "$name" ]; then echo "have  $name"; continue; fi
  echo "fetch $name"
  curl -fsSL --retry 3 -o "$name" "$BASE/$path" || echo "  WARN: failed to fetch $name"
done
echo "done."
