// web/src/components/BreakpointsPanel.test.tsx
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { describe, it, expect, vi, beforeEach } from "vitest";
import { BreakpointsPanel } from "./BreakpointsPanel";
import type { EmulatorContextValue } from "../emulator/EmulatorProvider";

vi.mock("../emulator/EmulatorProvider", async (orig) => {
  const actual = await orig<typeof import("../emulator/EmulatorProvider")>();
  return { ...actual, useEmulator: () => mockCtx };
});

const addToast = vi.fn();
vi.mock("./toast/ToastProvider", () => ({
  useToast: () => ({ toasts: [], addToast }),
}));

let mockCtx: EmulatorContextValue;
const addBreakpoint = vi.fn();
const removeBreakpoint = vi.fn();

function makeCtx(breakpoints: number[]): EmulatorContextValue {
  return {
    status: "ready",
    snapshot: null,
    breakpoints,
    running: false,
    dbg: null,
    actions: { addBreakpoint, removeBreakpoint } as unknown as EmulatorContextValue["actions"],
  };
}

beforeEach(() => {
  addToast.mockClear();
  addBreakpoint.mockClear();
  removeBreakpoint.mockClear();
});

describe("BreakpointsPanel", () => {
  it("shows the empty state when there are no breakpoints", () => {
    mockCtx = makeCtx([]);
    render(<BreakpointsPanel />);
    expect(screen.getByText("No breakpoints set")).toBeInTheDocument();
  });

  it("adds a valid breakpoint and raises a success toast", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx([]);
    render(<BreakpointsPanel />);
    await user.type(screen.getByTestId("breakpoint-input"), "C000");
    await user.click(screen.getByTestId("breakpoint-add"));
    expect(addBreakpoint).toHaveBeenCalledWith(0xc000);
    expect(addToast).toHaveBeenCalledWith("Breakpoint added at $C000", "success");
  });

  it("rejects invalid input with a danger toast", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx([]);
    render(<BreakpointsPanel />);
    await user.type(screen.getByTestId("breakpoint-input"), "ZZZZ");
    await user.click(screen.getByTestId("breakpoint-add"));
    expect(addBreakpoint).not.toHaveBeenCalled();
    expect(addToast).toHaveBeenCalledWith("Invalid breakpoint address", "danger");
  });

  it("rejects out-of-range input with a danger toast", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx([]);
    render(<BreakpointsPanel />);
    await user.type(screen.getByTestId("breakpoint-input"), "10000");
    await user.click(screen.getByTestId("breakpoint-add"));
    expect(addBreakpoint).not.toHaveBeenCalled();
    expect(addToast).toHaveBeenCalledWith("Invalid breakpoint address", "danger");
  });

  it("lists breakpoints sorted ascending with a remove control", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx([0xc000, 0x8000]);
    render(<BreakpointsPanel />);
    const items = screen.getAllByTestId(/^breakpoint-item-/);
    expect(items.map((i) => i.textContent)).toEqual(
      expect.arrayContaining(["$8000Remove", "$C000Remove"]),
    );
    await user.click(screen.getByTestId("breakpoint-remove-0x8000"));
    expect(removeBreakpoint).toHaveBeenCalledWith(0x8000);
    expect(addToast).toHaveBeenCalledWith("Breakpoint removed from $8000", "info");
  });
});
