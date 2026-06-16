// Parser for the .nesmovie format produced by the headless recorder
// (tools/record_demo, python/nesenv). A movie is fully self-contained and
// deterministic: it carries the ROM plus one controller byte per frame, so the
// browser can replay exactly what the agent did, frame for frame.
//
// Layout (little-endian):
//   0   "NESMOVIE" (8 bytes magic)
//   8   u32 version (currently 1)
//   12  u32 rom_len
//   16  u32 frame_count
//   20  rom bytes (rom_len)
//   20+rom_len  input bytes (frame_count, one P1 button mask per frame)

export interface NesMovie {
  version: number;
  rom: Uint8Array;
  inputs: Uint8Array; // one byte per frame, player-1 button bitmask
}

const MAGIC = "NESMOVIE";

export function parseMovie(bytes: Uint8Array): NesMovie {
  if (bytes.length < 20) throw new Error("movie too small");
  const magic = new TextDecoder("ascii").decode(bytes.subarray(0, 8));
  if (magic !== MAGIC) throw new Error("not a .nesmovie file");

  const view = new DataView(bytes.buffer, bytes.byteOffset, bytes.byteLength);
  const version = view.getUint32(8, true);
  const romLen = view.getUint32(12, true);
  const frameCount = view.getUint32(16, true);

  const romStart = 20;
  const inputStart = romStart + romLen;
  if (inputStart + frameCount > bytes.length) {
    throw new Error("movie is truncated");
  }

  return {
    version,
    rom: bytes.subarray(romStart, inputStart),
    inputs: bytes.subarray(inputStart, inputStart + frameCount),
  };
}
