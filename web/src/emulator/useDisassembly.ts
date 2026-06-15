// web/src/emulator/useDisassembly.ts
import { useMemo } from "react";
import { useEmulator } from "./EmulatorProvider";
import type { DisassembledInstruction } from "../wasm/types";

const DEFAULT_BEFORE = 5;
const DEFAULT_AFTER = 30;

export function useDisassembly(
  before: number = DEFAULT_BEFORE,
  after: number = DEFAULT_AFTER,
): DisassembledInstruction[] {
  const { dbg, snapshot } = useEmulator();

  return useMemo(() => {
    if (!dbg) return [];
    return dbg.disassembleAroundPC(before, after);
    // snapshot is a dependency so the listing recomputes when PC moves.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [dbg, before, after, snapshot]);
}
