"""A Super Mario Bros environment: observation, a small action set, and a reward
that rewards moving right and staying alive.

Reset boots the ROM and auto-advances through the title and the WORLD 1-1 card so
every episode begins in gameplay, deterministically. When record=True, every frame
(including the boot sequence) is captured, so save_movie() produces a self-contained
.nesmovie you can replay in the browser.
"""

from __future__ import annotations

from . import core
from .env import NesEnv

# SMB RAM locations.
OPER_MODE = 0x0770   # 1 once a level is running
PLAYER_X = 0x0086    # on-screen X within the page
PLAYER_PAGE = 0x006D  # horizontal page
LIVES = 0x075A

# A compact, useful action set (button masks).
ACTIONS = [
    0,                                  # 0 NOOP
    core.RIGHT,                         # 1 walk right
    core.RIGHT | core.A,               # 2 jump right
    core.RIGHT | core.B,               # 3 run right
    core.RIGHT | core.A | core.B,      # 4 run-jump right
    core.A,                            # 5 jump in place
    core.LEFT,                         # 6 walk left
    core.DOWN,                         # 7 crouch
]


class SuperMarioBrosEnv(NesEnv):
    def __init__(self, rom: bytes, frameskip: int = 1, obs: str = "ram", record: bool = False) -> None:
        super().__init__(rom, actions=ACTIONS, frameskip=frameskip, obs=obs, record=record)
        self._progress = 0
        self._lives = 0

    def _progress_now(self) -> int:
        return self.nes.peek(PLAYER_PAGE) * 256 + self.nes.peek(PLAYER_X)

    def reset(self) -> tuple[bytes, dict]:
        self.nes.reset()
        if self.record:
            self.history = bytearray()
        # Boot, press Start, then idle through the WORLD 1-1 card until controllable.
        for _ in range(80):
            self._apply(0)
        for _ in range(10):
            self._apply(core.START)
        frame = 90
        while frame < 600:
            self._apply(0)
            frame += 1
            if self.nes.peek(OPER_MODE) == 1 and frame > 250:
                break
        self._progress = self._progress_now()
        self._lives = self.nes.peek(LIVES)
        return self._obs(), {"progress": self._progress}

    def step(self, action: int) -> tuple[bytes, float, bool, bool, dict]:
        mask = self.actions[action]
        for _ in range(self.frameskip):
            self._apply(mask)

        progress = self._progress_now()
        # Reward forward progress (clamped so a page wrap can't spike it), with a
        # small time penalty and a big penalty for dying.
        reward = max(-15.0, min(15.0, float(progress - self._progress))) - 0.1
        self._progress = progress

        lives = self.nes.peek(LIVES)
        terminated = lives < self._lives
        if terminated:
            reward -= 15.0

        info = {"progress": progress, "x": self.nes.peek(PLAYER_X), "lives": lives}
        return self._obs(), reward, terminated, False, info
