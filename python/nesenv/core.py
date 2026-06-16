"""A thin, safe Python handle over the C ABI in nes_env.h.

One Nes object is one independent emulator. It is deterministic: the same ROM and
the same inputs always produce the same frames, on any machine.
"""

from __future__ import annotations

import ctypes

from ._native import lib

RAM_SIZE = 0x0800
FRAMEBUFFER_SIZE = 256 * 240 * 4

# Controller button bits (match the hardware shift order).
A = 0x01
B = 0x02
SELECT = 0x04
START = 0x08
UP = 0x10
DOWN = 0x20
LEFT = 0x40
RIGHT = 0x80


class Nes:
    """An independent NES. Call load() before stepping."""

    def __init__(self) -> None:
        self._h = lib.nes_create()
        if not self._h:
            raise MemoryError("nes_create failed")

    def close(self) -> None:
        if getattr(self, "_h", None):
            lib.nes_destroy(self._h)
            self._h = None

    def __del__(self) -> None:
        self.close()

    def load(self, rom: bytes) -> None:
        buf = (ctypes.c_ubyte * len(rom)).from_buffer_copy(rom)
        status = lib.nes_load(self._h, buf, len(rom))
        if status != 0:
            raise ValueError(f"nes_load rejected the ROM (iNES status {status})")

    def reset(self) -> None:
        """Power-on reboot of the loaded ROM (fully deterministic)."""
        lib.nes_reset(self._h)

    def step(self, buttons: int = 0) -> int:
        """Set player 1's buttons and advance one frame. Returns the frame reason."""
        return lib.nes_step(self._h, buttons & 0xFF)

    def ram(self) -> bytes:
        """The 2 KB of CPU work RAM ($0000-$07FF)."""
        buf = (ctypes.c_ubyte * RAM_SIZE)()
        lib.nes_get_ram(self._h, buf)
        return bytes(buf)

    def peek(self, addr: int) -> int:
        return lib.nes_peek(self._h, addr & 0xFFFF)

    def framebuffer(self) -> bytes:
        """The current frame as 256*240 RGBA bytes."""
        ptr = lib.nes_framebuffer(self._h)
        return ctypes.string_at(ptr, FRAMEBUFFER_SIZE)

    def frame_count(self) -> int:
        return lib.nes_frame_count(self._h)


def version() -> str:
    return lib.nes_version().decode()
