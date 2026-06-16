// web/src/components/StackPanel.test.tsx
import { render, screen } from "@testing-library/react";
import { describe, it, expect, vi } from "vitest";
import { StackPanel } from "./StackPanel";
import type { EmulatorContextValue } from "../emulator/EmulatorProvider";
import type { EmulatorSnapshot } from "../wasm/types";
import type { Debugger } from "../wasm/bridge";

vi.mock("../emulator/EmulatorProvider", async (orig) => {
  const actual = await orig<typeof import("../emulator/EmulatorProvider")>();
  return { ...actual, useEmulator: () => mockCtx };
});

let mockCtx: EmulatorContextValue;

function makeSnapshot(sp: number): EmulatorSnapshot {
  return {
    registers: { a: 0, x: 0, y: 0, sp, pc: 0x0c00, status: 0x24 },
    flags: { n: false, v: false, u: true, b: false, d: false, i: true, z: false, c: false },
    stats: { instructionCount: 0, cycleCount: 0 },
    running: false,
  };
}

function makeDbg(): Debugger {
  return {
    readMemory: (addr: number) => addr & 0xff,
  } as unknown as Debugger;
}

describe("StackPanel", () => {
  it("labels the stack pointer as $01<SP>", () => {
    mockCtx = {
      status: "ready",
      snapshot: makeSnapshot(0xfb),
      breakpoints: [],
      running: false,
      dbg: makeDbg(),
      actions: {} as EmulatorContextValue["actions"],
    };
    render(<StackPanel />);
    expect(screen.getByTestId("stack-pointer-label")).toHaveTextContent("$01FB");
  });

  it("renders stack bytes read around SP via the bridge", () => {
    mockCtx = {
      status: "ready",
      snapshot: makeSnapshot(0xfd),
      breakpoints: [],
      running: false,
      dbg: makeDbg(),
      actions: {} as EmulatorContextValue["actions"],
    };
    render(<StackPanel />);
    // SP row: address $01FD (sr-only), byte = 0xFD rendered as "FD"
    const spRow = screen.getByTestId("stack-row-0x01fd");
    expect(spRow).toHaveTextContent("$01FD");
    expect(spRow).toHaveTextContent("FD");
    expect(spRow.getAttribute("data-current")).toBe("true");
  });

  it("renders nothing when dbg or snapshot is missing", () => {
    mockCtx = {
      status: "loading",
      snapshot: null,
      breakpoints: [],
      running: false,
      dbg: null,
      actions: {} as EmulatorContextValue["actions"],
    };
    render(<StackPanel />);
    expect(screen.queryByTestId("stack-pointer-label")).toBeNull();
  });
});
