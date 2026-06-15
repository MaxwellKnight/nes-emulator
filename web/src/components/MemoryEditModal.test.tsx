// web/src/components/MemoryEditModal.test.tsx
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { describe, it, expect, vi, beforeEach } from "vitest";
import { MemoryEditModal } from "./MemoryEditModal";
import type { EmulatorContextValue } from "../emulator/EmulatorProvider";
import type { Debugger } from "../wasm/bridge";

vi.mock("../emulator/EmulatorProvider", async (orig) => {
  const actual = await orig<typeof import("../emulator/EmulatorProvider")>();
  return { ...actual, useEmulator: () => mockCtx };
});

const addToast = vi.fn();
vi.mock("./toast/ToastProvider", () => ({
  useToast: () => ({ toasts: [], addToast }),
}));

let mockCtx: EmulatorContextValue;
const writeMemory = vi.fn();
const onClose = vi.fn();

function makeCtx(currentValue: number): EmulatorContextValue {
  return {
    status: "ready",
    snapshot: null,
    breakpoints: [],
    running: false,
    framebuffer: null,
    dbg: { readMemory: () => currentValue } as unknown as Debugger,
    actions: { writeMemory } as unknown as EmulatorContextValue["actions"],
  };
}

beforeEach(() => {
  addToast.mockClear();
  writeMemory.mockClear();
  onClose.mockClear();
});

describe("MemoryEditModal", () => {
  it("shows the current value for the address on open", () => {
    mockCtx = makeCtx(0x2a);
    render(<MemoryEditModal address={0x0240} open onClose={onClose} />);
    expect((screen.getByTestId("memedit-hex") as HTMLInputElement).value).toBe("2A");
    expect((screen.getByTestId("memedit-dec") as HTMLInputElement).value).toBe("42");
    expect((screen.getByTestId("memedit-bin") as HTMLInputElement).value).toBe("00101010");
  });

  it("keeps hex and decimal bidirectional and binary derived read-only", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx(0x00);
    render(<MemoryEditModal address={0x0240} open onClose={onClose} />);
    const dec = screen.getByTestId("memedit-dec");
    await user.clear(dec);
    await user.type(dec, "255");
    expect((screen.getByTestId("memedit-hex") as HTMLInputElement).value).toBe("FF");
    expect((screen.getByTestId("memedit-bin") as HTMLInputElement).value).toBe("11111111");
    expect(screen.getByTestId("memedit-bin")).toHaveAttribute("readonly");
  });

  it("saves a valid byte, raises a success toast and closes", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx(0x00);
    render(<MemoryEditModal address={0x0240} open onClose={onClose} />);
    const hex = screen.getByTestId("memedit-hex");
    await user.clear(hex);
    await user.type(hex, "7F");
    await user.click(screen.getByTestId("memedit-save"));
    expect(writeMemory).toHaveBeenCalledWith(0x0240, 0x7f);
    expect(addToast).toHaveBeenCalledWith("Memory at $0240 set to $7F", "success");
    expect(onClose).toHaveBeenCalled();
  });

  it("rejects an out-of-range value with a danger toast and does not write", async () => {
    const user = userEvent.setup();
    mockCtx = makeCtx(0x00);
    render(<MemoryEditModal address={0x0240} open onClose={onClose} />);
    const dec = screen.getByTestId("memedit-dec");
    await user.clear(dec);
    await user.type(dec, "300");
    await user.click(screen.getByTestId("memedit-save"));
    expect(writeMemory).not.toHaveBeenCalled();
    expect(addToast).toHaveBeenCalledWith("Invalid byte value (0-255)", "danger");
  });
});
