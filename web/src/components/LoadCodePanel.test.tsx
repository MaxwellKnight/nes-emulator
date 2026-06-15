// web/src/components/LoadCodePanel.test.tsx
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { describe, it, expect, vi, beforeEach } from "vitest";
import { LoadCodePanel } from "./LoadCodePanel";
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
const loadOpcodes = vi.fn();

function makeCtx(): EmulatorContextValue {
  return {
    status: "ready",
    snapshot: null,
    breakpoints: [],
    running: false,
    dbg: null,
    actions: { loadOpcodes } as unknown as EmulatorContextValue["actions"],
  };
}

beforeEach(() => {
  addToast.mockClear();
  loadOpcodes.mockReset();
});

describe("LoadCodePanel", () => {
  it("warns when the textarea is empty", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx();
    render(<LoadCodePanel />);
    await user.click(screen.getByTestId("loadcode-button"));
    expect(loadOpcodes).not.toHaveBeenCalled();
    expect(addToast).toHaveBeenCalledWith("Please enter opcodes", "warning");
  });

  it("loads opcodes and reports success", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx();
    render(<LoadCodePanel />);
    await user.type(screen.getByTestId("loadcode-textarea"), "A9 01 00");
    await user.click(screen.getByTestId("loadcode-button"));
    expect(loadOpcodes).toHaveBeenCalledWith("A9 01 00");
    expect(addToast).toHaveBeenCalledWith("Opcodes loaded successfully", "success");
  });

  it("raises a danger toast when parsing throws", async () => {
    const user = userEvent.setup();
    loadOpcodes.mockImplementation(() => {
      throw new Error("Invalid opcode format: ZZ");
    });
    mockCtx = makeCtx();
    render(<LoadCodePanel />);
    await user.type(screen.getByTestId("loadcode-textarea"), "ZZ");
    await user.click(screen.getByTestId("loadcode-button"));
    expect(addToast).toHaveBeenCalledWith("Invalid opcode format: ZZ", "danger");
  });

  it("copies the example program to the clipboard", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx();
    render(<LoadCodePanel />);
    const writeText = vi.spyOn(navigator.clipboard, "writeText").mockResolvedValue(undefined);
    await user.click(screen.getByTestId("loadcode-copy"));
    expect(writeText).toHaveBeenCalled();
    expect(writeText.mock.calls[0][0].length).toBeGreaterThan(0);
    writeText.mockRestore();
  });
});
