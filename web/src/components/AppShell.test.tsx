import { describe, it, expect, vi } from "vitest";
import { render, screen } from "@testing-library/react";
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
  };
  return {
    status: "ready",
    snapshot: null,
    breakpoints: [],
    running: false,
    framebuffer: null,
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
});
