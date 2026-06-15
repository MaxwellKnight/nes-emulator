// web/src/wasm/types.ts
export type EmulatorStatus = "loading" | "ready" | "error";

export interface Registers {
  a: number;
  x: number;
  y: number;
  sp: number;
  pc: number;
  status: number;
}

export interface Flags {
  n: boolean;
  v: boolean;
  u: boolean;
  b: boolean;
  d: boolean;
  i: boolean;
  z: boolean;
  c: boolean;
}

export interface Stats {
  instructionCount: number;
  cycleCount: number;
}

export interface EmulatorSnapshot {
  registers: Registers;
  flags: Flags;
  stats: Stats;
  running: boolean;
}

export interface DisassembledInstruction {
  address: number;
  opcode: number;
  mnemonic: string;
  operand: number;
  formatted: string;
  bytes: number;
  cycles: number;
}

export type MemoryPageId = "zeropage" | "stack" | "ram" | "vectors";

export interface MemoryPage {
  id: MemoryPageId;
  label: string;
  start: number;
  size: number;
}

export const MEMORY_PAGES: MemoryPage[] = [
  { id: "zeropage", label: "Zero Page ($0000-$00FF)", start: 0x0000, size: 0x100 },
  { id: "stack", label: "Stack ($0100-$01FF)", start: 0x0100, size: 0x100 },
  { id: "ram", label: "RAM ($0200-$07FF)", start: 0x0200, size: 0x100 },
  { id: "vectors", label: "Vectors ($FFFA-$FFFF)", start: 0xfffa, size: 6 },
];

// bit index passed to debugger_get_status_flag (verified against current debugger.js getState)
export const FLAG_BITS = { c: 0, z: 1, i: 2, d: 3, b: 4, u: 5, v: 6, n: 7 } as const;
