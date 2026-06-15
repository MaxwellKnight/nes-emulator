// web/src/components/MemoryPanel.test.tsx
import { render, screen, fireEvent } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { describe, it, expect, vi, beforeEach } from "vitest";
import { MemoryPanel } from "./MemoryPanel";
import type { EmulatorContextValue } from "../emulator/EmulatorProvider";
import type { MemoryPage } from "../wasm/types";

let mockRows: Array<{ address: number; bytes: number[]; ascii: string }>;
vi.mock("../emulator/useMemory", () => ({
  useMemory: (_page: MemoryPage) => ({ rows: mockRows }),
}));

vi.mock("../emulator/EmulatorProvider", async (orig) => {
  const actual = await orig<typeof import("../emulator/EmulatorProvider")>();
  return { ...actual, useEmulator: () => mockCtx };
});

const addToast = vi.fn();
vi.mock("./toast/ToastProvider", () => ({
  useToast: () => ({ toasts: [], addToast }),
}));

let mockCtx: EmulatorContextValue;

function makeCtx(pc: number, sp: number): EmulatorContextValue {
  return {
    status: "ready",
    snapshot: {
      registers: { a: 0, x: 0, y: 0, sp, pc, status: 0x24 },
      flags: { n: false, v: false, u: true, b: false, d: false, i: true, z: false, c: false },
      stats: { instructionCount: 0, cycleCount: 0 },
      running: false,
    },
    breakpoints: [],
    running: false,
    framebuffer: null,
    dbg: null,
    actions: {} as EmulatorContextValue["actions"],
  };
}

beforeEach(() => {
  addToast.mockClear();
  // one 16-byte row at $0C00 so PC $0C03 falls inside
  mockRows = [
    {
      address: 0x0c00,
      bytes: [0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
      ascii: "................",
    },
  ];
});

describe("MemoryPanel", () => {
  it("renders a page select built from MEMORY_PAGES and 0..F column headers", () => {
    mockCtx = makeCtx(0x0c03, 0xfd);
    render(<MemoryPanel />);
    const select = screen.getByTestId("memory-page-select") as HTMLSelectElement;
    expect(select.options).toHaveLength(4);
    expect(select.options[0].textContent).toContain("Zero Page");
    for (const col of ["0", "F"]) {
      expect(screen.getByTestId(`memory-col-header-${col}`)).toBeInTheDocument();
    }
  });

  it("raises an info toast when the page changes", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx(0x0c03, 0xfd);
    render(<MemoryPanel />);
    await user.selectOptions(screen.getByTestId("memory-page-select"), "stack");
    expect(addToast).toHaveBeenCalledWith(
      expect.stringContaining("Memory view changed to"),
      "info",
    );
  });

  it("raises a danger toast on an invalid jump address", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx(0x0c03, 0xfd);
    render(<MemoryPanel />);
    await user.type(screen.getByTestId("memory-jump-input"), "GGGG");
    await user.click(screen.getByTestId("memory-jump-button"));
    expect(addToast).toHaveBeenCalledWith("Invalid memory address", "danger");
  });

  it("raises an info toast when a valid jump navigates the view", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx(0x0c03, 0xfd);
    render(<MemoryPanel />);
    await user.type(screen.getByTestId("memory-jump-input"), "0240");
    await user.click(screen.getByTestId("memory-jump-button"));
    expect(addToast).toHaveBeenCalledWith(
      "Jumped to memory address $0200",
      "info",
    );
  });

  it("jumps to an arbitrary 256-byte page not in the preset list", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx(0x0c03, 0xfd);
    render(<MemoryPanel />);
    // $0C00 page is not one of the four MEMORY_PAGES presets.
    await user.type(screen.getByTestId("memory-jump-input"), "0C34");
    await user.click(screen.getByTestId("memory-jump-button"));
    expect(addToast).toHaveBeenCalledWith(
      "Jumped to memory address $0C00",
      "info",
    );
  });

  it("does not show a jump toast when the jump target is the current page", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx(0x0c03, 0xfd);
    render(<MemoryPanel />);
    // Starting page is Zero Page ($0000). Jump within the same page.
    await user.type(screen.getByTestId("memory-jump-input"), "0042");
    await user.click(screen.getByTestId("memory-jump-button"));
    expect(addToast).not.toHaveBeenCalledWith(
      expect.stringContaining("Jumped to"),
      "info",
    );
  });

  it("highlights the PC cell red", () => {
    mockCtx = makeCtx(0x0c03, 0xfd);
    render(<MemoryPanel />);
    const cell = screen.getByTestId("memory-cell-0x0c03");
    expect(cell.getAttribute("data-pc")).toBe("true");
  });

  it("opens the edit modal for the clicked cell address", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx(0x0c03, 0xfd);
    render(<MemoryPanel />);
    await user.click(screen.getByTestId("memory-cell-0x0c05"));
    expect(screen.getByTestId("memory-edit-modal")).toHaveAttribute("data-address", "0x0c05");
  });

  it("opens the edit modal via the keyboard (Enter) on a focusable cell", () => {
    mockCtx = makeCtx(0x0c03, 0xfd);
    render(<MemoryPanel />);
    const cell = screen.getByTestId("memory-cell-0x0c05");
    expect(cell).toHaveAttribute("role", "button");
    expect(cell).toHaveAttribute("tabindex", "0");
    fireEvent.keyDown(cell, { key: "Enter" });
    expect(screen.getByTestId("memory-edit-modal")).toHaveAttribute(
      "data-address",
      "0x0c05",
    );
  });

  it("makes interactive cells non-focusable while running", () => {
    mockCtx = makeCtx(0x0c03, 0xfd);
    mockCtx.running = true;
    render(<MemoryPanel />);
    expect(screen.getByTestId("memory-cell-0x0c05")).toHaveAttribute(
      "tabindex",
      "-1",
    );
  });
});
