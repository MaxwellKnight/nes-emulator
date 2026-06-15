import { describe, it, expect } from "vitest";
import { render, screen } from "@testing-library/react";
import type { Debugger } from "../wasm/bridge";
import { OamViewer } from "./OamViewer";

function makeDbg(oam: Uint8Array): Debugger {
  // OamViewer only reads getOam(); cast a partial double.
  return { getOam: () => oam } as unknown as Debugger;
}

describe("OamViewer", () => {
  it("lists 64 sprites and counts the on-screen ones", () => {
    const oam = new Uint8Array(256);
    // Two on-screen sprites (Y < 0xEF), the rest parked at $F8.
    for (let i = 0; i < 64; i++) oam[i * 4] = 0xf8;
    oam[0] = 0x20; // sprite 0 on screen
    oam[4] = 0x40; // sprite 1 on screen
    render(<OamViewer dbg={makeDbg(oam)} />);

    expect(screen.getByTestId("oam-viewer")).toBeInTheDocument();
    expect(screen.getByText("2/64 on screen")).toBeInTheDocument();
    // All 64 rows render.
    expect(screen.getByTestId("oam-row-0")).toBeInTheDocument();
    expect(screen.getByTestId("oam-row-63")).toBeInTheDocument();
  });

  it("decodes sprite attributes (palette, priority, flips)", () => {
    const oam = new Uint8Array(256).fill(0xf8);
    // Sprite 0: Y=$30, tile=$AB, attr=$E3 (flipV+flipH, behind, palette 3), X=$50
    oam[0] = 0x30;
    oam[1] = 0xab;
    oam[2] = 0xe3;
    oam[3] = 0x50;
    render(<OamViewer dbg={makeDbg(oam)} />);

    const row = screen.getByTestId("oam-row-0");
    expect(row.textContent).toContain("$AB"); // tile
    expect(row.textContent).toContain("$50"); // X
    expect(row.textContent).toContain("7"); // palette 4 + (attr&3=3) = 7
    expect(row.textContent).toContain("bg"); // behind background
    expect(row.textContent).toContain("HV"); // both flips
  });

  it("renders empty gracefully without a debugger", () => {
    render(<OamViewer dbg={null} />);
    expect(screen.getByText("0/64 on screen")).toBeInTheDocument();
  });
});
