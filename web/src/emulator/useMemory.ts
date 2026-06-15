// web/src/emulator/useMemory.ts
import { useMemo } from "react";
import { useEmulator } from "./EmulatorProvider";
import type { MemoryPage } from "../wasm/types";

const ROW_WIDTH = 16;

function asciiOf(byte: number): string {
  return byte >= 32 && byte <= 126 ? String.fromCharCode(byte) : ".";
}

export function useMemory(page: MemoryPage): {
  rows: Array<{ address: number; bytes: number[]; ascii: string }>;
} {
  const { dbg, snapshot } = useEmulator();

  const rows = useMemo(() => {
    if (!dbg) return [];
    const end = page.start + page.size - 1;
    const flat = dbg.readMemoryRange(page.start, end);
    const out: Array<{ address: number; bytes: number[]; ascii: string }> = [];
    for (let offset = 0; offset < flat.length; offset += ROW_WIDTH) {
      const bytes = flat.slice(offset, offset + ROW_WIDTH);
      out.push({
        address: page.start + offset,
        bytes,
        ascii: bytes.map(asciiOf).join(""),
      });
    }
    return out;
    // snapshot is intentionally a dependency so rows recompute after each
    // step/write/reset that publishes a fresh snapshot.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [dbg, page.start, page.size, snapshot]);

  return { rows };
}
