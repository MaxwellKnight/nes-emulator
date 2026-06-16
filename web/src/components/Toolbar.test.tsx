import { describe, it, expect, vi, beforeEach } from "vitest";
import { render, screen, fireEvent } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import type { EmulatorContextValue } from "../emulator/EmulatorProvider";
import { Toolbar } from "./Toolbar";

const mockContext = vi.hoisted(() => ({ value: null as EmulatorContextValue | null }));

vi.mock("../emulator/EmulatorProvider", () => ({
  useEmulator: () => mockContext.value,
}));

const addToast = vi.fn();
vi.mock("./toast/ToastProvider", () => ({
  useToast: () => ({ toasts: [], addToast }),
}));

function makeContext(
  overrides: Partial<EmulatorContextValue> = {},
): EmulatorContextValue {
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
    connectLiveAgent: vi.fn(),
    disconnectLiveAgent: vi.fn(),
  };
  return {
    status: "ready",
    snapshot: {
      registers: { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0x0c00, status: 0 },
      flags: { n: false, v: false, u: true, b: false, d: false, i: false, z: false, c: false },
      stats: { instructionCount: 42, cycleCount: 100 },
      running: false,
    },
    breakpoints: [],
    running: false,
    framebuffer: null,
    movie: { playing: false, frame: 0, total: 0 },
    liveAgent: { connected: false, frame: 0 },
    dbg: null,
    actions,
    ...overrides,
  };
}

describe("Toolbar", () => {
  beforeEach(() => {
    localStorage.clear();
    delete document.documentElement.dataset.theme;
    addToast.mockClear();
    mockContext.value = makeContext();
  });

  it("shows the app title", () => {
    render(<Toolbar onHelp={vi.fn()} />);
    expect(screen.getByText(/6502/i)).toBeInTheDocument();
  });

  it("shows the ROM name when provided", () => {
    render(<Toolbar romName="demo.nes" onHelp={vi.fn()} />);
    expect(screen.getByText("demo.nes")).toBeInTheDocument();
  });

  it("connects to a live agent when Spawn Agent is clicked", () => {
    const ctx = makeContext();
    mockContext.value = ctx;
    render(<Toolbar onHelp={vi.fn()} />);
    fireEvent.click(screen.getByTestId("spawn-agent"));
    expect(ctx.actions.connectLiveAgent).toHaveBeenCalled();
  });

  it("disconnects when an agent is already live", () => {
    const ctx = makeContext({ liveAgent: { connected: true, frame: 99 } });
    mockContext.value = ctx;
    render(<Toolbar onHelp={vi.fn()} />);
    fireEvent.click(screen.getByTestId("spawn-agent"));
    expect(ctx.actions.disconnectLiveAgent).toHaveBeenCalled();
  });

  it("shows live instruction and cycle stats from the snapshot", () => {
    render(<Toolbar onHelp={vi.fn()} />);
    expect(screen.getByText(/42/)).toBeInTheDocument();
    expect(screen.getByText(/100/)).toBeInTheDocument();
  });

  it("wires Run, Step, Stop, Reset to emulator actions", async () => {
    const ctx = makeContext();
    mockContext.value = ctx;
    render(<Toolbar onHelp={vi.fn()} />);
    await userEvent.click(screen.getByRole("button", { name: /Run/ }));
    await userEvent.click(screen.getByRole("button", { name: /Step/ }));
    await userEvent.click(screen.getByRole("button", { name: /Reset/ }));
    expect(ctx.actions.run).toHaveBeenCalledTimes(1);
    expect(ctx.actions.step).toHaveBeenCalledTimes(1);
    expect(ctx.actions.reset).toHaveBeenCalledTimes(1);
  });

  it("disables Run and Step while running and enables Stop", () => {
    mockContext.value = makeContext({ running: true });
    render(<Toolbar onHelp={vi.fn()} />);
    expect(screen.getByRole("button", { name: /Run/ })).toBeDisabled();
    expect(screen.getByRole("button", { name: /Step/ })).toBeDisabled();
    expect(screen.getByRole("button", { name: /Stop/ })).toBeEnabled();
  });

  it("disables Stop when not running", () => {
    mockContext.value = makeContext({ running: false });
    render(<Toolbar onHelp={vi.fn()} />);
    expect(screen.getByRole("button", { name: /Stop/ })).toBeDisabled();
  });

  it("pulses the running dot while executing", () => {
    mockContext.value = makeContext({ running: true });
    render(<Toolbar onHelp={vi.fn()} />);
    expect(screen.getByTestId("running-dot").getAttribute("data-running")).toBe(
      "true",
    );
  });

  it("gives transport, Load ROM, and Load Code buttons accessible names and tooltips", () => {
    render(<Toolbar onHelp={vi.fn()} onLoadCode={vi.fn()} />);
    for (const name of ["Run", "Step", "Stop", "Reset", "Load ROM", "Load Code"]) {
      const btn = screen.getByRole("button", { name });
      expect(btn).toBeInTheDocument();
      expect(btn).toHaveAttribute("title");
    }
  });

  it("opens the Load Code popover via onLoadCode", async () => {
    const onLoadCode = vi.fn();
    render(<Toolbar onHelp={vi.fn()} onLoadCode={onLoadCode} />);
    await userEvent.click(screen.getByTestId("loadcode-open"));
    expect(onLoadCode).toHaveBeenCalledTimes(1);
  });

  it("calls onHelp when the Help button is clicked", async () => {
    const onHelp = vi.fn();
    render(<Toolbar onHelp={onHelp} />);
    await userEvent.click(screen.getByRole("button", { name: "Help" }));
    expect(onHelp).toHaveBeenCalledTimes(1);
  });

  it("reads a ROM file and calls actions.loadRom with the bytes", async () => {
    const ctx = makeContext();
    mockContext.value = ctx;
    render(<Toolbar onHelp={vi.fn()} />);
    const input = screen.getByTestId("rom-file-input") as HTMLInputElement;
    const file = new File([new Uint8Array([0xa9, 0x01, 0x00])], "prog.nes");
    await userEvent.upload(input, file);
    await vi.waitFor(() => {
      expect(ctx.actions.loadRom).toHaveBeenCalledTimes(1);
    });
    const arg = vi.mocked(ctx.actions.loadRom).mock.calls[0][0] as Uint8Array;
    expect(Array.from(arg)).toEqual([0xa9, 0x01, 0x00]);
  });

  it("renders the theme toggle", () => {
    render(<Toolbar onHelp={vi.fn()} />);
    expect(
      screen.getByRole("button", { name: /toggle theme/i }),
    ).toBeInTheDocument();
  });

  it("renders contact links and the developer credit", () => {
    render(<Toolbar onHelp={vi.fn()} />);
    const github = screen.getByRole("link", { name: /github/i });
    const linkedin = screen.getByRole("link", { name: /linkedin/i });
    const email = screen.getByRole("link", { name: /email/i });
    expect(github).toHaveAttribute(
      "href",
      "https://github.com/MaxwellKnight/nes-emulator",
    );
    expect(linkedin).toHaveAttribute(
      "href",
      "https://www.linkedin.com/in/maxwell-knight/",
    );
    expect(email).toHaveAttribute("href", "mailto:maxwell.knight98@gmail.com");
    expect(screen.getByText(/by Maxwell Knight/i)).toBeInTheDocument();
  });

  it("shows the loaded ROM name and a success toast on file load", async () => {
    const ctx = makeContext();
    vi.mocked(ctx.actions.loadRom).mockReturnValue(0);
    mockContext.value = ctx;
    render(<Toolbar onHelp={vi.fn()} />);
    const input = screen.getByTestId("rom-file-input") as HTMLInputElement;
    const file = new File([new Uint8Array([0xa9, 0x01, 0x00])], "prog.nes");
    await userEvent.upload(input, file);
    await vi.waitFor(() => {
      expect(ctx.actions.loadRom).toHaveBeenCalledTimes(1);
    });
    await vi.waitFor(() => {
      expect(screen.getByText("prog.nes")).toBeInTheDocument();
    });
    expect(addToast).toHaveBeenCalledWith("ROM loaded: prog.nes", "success");
  });

  it("calls actions.loadRom with the .nes bytes and toasts success on status 0", async () => {
    const ctx = makeContext();
    vi.mocked(ctx.actions.loadRom).mockReturnValue(0);
    mockContext.value = ctx;
    render(<Toolbar onHelp={vi.fn()} />);
    const input = screen.getByTestId("rom-file-input") as HTMLInputElement;
    const file = new File([new Uint8Array([0x4e, 0x45, 0x53, 0x1a])], "game.nes");
    await userEvent.upload(input, file);
    await vi.waitFor(() => {
      expect(ctx.actions.loadRom).toHaveBeenCalledTimes(1);
    });
    const arg = vi.mocked(ctx.actions.loadRom).mock.calls[0][0] as Uint8Array;
    expect(Array.from(arg)).toEqual([0x4e, 0x45, 0x53, 0x1a]);
    await vi.waitFor(() => {
      expect(screen.getByText("game.nes")).toBeInTheDocument();
    });
    expect(addToast).toHaveBeenCalledWith("ROM loaded: game.nes", "success");
  });

  it("toasts 'Unsupported mapper' on status 2", async () => {
    const ctx = makeContext();
    vi.mocked(ctx.actions.loadRom).mockReturnValue(2);
    mockContext.value = ctx;
    render(<Toolbar onHelp={vi.fn()} />);
    const input = screen.getByTestId("rom-file-input") as HTMLInputElement;
    const file = new File([new Uint8Array([0x00])], "mmc.nes");
    await userEvent.upload(input, file);
    await vi.waitFor(() => {
      expect(addToast).toHaveBeenCalledWith("Unsupported mapper", "danger");
    });
  });

  it("toasts 'Invalid ROM file' on any other nonzero status", async () => {
    const ctx = makeContext();
    vi.mocked(ctx.actions.loadRom).mockReturnValue(1);
    mockContext.value = ctx;
    render(<Toolbar onHelp={vi.fn()} />);
    const input = screen.getByTestId("rom-file-input") as HTMLInputElement;
    const file = new File([new Uint8Array([0x00])], "broken.nes");
    await userEvent.upload(input, file);
    await vi.waitFor(() => {
      expect(addToast).toHaveBeenCalledWith("Invalid ROM file", "danger");
    });
  });
});
