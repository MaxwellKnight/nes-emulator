# nesenv: headless NES + RL environment

A Python wrapper over the C++ core. It drives the emulator from outside the browser
for scripting, training, and recording agent movies you can watch in NES Studio.
The base package is standard-library only (it talks to the C ABI via `ctypes`);
numpy and gymnasium are optional extras for the Gymnasium adapter.

## 1. Build the shared library

The Python package loads `libnesenv`, which CMake builds from the C++ core:

```bash
# from the repo root
cmake -B native_build -DCMAKE_BUILD_TYPE=Release
cmake --build native_build --target nesenv
```

That produces `native_build/libnesenv.{dylib,so,dll}`. The loader finds it
automatically; if you put it elsewhere, set `NESENV_LIB` to the full path.

## 2. Use it

Either install it, or just put the package on the path:

```bash
cd python
pip install -e .            # optional; or skip and use PYTHONPATH=.
```

```python
from nesenv import Nes, RIGHT, A

nes = Nes()
nes.load(open("Super Mario Bros.nes", "rb").read())
for _ in range(120):
    nes.step(RIGHT | A)          # advance a frame with buttons held
rgb = nes.framebuffer()          # 256x240 RGBA bytes
print("Mario X:", nes.peek(0x0086))
```

## 3. Generate a movie

A movie is the ROM plus one controller byte per frame. Because the core is
deterministic it replays exactly, so a movie is how you capture an agent.

Record a scripted agent playing Super Mario Bros:

```bash
cd python
PYTHONPATH=. python3 examples/record_scripted.py "Super Mario Bros.nes" smb.nesmovie
```

Or run the Gymnasium-style env with a (seeded) random agent and save that episode:

```bash
PYTHONPATH=. python3 examples/random_agent.py "Super Mario Bros.nes" random.nesmovie
```

There is also a C++ recorder with the exact same output:

```bash
./native_build/record_demo "Super Mario Bros.nes" smb.nesmovie
```

(The Python and C++ recorders produce byte-identical movies, which is a nice proof
that the boundary is deterministic.)

## 4. Watch it in the browser

1. Start NES Studio: `docker compose --profile dev up web-dev` (or `cd web && pnpm dev`).
2. Open http://localhost:5173.
3. Click **Watch Movie** in the top bar and choose your `.nesmovie`.

The browser loads the embedded ROM and replays the recorded inputs frame for frame.
No server, no backend; the WASM core and the recorder are the same code.

## 5. Watch an agent live from the UI

Instead of recording a movie, you can stream an agent to the browser in real time.
Start the live server (it streams one action byte per frame over Server-Sent Events,
which is plain HTTP, no extra dependencies):

```bash
cd python
PYTHONPATH=. python3 -m nesenv.live "Super Mario Bros.nes" --agent scripted --port 8000
```

Then open NES Studio and click **Spawn Agent** in the top bar. The browser connects to
the stream, loads the ROM the server sends, and re-simulates the streamed actions on its
own WASM core, so you watch the agent play live, in sync, at tiny bandwidth.

To watch an agent *learn*, swap the policy in `agent_action` (in `nesenv/live.py`) for a
training loop's current policy: train in the background, and between updates let the
agent stream a live episode. The streaming protocol does not change, so the browser side
stays exactly the same. Getting the agent to actually improve is the slow part (real RL
training needs the `gym` extras and time); the live view is instant.

## 6. The RL environment

`SuperMarioBrosEnv` is a Gymnasium-style env. `reset()` boots the ROM and advances
to gameplay; `step(action)` takes a discrete action index and returns
`(observation, reward, terminated, truncated, info)`. The reward rewards rightward
progress and penalises dying. Observations are the 2 KB RAM by default (`obs="ram"`)
or the framebuffer (`obs="rgb"`).

```python
from nesenv import SuperMarioBrosEnv

env = SuperMarioBrosEnv(rom, frameskip=4, record=True)
obs, info = env.reset()
obs, reward, terminated, truncated, info = env.step(1)   # action 1 = walk right
env.save_movie("episode.nesmovie")                       # watch it in the browser
```

### Training with PPO

`examples/train_ppo.py` trains a stable-baselines3 PPO agent on vectorised envs and,
every so often, saves a `.nesmovie` of the current policy so you can watch it improve
in NES Studio. Use a Python with torch wheels (3.11 works; 3.14 does not yet):

```bash
python3.11 -m venv .venv && . .venv/bin/activate
pip install -e ".[train]"

# smoke run (a couple of minutes; will NOT be good yet)
python examples/train_ppo.py "Super Mario Bros.nes" --timesteps 8000 --n-envs 4

# longer, with parallel processes for throughput (handles share no state)
python examples/train_ppo.py "Super Mario Bros.nes" --timesteps 2000000 --n-envs 8 --subproc
```

Watch progress by loading `ppo_out/policy_*.nesmovie` with **Watch Movie**. Be
realistic: Super Mario Bros is a hard RL benchmark, so a genuinely good agent is hours
of training plus reward tuning, not minutes. The reward, action set, and episode start
live in `nesenv/smb.py` if you want to tune them.

The minimal building block, if you want to drive PPO yourself:

```python
from stable_baselines3 import PPO
from nesenv.gymnasium_env import SmbGymEnv

env = SmbGymEnv(rom, frameskip=4, obs="ram")
model = PPO("MlpPolicy", env, verbose=1)
model.learn(total_timesteps=1_000_000)
```

## Tests

```bash
cd python
PYTHONPATH=. python3 -m unittest discover -s tests          # stdlib-only checks
NES_SMB_ROM="Super Mario Bros.nes" PYTHONPATH=. python3 -m unittest discover -s tests
```
