"""A small, Gymnasium-style environment base built on the deterministic core.

It deliberately uses only the standard library: observations are returned as bytes
(RAM) or RGBA bytes (the framebuffer). The optional Gymnasium adapter in
gymnasium_env.py wraps this for the wider RL ecosystem.

A subclass provides the game-specific reset() (how an episode starts) and the
reward/termination logic inside step(); everything else is generic.
"""

from __future__ import annotations

from pathlib import Path

from .core import Nes
from .movie import write_movie


class NesEnv:
    """Base environment. action = an index into `actions` (a list of button masks)."""

    def __init__(
        self,
        rom: bytes,
        actions: list[int],
        frameskip: int = 1,
        obs: str = "ram",
        record: bool = False,
    ) -> None:
        if obs not in ("ram", "rgb"):
            raise ValueError("obs must be 'ram' or 'rgb'")
        self.rom = bytes(rom)
        self.actions = list(actions)
        self.frameskip = max(1, frameskip)
        self.obs_kind = obs
        self.record = record
        self.history = bytearray()
        self.nes = Nes()
        self.nes.load(self.rom)

    @property
    def num_actions(self) -> int:
        return len(self.actions)

    def _apply(self, mask: int) -> None:
        """Advance one frame with `mask` held, recording it if requested."""
        self.nes.step(mask)
        if self.record:
            self.history.append(mask & 0xFF)

    def _obs(self) -> bytes:
        return self.nes.ram() if self.obs_kind == "ram" else self.nes.framebuffer()

    def save_movie(self, path: str | Path) -> None:
        """Write everything applied since the last reset as a .nesmovie."""
        write_movie(path, self.rom, bytes(self.history))

    # Subclasses override these two.
    def reset(self) -> tuple[bytes, dict]:
        raise NotImplementedError

    def step(self, action: int) -> tuple[bytes, float, bool, bool, dict]:
        raise NotImplementedError
