"""Read and write the .nesmovie format that NES Studio's "Watch Movie" replays.

A movie is self-contained: the ROM plus one controller byte per frame. Because the
core is deterministic, replaying it (natively or in the browser) reproduces exactly
what produced it.

Layout (little-endian):
    0   b"NESMOVIE"
    8   uint32 version (1)
    12  uint32 rom_len
    16  uint32 frame_count
    20  rom bytes
    20+rom_len  input bytes (one player-1 mask per frame)
"""

from __future__ import annotations

import struct
from dataclasses import dataclass
from pathlib import Path

_MAGIC = b"NESMOVIE"


@dataclass
class Movie:
    version: int
    rom: bytes
    inputs: bytes


def write_movie(path: str | Path, rom: bytes, inputs: bytes | bytearray | list[int]) -> None:
    data = bytes(inputs)
    with open(path, "wb") as f:
        f.write(_MAGIC)
        f.write(struct.pack("<III", 1, len(rom), len(data)))
        f.write(rom)
        f.write(data)


def read_movie(path: str | Path) -> Movie:
    with open(path, "rb") as f:
        blob = f.read()
    if len(blob) < 20 or blob[:8] != _MAGIC:
        raise ValueError("not a .nesmovie file")
    version, rom_len, frames = struct.unpack_from("<III", blob, 8)
    rom = blob[20 : 20 + rom_len]
    inputs = blob[20 + rom_len : 20 + rom_len + frames]
    if len(rom) != rom_len or len(inputs) != frames:
        raise ValueError("movie is truncated")
    return Movie(version=version, rom=rom, inputs=inputs)
