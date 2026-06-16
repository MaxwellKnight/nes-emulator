// web/src/emulator/useMemory.ts
import { useMemo } from "react";
import { useEmulator } from "./EmulatorProvider";
import type { MemoryPage } from "../wasm/types";

const ROW_WIDTH = 16;

function asciiOf(byte: number): string {
  return byte >= 32 && byte <= 126 ? String.fromCharCode(byte) : ".";
}

export interface MemoryRow {
  address: number;
  bytes: number[];
  ascii: string;
}

/**
 * Reads memory as 16-byte rows starting at the page's base address.
 *
 * When `rowCount` is provided the view renders exactly that many contiguous
 * rows — continuing past the page boundary if needed so the panel fills its
 * measured height with no empty bottom. The page <select> only sets the base
 * address; scroll/jump navigate within the contiguous range.
 *
 * When `rowCount` is omitted the view falls back to the page's declared size
 * (e.g. 256 bytes = 16 rows, the 6-byte vectors page = 1 partial row), wrapping
 * the read at the 16-bit address space.
 */
export function useMemory(
  page: MemoryPage,
  rowCount?: number,
): { rows: MemoryRow[] } {
  const { dbg, snapshot } = useEmulator();

  const rows = useMemo(() => {
    if (!dbg) return [];
    const out: MemoryRow[] = [];

    if (rowCount === undefined) {
      // page-size mode (preserves existing per-page behaviour)
      const end = page.start + page.size - 1;
      const flat = dbg.readMemoryRange(page.start, end);
      for (let offset = 0; offset < flat.length; offset += ROW_WIDTH) {
        const bytes = flat.slice(offset, offset + ROW_WIDTH);
        out.push({
          address: page.start + offset,
          bytes,
          ascii: bytes.map(asciiOf).join(""),
        });
      }
      return out;
    }

    // fill mode: exactly `rowCount` contiguous 16-byte rows from the base,
    // wrapping the read address inside the 16-bit space.
    for (let r = 0; r < rowCount; r += 1) {
      const rowAddr = (page.start + r * ROW_WIDTH) & 0xffff;
      const bytes: number[] = [];
      for (let i = 0; i < ROW_WIDTH; i += 1) {
        bytes.push(dbg.readMemory((rowAddr + i) & 0xffff) & 0xff);
      }
      out.push({
        address: rowAddr,
        bytes,
        ascii: bytes.map(asciiOf).join(""),
      });
    }
    return out;
    // snapshot is intentionally a dependency so rows recompute after each
    // step/write/reset that publishes a fresh snapshot.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [dbg, page.start, page.size, rowCount, snapshot]);

  return { rows };
}
