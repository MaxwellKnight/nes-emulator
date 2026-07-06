"""nesenv - a deterministic, headless NES environment built on the C++ core.

    from nesenv import Nes, SuperMarioBrosEnv, write_movie

`Nes` is the low-level handle; `SuperMarioBrosEnv` is a Gymnasium-style RL env;
`write_movie` saves a .nesmovie you can replay in the browser ("Watch Movie").
"""

from .core import Nes, version
from .core import A, B, SELECT, START, UP, DOWN, LEFT, RIGHT
from .env import NesEnv
from .smb import SuperMarioBrosEnv, ACTIONS as SMB_ACTIONS
from .movie import Movie, read_movie, write_movie

__all__ = [
    "Nes",
    "version",
    "NesEnv",
    "SuperMarioBrosEnv",
    "SMB_ACTIONS",
    "Movie",
    "read_movie",
    "write_movie",
    "A",
    "B",
    "SELECT",
    "START",
    "UP",
    "DOWN",
    "LEFT",
    "RIGHT",
]
