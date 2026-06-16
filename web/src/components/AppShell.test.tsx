import { describe, it, expect, vi } from "vitest";
import { render, screen, fireEvent } from "@testing-library/react";
import { AppShell } from "./AppShell";
import type { EmulatorContextValue } from "../emulator/EmulatorProvider";

vi.mock("../emulator/EmulatorProvider", async (orig) => {
  const actual = await orig<typeof import("../emulator/EmulatorProvider")>();
  return { ...actual, useEmulator: () => mockCtx };
});

vi.mock("./toast/ToastProvider", async (orig) => {
  const actual = await orig<typeof import("./toast/ToastProvider")>();
  return { ...actual, useToast: () => ({ toasts: [], addToast: vi.fn() }) };
});

let mockCtx: EmulatorContextValue;

function makeCtx(overrides: Partial<EmulatorContextValue> = {}): EmulatorContextValue {
  const actions = {
    step: vi.fn(),
    run: vi.fn(),
    stop: vi.fn(),
    reset: vi.fn(),
    addBreakpoint: vi.fn(),
    removeBreakpoint: vi.fn(),
    toggleBreakpoint: vi.fn(),
    writeMemory: vi.fn(),
    loadROM: vi.fn(),
    loadRom: vi.fn(() => 0),
    loadOpcodes: vi.fn(),
    setController: vi.fn(),
    playMovie: vi.fn(),
    stopMovie: vi.fn(),
  };
  return {
    status: "ready",
    snapshot: null,
    breakpoints: [],
    running: false,
    framebuffer: null,
    movie: { playing: false, frame: 0, total: 0 },
    dbg: null,
    actions,
    ...overrides,
  };
}

describe("AppShell", () => {
  it("renders the loading state when status is loading", () => {
    mockCtx = makeCtx({ status: "loading" });
    render(<AppShell />);
    expect(screen.getByTestId("app-loading")).toBeInTheDocument();
  });

  it("renders the error state when status is error", () => {
    mockCtx = makeCtx({ status: "error" });
    render(<AppShell />);
    expect(screen.getByTestId("app-error")).toBeInTheDocument();
  });

  it("renders the toolbar and panels when status is ready", () => {
    mockCtx = makeCtx({ status: "ready" });
    render(<AppShell />);
    expect(screen.getByTestId("app-toolbar")).toBeInTheDocument();
  });

  it("captures the keyboard for player 1 while the core is running in the cockpit", () => {
    // Regression: keyboard was only wired in Play mode, so a game Run from the
    // cockpit could not be controlled. Running in the cockpit must capture keys.
    mockCtx = makeCtx({ status: "ready", running: true });
    render(<AppShell />);
    fireEvent.keyDown(window, { code: "Enter" }); // Start (bit 0x08)
    expect(mockCtx.actions.setController).toHaveBeenCalledWith(0x08);
  });

  it("does not capture controller keys when typing into a debugger input", () => {
    // Running in the cockpit must not steal Enter/arrows from text inputs.
    mockCtx = makeCtx({ status: "ready", running: true });
    render(<AppShell />);
    const input = document.createElement("input");
    document.body.appendChild(input);
    input.focus();
    fireEvent.keyDown(input, { code: "ArrowRight" });
    expect(mockCtx.actions.setController).not.toHaveBeenCalledWith(0x80);
    input.remove();
  });

  it("releases the keyboard when the core is not running", () => {
    mockCtx = makeCtx({ status: "ready", running: false });
    render(<AppShell />);
    vi.mocked(mockCtx.actions.setController).mockClear();
    fireEvent.keyDown(window, { code: "Enter" });
    expect(mockCtx.actions.setController).not.toHaveBeenCalledWith(0x08);
  });
});
