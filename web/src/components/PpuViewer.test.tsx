import { describe, it, expect, vi, beforeEach } from "vitest";
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import type { Debugger } from "../wasm/bridge";
import { PpuViewer } from "./PpuViewer";

const putImageData = vi.fn();
const getContextMock = vi.fn(() => ({
  putImageData,
  imageSmoothingEnabled: true,
}));

function makeDbg(): Debugger {
  return {
    step() {},
    run() {},
    stop() {},
    reset() {},
    isRunning() {
      return false;
    },
    addBreakpoint() {},
    removeBreakpoint() {},
    clearBreakpoints() {},
    getRegisters() {
      return { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0, status: 0 };
    },
    getFlags() {
      return {
        n: false,
        v: false,
        u: true,
        b: false,
        d: false,
        i: false,
        z: false,
        c: false,
      };
    },
    getStats() {
      return { instructionCount: 0, cycleCount: 0 };
    },
    getSnapshot() {
      return {
        registers: { a: 0, x: 0, y: 0, sp: 0xfd, pc: 0, status: 0 },
        flags: {
          n: false,
          v: false,
          u: true,
          b: false,
          d: false,
          i: false,
          z: false,
          c: false,
        },
        stats: { instructionCount: 0, cycleCount: 0 },
        running: false,
      };
    },
    readMemory() {
      return 0;
    },
    writeMemory() {},
    readMemoryRange() {
      return [];
    },
    setPC() {},
    disassembleAroundPC() {
      return [];
    },
    disassembleRange() {
      return [];
    },
    loadROM() {},
    loadRom() {
      return 0;
    },
    getFramebuffer() {
      return new Uint8ClampedArray(256 * 240 * 4);
    },
    runFrame() {
      return 0;
    },
    frameCount() {
      return 0;
    },
    renderPatternTable: vi.fn((_table: number, _palette: number) => {
      const buf = new Uint8ClampedArray(128 * 128 * 4);
      buf[0] = 0x12;
      return buf;
    }),
    getNametable() {
      const nt = new Uint8Array(2048);
      nt[0] = 0xab;
      return nt;
    },
    getPaletteRam() {
      const pal = new Uint8Array(32);
      for (let i = 0; i < 32; i += 1) pal[i] = i;
      return pal;
    },
    ppuState() {
      return { ctrl: 0x90, mask: 0x1e, status: 0x80, scanline: 30 };
    },
  } as unknown as Debugger;
}

describe("PpuViewer", () => {
  beforeEach(() => {
    putImageData.mockClear();
    getContextMock.mockClear();
    HTMLCanvasElement.prototype.getContext = getContextMock as unknown as HTMLCanvasElement["getContext"];
    if (typeof globalThis.ImageData === "undefined") {
      // @ts-expect-error test shim
      globalThis.ImageData = class {
        data: Uint8ClampedArray;
        width: number;
        height: number;
        constructor(data: Uint8ClampedArray, width: number, height: number) {
          this.data = data;
          this.width = width;
          this.height = height;
        }
      };
    }
  });

  it("renders two 128x128 pattern-table canvases", () => {
    render(<PpuViewer dbg={makeDbg()} />);
    const left = screen.getByTestId("pattern-table-0") as HTMLCanvasElement;
    const right = screen.getByTestId("pattern-table-1") as HTMLCanvasElement;
    expect(left.width).toBe(128);
    expect(left.height).toBe(128);
    expect(right.width).toBe(128);
    expect(right.height).toBe(128);
  });

  it("calls renderPatternTable for both tables with the selected palette", () => {
    const dbg = makeDbg();
    render(<PpuViewer dbg={dbg} />);
    expect(dbg.renderPatternTable).toHaveBeenCalledWith(0, 0);
    expect(dbg.renderPatternTable).toHaveBeenCalledWith(1, 0);
  });

  it("re-renders pattern tables when the palette selector changes", async () => {
    const dbg = makeDbg();
    render(<PpuViewer dbg={dbg} />);
    vi.mocked(dbg.renderPatternTable).mockClear();
    const select = screen.getByTestId("ppu-palette-select") as HTMLSelectElement;
    await userEvent.selectOptions(select, "3");
    expect(dbg.renderPatternTable).toHaveBeenCalledWith(0, 3);
    expect(dbg.renderPatternTable).toHaveBeenCalledWith(1, 3);
  });

  it("renders 32 palette swatches from getPaletteRam", () => {
    render(<PpuViewer dbg={makeDbg()} />);
    const swatches = screen.getAllByTestId(/palette-swatch-/);
    expect(swatches).toHaveLength(32);
  });

  it("shows the PPU register readout from ppuState", () => {
    render(<PpuViewer dbg={makeDbg()} />);
    expect(screen.getByTestId("ppu-ctrl").textContent).toContain("90");
    expect(screen.getByTestId("ppu-mask").textContent).toContain("1E");
    expect(screen.getByTestId("ppu-status").textContent).toContain("80");
    expect(screen.getByTestId("ppu-scanline").textContent).toContain("30");
  });

  it("renders nothing useful but does not crash when dbg is null", () => {
    render(<PpuViewer dbg={null} />);
    expect(screen.getByTestId("ppu-viewer")).toBeInTheDocument();
  });
});
