// web/src/components/RegistersPanel.test.tsx
import { render, screen } from "@testing-library/react";
import { describe, it, expect, vi } from "vitest";
import { RegistersPanel } from "./RegistersPanel";
import { FlagsBlock } from "./FlagsBlock";
import type { EmulatorContextValue } from "../emulator/EmulatorProvider";
import type { EmulatorSnapshot } from "../wasm/types";

vi.mock("../emulator/EmulatorProvider", async (orig) => {
  const actual = await orig<typeof import("../emulator/EmulatorProvider")>();
  return { ...actual, useEmulator: () => mockCtx };
});

let mockCtx: EmulatorContextValue;

function makeSnapshot(): EmulatorSnapshot {
  return {
    registers: { a: 0x0a, x: 0xff, y: 0x01, sp: 0xfd, pc: 0x0c00, status: 0x24 },
    flags: { n: false, v: false, u: true, b: false, d: false, i: true, z: false, c: false },
    stats: { instructionCount: 0, cycleCount: 0 },
    running: false,
  };
}

describe("RegistersPanel", () => {
  it("renders A/X/Y/SP as $XX and PC as $XXXX", () => {
    mockCtx = {
      status: "ready",
      snapshot: makeSnapshot(),
      breakpoints: [],
      running: false,
      dbg: null,
      actions: {} as EmulatorContextValue["actions"],
    };
    render(<RegistersPanel />);
    expect(screen.getByTestId("reg-a")).toHaveTextContent("$0A");
    expect(screen.getByTestId("reg-x")).toHaveTextContent("$FF");
    expect(screen.getByTestId("reg-y")).toHaveTextContent("$01");
    expect(screen.getByTestId("reg-sp")).toHaveTextContent("$FD");
    expect(screen.getByTestId("reg-pc")).toHaveTextContent("$0C00");
  });

  it("renders the FlagsBlock LED bank with a set flag", () => {
    mockCtx = {
      status: "ready",
      snapshot: makeSnapshot(),
      breakpoints: [],
      running: false,
      dbg: null,
      actions: {} as EmulatorContextValue["actions"],
    };
    // The Flags LED bank lives in the readout strip's FlagsBlock, not the
    // register gauges.
    render(<FlagsBlock />);
    expect(screen.getByTestId("flag-i").getAttribute("data-set")).toBe("true");
    expect(screen.getByTestId("flag-u").getAttribute("data-set")).toBe("true");
    expect(screen.getByTestId("flag-n").getAttribute("data-set")).toBe("false");
  });

  it("renders nothing of substance when there is no snapshot", () => {
    mockCtx = {
      status: "loading",
      snapshot: null,
      breakpoints: [],
      running: false,
      dbg: null,
      actions: {} as EmulatorContextValue["actions"],
    };
    render(<RegistersPanel />);
    expect(screen.queryByTestId("reg-a")).toBeNull();
  });
});
