"""Locate and load the libnesenv shared library and declare its C ABI.

The library is built by CMake (see BUILD_ENV in the top-level CMakeLists.txt). Set
NESENV_LIB to point at it explicitly, otherwise we search the usual build folders.
"""

from __future__ import annotations

import ctypes
import os
import sys
from pathlib import Path

_LIB_NAMES = {
    "darwin": "libnesenv.dylib",
    "linux": "libnesenv.so",
    "win32": "nesenv.dll",
}


def _candidate_paths() -> list[Path]:
    env = os.environ.get("NESENV_LIB")
    if env:
        return [Path(env)]
    name = _LIB_NAMES.get(sys.platform, "libnesenv.so")
    # repo root is two levels up from python/nesenv/
    root = Path(__file__).resolve().parents[2]
    return [
        root / "native_build" / name,
        root / "build" / name,
        root / name,
    ]


def _load() -> ctypes.CDLL:
    tried = []
    for path in _candidate_paths():
        tried.append(str(path))
        if path.exists():
            return ctypes.CDLL(str(path))
    raise OSError(
        "could not find libnesenv. Build it with:\n"
        "  cmake -B native_build -DCMAKE_BUILD_TYPE=Release\n"
        "  cmake --build native_build --target nesenv\n"
        "or set NESENV_LIB to the library path.\n"
        f"Looked in: {', '.join(tried)}"
    )


lib = _load()

# Signatures.
lib.nes_create.restype = ctypes.c_void_p
lib.nes_destroy.argtypes = [ctypes.c_void_p]
lib.nes_load.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ubyte), ctypes.c_int]
lib.nes_load.restype = ctypes.c_int
lib.nes_reset.argtypes = [ctypes.c_void_p]
lib.nes_step.argtypes = [ctypes.c_void_p, ctypes.c_ubyte]
lib.nes_step.restype = ctypes.c_int
lib.nes_set_controller.argtypes = [ctypes.c_void_p, ctypes.c_int, ctypes.c_ubyte]
lib.nes_framebuffer.argtypes = [ctypes.c_void_p]
lib.nes_framebuffer.restype = ctypes.POINTER(ctypes.c_ubyte)
lib.nes_framebuffer_size.argtypes = [ctypes.c_void_p]
lib.nes_framebuffer_size.restype = ctypes.c_int
lib.nes_get_ram.argtypes = [ctypes.c_void_p, ctypes.POINTER(ctypes.c_ubyte)]
lib.nes_peek.argtypes = [ctypes.c_void_p, ctypes.c_ushort]
lib.nes_peek.restype = ctypes.c_ubyte
lib.nes_frame_count.argtypes = [ctypes.c_void_p]
lib.nes_frame_count.restype = ctypes.c_uint
lib.nes_version.restype = ctypes.c_char_p
