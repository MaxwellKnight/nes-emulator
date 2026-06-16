"""Run a random agent through the Super Mario Bros environment for one episode and
record it to a movie. This exercises the Gymnasium-style API (reset/step, actions,
reward) rather than a fixed script.

    python examples/random_agent.py "Super Mario Bros.nes" random.nesmovie
"""

import random
import sys

from nesenv import SuperMarioBrosEnv


def main() -> int:
    if len(sys.argv) < 2:
        print("usage: random_agent.py <rom.nes> [out.nesmovie]")
        return 2
    rom = open(sys.argv[1], "rb").read()
    out = sys.argv[2] if len(sys.argv) > 2 else "random.nesmovie"

    env = SuperMarioBrosEnv(rom, frameskip=4, record=True)
    env.reset()

    total = 0.0
    rng = random.Random(0)  # seeded, so this run is reproducible too
    for _ in range(150):
        action = rng.randrange(env.num_actions)
        _obs, reward, terminated, _truncated, info = env.step(action)
        total += reward
        if terminated:
            print(f"died at progress {info['progress']}")
            break

    env.save_movie(out)
    print(f"episode reward {total:.1f}, final progress {info['progress']}")
    print(f"wrote {out} - load it with 'Watch Movie' in NES Studio")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
