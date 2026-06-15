// web/src/components/StatisticsPanel.test.tsx
import { render, screen } from "@testing-library/react";
import { describe, it, expect, vi } from "vitest";
import { StatisticsPanel } from "./StatisticsPanel";
import type { EmulatorContextValue } from "../emulator/EmulatorProvider";
import type { EmulatorSnapshot } from "../wasm/types";

vi.mock("../emulator/EmulatorProvider", async (orig) => {
  const actual = await orig<typeof import("../emulator/EmulatorProvider")>();
  return { ...actual, useEmulator: () => mockCtx };
});

let mockCtx: EmulatorContextValue;

function makeSnapshot(instr: number, cycles: number): EmulatorSnapshot {
  return {
    registers: { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0x0c00, status: 0x24 },
    flags: { n: false, v: false, u: true, b: false, d: false, i: true, z: false, c: false },
    stats: { instructionCount: instr, cycleCount: cycles },
    running: false,
  };
}

describe("StatisticsPanel", () => {
  it("shows instruction and cycle counts from the snapshot", () => {
    mockCtx = {
      status: "ready",
      snapshot: makeSnapshot(42, 137),
      breakpoints: [],
      running: false,
      dbg: null,
      actions: {} as EmulatorContextValue["actions"],
    };
    render(<StatisticsPanel />);
    expect(screen.getByTestId("stat-instructions")).toHaveTextContent("42");
    expect(screen.getByTestId("stat-cycles")).toHaveTextContent("137");
  });

  it("shows zeros when there is no snapshot", () => {
    mockCtx = {
      status: "loading",
      snapshot: null,
      breakpoints: [],
      running: false,
      dbg: null,
      actions: {} as EmulatorContextValue["actions"],
    };
    render(<StatisticsPanel />);
    expect(screen.getByTestId("stat-instructions")).toHaveTextContent("0");
    expect(screen.getByTestId("stat-cycles")).toHaveTextContent("0");
  });
});
