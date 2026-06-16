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

function makeCtx(breakpoints: number[], running = false): EmulatorContextValue {
  return {
    status: "ready",
    snapshot: null,
    breakpoints,
    running,
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
    const ids = items.map((i) => i.getAttribute("data-testid"));
    expect(ids).toEqual([
      "breakpoint-item-0x8000",
      "breakpoint-item-0xc000",
    ]);
    // Each item shows its address and an ✕ remove control.
    expect(screen.getByTestId("breakpoint-item-0x8000")).toHaveTextContent("$8000");
    expect(screen.getByTestId("breakpoint-remove-0x8000")).toHaveTextContent("✕");

    await user.click(screen.getByTestId("breakpoint-remove-0x8000"));
    // The info toast is immediate; the actual removal is animated/deferred.
    expect(addToast).toHaveBeenCalledWith("Breakpoint removed from $8000", "info");
    await vi.waitFor(() => {
      expect(removeBreakpoint).toHaveBeenCalledWith(0x8000);
    });
  });

  it("disables the input, add button, and remove buttons while running", () => {
    mockCtx = makeCtx([0x8000, 0xc000], true);
    render(<BreakpointsPanel />);
    expect(screen.getByTestId("breakpoint-input")).toBeDisabled();
    expect(screen.getByTestId("breakpoint-add")).toBeDisabled();
    expect(screen.getByTestId("breakpoint-remove-0x8000")).toBeDisabled();
    expect(screen.getByTestId("breakpoint-remove-0xc000")).toBeDisabled();
  });
});
