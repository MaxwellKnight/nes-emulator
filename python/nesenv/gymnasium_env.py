"""Optional Gymnasium adapter so the env plugs into the wider RL ecosystem
(stable-baselines3, etc.). Requires the extras:

    pip install "gymnasium" "numpy"

It is intentionally not imported from the package __init__, so `import nesenv`
works without these dependencies.
"""

from __future__ import annotations

import gymnasium as gym
import numpy as np
from gymnasium import spaces

from .smb import SuperMarioBrosEnv


class SmbGymEnv(gym.Env):
    """A Gymnasium wrapper around SuperMarioBrosEnv.

    Observations are numpy arrays: the 2 KB RAM (obs="ram") or the 240x256x4 RGBA
    frame (obs="rgb"). Actions are a Discrete index into the SMB action set.
    """

    metadata = {"render_modes": ["rgb_array"]}

    def __init__(self, rom: bytes, frameskip: int = 4, obs: str = "ram", record: bool = False) -> None:
        self._env = SuperMarioBrosEnv(rom, frameskip=frameskip, obs=obs, record=record)
        self._obs_kind = obs
        self.action_space = spaces.Discrete(self._env.num_actions)
        if obs == "ram":
            self.observation_space = spaces.Box(0, 255, (2048,), dtype=np.uint8)
        else:
            self.observation_space = spaces.Box(0, 255, (240, 256, 4), dtype=np.uint8)

    def save_movie(self, path) -> None:
        """Save everything since the last reset as a .nesmovie (needs record=True)."""
        self._env.save_movie(path)

    def _to_np(self, raw: bytes) -> np.ndarray:
        arr = np.frombuffer(raw, dtype=np.uint8)
        return arr if self._obs_kind == "ram" else arr.reshape(240, 256, 4)

    def reset(self, *, seed=None, options=None):
        super().reset(seed=seed)
        obs, info = self._env.reset()
        return self._to_np(obs), info

    def step(self, action):
        obs, reward, terminated, truncated, info = self._env.step(int(action))
        return self._to_np(obs), reward, terminated, truncated, info

    def render(self):
        return self._to_np(self._env.nes.framebuffer())
