"""Record a scripted agent playing Super Mario Bros into a .nesmovie.

This uses the low-level Nes handle directly with a simple "hold right, jump on a
rhythm and when stuck" policy. It is the Python twin of tools/record_demo.cpp and,
because the core is deterministic, produces a byte-identical movie. Replay the
result in NES Studio with "Watch Movie".

    python examples/record_scripted.py "Super Mario Bros.nes" smb.nesmovie
"""

import sys

from nesenv import Nes, write_movie
from nesenv.core import A, RIGHT, START
from nesenv.smb import LIVES, OPER_MODE, PLAYER_PAGE, PLAYER_X


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: record_scripted.py <rom.nes> [out.nesmovie]")
        return 2
    rom = open(sys.argv[1], "rb").read()
    out = sys.argv[2] if len(sys.argv) > 2 else "smb_demo.nesmovie"

    nes = Nes()
    nes.load(rom)

    inputs = bytearray()
    lives0 = -1
    gameplay_start = -1
    max_progress = 0
    stall = 0

    for f in range(1100):
        mask = 0
        if f < 80:
            mask = 0
        elif f < 90:
            mask = START
        else:
            oper = nes.peek(OPER_MODE)
            progress = nes.peek(PLAYER_PAGE) * 256 + nes.peek(PLAYER_X)
            if oper == 1 and f > 250:
                if gameplay_start < 0:
                    gameplay_start = f
                    lives0 = nes.peek(LIVES)
                mask = RIGHT
                stall = stall + 1 if progress <= max_progress else 0
                if (f % 24) < 13 or stall > 4:
                    mask |= A
                max_progress = max(max_progress, progress)

        inputs.append(mask)
        nes.step(mask)

        if gameplay_start > 0 and nes.peek(LIVES) < lives0:
            print(f"agent died at frame {f}")
            break

    progress = nes.peek(PLAYER_PAGE) * 256 + nes.peek(PLAYER_X)
    write_movie(out, rom, inputs)
    print(f"recorded {len(inputs)} frames, final progress {progress}")
    print(f"wrote {out} - load it with 'Watch Movie' in NES Studio")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
