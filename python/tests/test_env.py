"""Stdlib-only tests for the nesenv package. Run with:

    PYTHONPATH=. python3 -m unittest discover -s tests

They use a tiny synthetic NROM image, so there is no dependency on any commercial
ROM. The SMB-specific check runs only if NES_SMB_ROM points at a Super Mario Bros
image.
"""

import os
import tempfile
import unittest

from nesenv import Nes, read_movie, version, write_movie


def synthetic_rom() -> bytes:
    rom = bytearray(16 + 16384 + 8192)
    rom[0:4] = b"NES\x1a"
    rom[4] = 1  # 16KB PRG
    rom[5] = 1  # 8KB CHR
    prg = 16
    rom[prg + 0x0000 : prg + 0x0003] = bytes([0x4C, 0x00, 0xC0])  # JMP $C000
    rom[prg + 0x3FFC : prg + 0x3FFE] = bytes([0x00, 0xC0])  # reset vector
    return bytes(rom)


class TestCore(unittest.TestCase):
    def test_version(self):
        self.assertTrue(version().startswith("nes_env"))

    def test_load_and_step(self):
        nes = Nes()
        nes.load(synthetic_rom())
        self.assertEqual(nes.frame_count(), 0)
        for _ in range(10):
            nes.step(0)
        self.assertEqual(nes.frame_count(), 10)
        self.assertEqual(len(nes.ram()), 2048)
        self.assertEqual(len(nes.framebuffer()), 256 * 240 * 4)

    def test_rejects_junk(self):
        nes = Nes()
        with self.assertRaises(ValueError):
            nes.load(b"\xff" * 64)

    def test_determinism_two_instances(self):
        rom = synthetic_rom()
        a, b = Nes(), Nes()
        a.load(rom)
        b.load(rom)
        seq = [0, 0x80, 0x81, 0x08, 0x10, 0x40]
        for f in range(120):
            a.step(seq[f % len(seq)])
            b.step(seq[f % len(seq)])
            self.assertEqual(a.framebuffer(), b.framebuffer(), f"diverged at frame {f}")

    def test_reset_replays_identically(self):
        rom = synthetic_rom()
        nes = Nes()
        nes.load(rom)
        seq = [0, 0x80, 0x01, 0x08]
        for i in range(40):
            nes.step(seq[i % 4])
        first = nes.ram()
        nes.reset()
        for i in range(40):
            nes.step(seq[i % 4])
        self.assertEqual(first, nes.ram())

    def test_movie_roundtrip(self):
        rom = synthetic_rom()
        inputs = bytes([0, 0x80, 0x81, 0x08, 0x40])
        with tempfile.TemporaryDirectory() as d:
            path = os.path.join(d, "x.nesmovie")
            write_movie(path, rom, inputs)
            m = read_movie(path)
            self.assertEqual(m.version, 1)
            self.assertEqual(m.rom, rom)
            self.assertEqual(m.inputs, inputs)


@unittest.skipUnless(os.environ.get("NES_SMB_ROM"), "set NES_SMB_ROM to run SMB checks")
class TestSmb(unittest.TestCase):
    def test_moving_right_is_rewarded(self):
        from nesenv import SuperMarioBrosEnv
        from nesenv.smb import ACTIONS

        rom = open(os.environ["NES_SMB_ROM"], "rb").read()
        env = SuperMarioBrosEnv(rom, frameskip=4)
        env.reset()
        right = ACTIONS.index(0x80)
        total = sum(env.step(right)[1] for _ in range(40))
        self.assertGreater(total, 0.0, "walking right should yield positive reward")


if __name__ == "__main__":
    unittest.main()
