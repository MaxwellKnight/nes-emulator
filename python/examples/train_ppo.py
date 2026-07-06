"""Train a PPO agent to play Super Mario Bros with stable-baselines3, and snapshot
the policy to a .nesmovie every so often so you can watch it improve in NES Studio.

Needs the RL extras (use a Python that has torch wheels, e.g. 3.11):

    python3.11 -m venv .venv && . .venv/bin/activate
    pip install -e ".[gym]" stable-baselines3

Quick smoke run (a couple of minutes, will NOT be good yet):

    python examples/train_ppo.py "Super Mario Bros.nes" --timesteps 8000 --n-envs 4

A real run is hours: Super Mario Bros is a hard RL benchmark. Watch progress by
loading the snapshot movies (out/policy_*.nesmovie) with "Watch Movie".
"""

from __future__ import annotations

import argparse
from pathlib import Path

from stable_baselines3 import PPO
from stable_baselines3.common.callbacks import BaseCallback
from stable_baselines3.common.vec_env import DummyVecEnv, SubprocVecEnv

from nesenv.gymnasium_env import SmbGymEnv


def make_env(rom_path: str, frameskip: int):
    """A picklable env factory (SB3 vectorises by calling these)."""

    def thunk():
        rom = open(rom_path, "rb").read()
        return SmbGymEnv(rom, frameskip=frameskip, obs="ram")

    return thunk


class SnapshotMovie(BaseCallback):
    """Every `every` steps, roll out the current greedy policy and save it as a
    .nesmovie plus a model checkpoint, so training progress is watchable."""

    def __init__(self, rom_path: str, out_dir: Path, every: int, frameskip: int):
        super().__init__()
        self.rom_path = rom_path
        self.out_dir = out_dir
        self.every = every
        self.frameskip = frameskip
        self._next = every

    def _record(self) -> int:
        rom = open(self.rom_path, "rb").read()
        env = SmbGymEnv(rom, frameskip=self.frameskip, obs="ram", record=True)
        obs, _ = env.reset()
        progress = 0
        for _ in range(1500):
            action, _ = self.model.predict(obs, deterministic=True)
            obs, _r, terminated, truncated, info = env.step(int(action))
            progress = info["progress"]
            if terminated or truncated:
                break
        movie = self.out_dir / f"policy_{self.num_timesteps}.nesmovie"
        env.save_movie(movie)
        return progress

    def _on_step(self) -> bool:
        if self.num_timesteps >= self._next:
            self._next += self.every
            progress = self._record()
            self.model.save(self.out_dir / f"ppo_{self.num_timesteps}")
            print(f"[snapshot] step {self.num_timesteps}: greedy policy reached progress {progress}")
        return True


def main() -> int:
    p = argparse.ArgumentParser(description="Train PPO on Super Mario Bros.")
    p.add_argument("rom")
    p.add_argument("--timesteps", type=int, default=100_000)
    p.add_argument("--n-envs", type=int, default=8)
    p.add_argument("--frameskip", type=int, default=4)
    p.add_argument("--out", default="ppo_out")
    p.add_argument("--snapshot-every", type=int, default=20_000)
    p.add_argument("--subproc", action="store_true", help="run envs in separate processes")
    args = p.parse_args()

    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    factories = [make_env(args.rom, args.frameskip) for _ in range(args.n_envs)]
    vec = SubprocVecEnv(factories) if args.subproc else DummyVecEnv(factories)

    model = PPO("MlpPolicy", vec, n_steps=512, batch_size=256, verbose=1)
    callback = SnapshotMovie(args.rom, out, args.snapshot_every, args.frameskip)
    model.learn(total_timesteps=args.timesteps, callback=callback)

    model.save(out / "ppo_final")
    print(f"done. checkpoints and snapshot movies are in {out}/")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
