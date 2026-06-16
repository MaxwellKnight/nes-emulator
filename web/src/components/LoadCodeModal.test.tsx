// web/src/components/LoadCodeModal.test.tsx
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { describe, it, expect, vi, beforeEach } from "vitest";
import { LoadCodeModal } from "./LoadCodeModal";
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
const onClose = vi.fn();

function makeCtx(overrides: Partial<EmulatorContextValue> = {}): EmulatorContextValue {
  return {
    status: "ready",
    snapshot: null,
    breakpoints: [],
    running: false,
    framebuffer: null,
    movie: { playing: false, frame: 0, total: 0 },
    dbg: null,
    actions: { loadOpcodes } as unknown as EmulatorContextValue["actions"],
    ...overrides,
  };
}

beforeEach(() => {
  addToast.mockClear();
  loadOpcodes.mockReset();
  onClose.mockClear();
});

describe("LoadCodeModal", () => {
  it("renders nothing when closed", () => {
    mockCtx = makeCtx();
    render(<LoadCodeModal open={false} onClose={onClose} />);
    expect(screen.queryByTestId("loadcode-modal")).toBeNull();
  });

  it("warns when the textarea is empty", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx();
    render(<LoadCodeModal open onClose={onClose} />);
    await user.click(screen.getByTestId("loadcode-button"));
    expect(loadOpcodes).not.toHaveBeenCalled();
    expect(addToast).toHaveBeenCalledWith("Please enter opcodes", "warning");
  });

  it("loads opcodes, reports the byte count and closes on success", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx();
    render(<LoadCodeModal open onClose={onClose} />);
    await user.type(screen.getByTestId("loadcode-textarea"), "A9 01 00");
    await user.click(screen.getByTestId("loadcode-button"));
    expect(loadOpcodes).toHaveBeenCalledWith("A9 01 00");
    expect(addToast).toHaveBeenCalledWith("Loaded 3 bytes successfully", "success");
    expect(onClose).toHaveBeenCalled();
  });

  it("warns when the parse yields zero bytes", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx();
    render(<LoadCodeModal open onClose={onClose} />);
    await user.type(screen.getByTestId("loadcode-textarea"), "; just a comment");
    await user.click(screen.getByTestId("loadcode-button"));
    expect(loadOpcodes).not.toHaveBeenCalled();
    expect(addToast).toHaveBeenCalledWith("No valid opcodes found", "warning");
  });

  it("raises a danger toast when parsing throws", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx();
    render(<LoadCodeModal open onClose={onClose} />);
    await user.type(screen.getByTestId("loadcode-textarea"), "ZZ");
    await user.click(screen.getByTestId("loadcode-button"));
    expect(addToast).toHaveBeenCalledWith("Invalid opcode format: ZZ", "danger");
  });

  it("disables the textarea and load button while execution is running", () => {
    mockCtx = makeCtx({ running: true });
    render(<LoadCodeModal open onClose={onClose} />);
    expect(screen.getByTestId("loadcode-textarea")).toBeDisabled();
    expect(screen.getByTestId("loadcode-button")).toBeDisabled();
  });

  it("copies the example program to the clipboard", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx();
    render(<LoadCodeModal open onClose={onClose} />);
    const writeText = vi
      .spyOn(navigator.clipboard, "writeText")
      .mockResolvedValue(undefined);
    await user.click(screen.getByTestId("loadcode-copy"));
    expect(writeText).toHaveBeenCalled();
    expect(writeText.mock.calls[0][0].length).toBeGreaterThan(0);
    writeText.mockRestore();
  });
});
