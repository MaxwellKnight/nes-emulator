import { useEffect, useRef, useState } from "react";
import type { Debugger } from "../wasm/bridge";
import { Module } from "./ui/Module";

export interface PpuViewerProps {
  dbg: Debugger | null;
  revealDelay?: number;
  className?: string;
}

// Standard NES 2C02 master palette as #rrggbb (mirrors include/palette.h),
// used to color the palette swatches by their stored index.
const MASTER_PALETTE: string[] = [
  "#626262", "#001fb2", "#2404c8", "#5200b2", "#730076", "#800024", "#730b00", "#522800",
  "#244400", "#005700", "#005c00", "#005324", "#003c76", "#000000", "#000000", "#000000",
  "#ab0000", "#0d57ff", "#4b30ff", "#8c00ff", "#bf00b2", "#d6005e", "#d62800", "#bf4b00",
  "#8c7300", "#349700", "#00ab00", "#00a344", "#008cb2", "#000000", "#000000", "#000000",
  "#ffffff", "#53aeff", "#9085ff", "#d365ff", "#ff57ff", "#ff5dcf", "#ff7757", "#fa9e00",
  "#bdc700", "#7ae700", "#43f611", "#26ef7e", "#2cd5f6", "#4e4e4e", "#000000", "#000000",
  "#ffffff", "#b6e1ff", "#ced1ff", "#e9c3ff", "#ffbcff", "#ffbdf4", "#ffc6c3", "#ffd59a",
  "#e9e681", "#cef481", "#b6fb9a", "#a9fac3", "#a9f0f6", "#b8b8b8", "#000000", "#000000",
];

const PT_SIZE = 128;

function PatternCanvas({
  dbg,
  table,
  palette,
}: {
  dbg: Debugger | null;
  table: number;
  palette: number;
}): JSX.Element {
  const ref = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    if (!dbg) return;
    const canvas = ref.current;
    if (!canvas) return;
    const ctx = canvas.getContext("2d");
    if (!ctx) return;
    ctx.imageSmoothingEnabled = false;
    const rgba = dbg.renderPatternTable(table, palette);
    ctx.putImageData(new ImageData(rgba as unknown as ImageDataArray, PT_SIZE, PT_SIZE), 0, 0);
  }, [dbg, table, palette]);

  return (
    <canvas
      ref={ref}
      data-testid={`pattern-table-${table}`}
      width={PT_SIZE}
      height={PT_SIZE}
      style={{ imageRendering: "pixelated" }}
      className="h-auto w-full rounded-[var(--radius-sm)] bg-black"
    />
  );
}

function hex2(n: number): string {
  return n.toString(16).toUpperCase().padStart(2, "0");
}

export function PpuViewer({
  dbg,
  revealDelay,
  className,
}: PpuViewerProps): JSX.Element {
  const [palette, setPalette] = useState(0);

  const ppu = dbg
    ? dbg.ppuState()
    : { ctrl: 0, mask: 0, status: 0, scanline: 0 };
  const paletteRam = dbg ? dbg.getPaletteRam() : new Uint8Array(32);

  return (
    <Module
      title="PPU"
      status={<span>pattern · palette</span>}
      revealDelay={revealDelay}
      className={className}
    >
      <div data-testid="ppu-viewer" className="flex flex-col gap-[10px]">
        <div className="flex items-center gap-2">
          <label
            htmlFor="ppu-palette-select"
            className="font-sans text-[10px] uppercase tracking-[0.1em] text-[var(--mut)]"
          >
            Palette
          </label>
          <select
            id="ppu-palette-select"
            data-testid="ppu-palette-select"
            value={palette}
            onChange={(e) => setPalette(Number(e.target.value))}
            className="rounded-[var(--radius-sm)] border border-[var(--bd)] bg-[var(--b2)] px-2 py-1 font-mono text-[10px] text-[var(--tx)]"
          >
            {Array.from({ length: 8 }, (_, i) => (
              <option key={i} value={i}>
                {i}
              </option>
            ))}
          </select>
        </div>

        <div className="grid grid-cols-2 gap-[8px]">
          <PatternCanvas dbg={dbg} table={0} palette={palette} />
          <PatternCanvas dbg={dbg} table={1} palette={palette} />
        </div>

        <div className="grid grid-cols-16 gap-[2px]" data-testid="palette-strip">
          {Array.from({ length: 32 }, (_, i) => {
            const entry = paletteRam[i] & 0x3f;
            return (
              <span
                key={i}
                data-testid={`palette-swatch-${i}`}
                title={`$3F${hex2(i)} → $${hex2(entry)}`}
                className="aspect-square w-full rounded-[2px] border border-black/30"
                style={{ backgroundColor: MASTER_PALETTE[entry] }}
              />
            );
          })}
        </div>

        <div className="grid grid-cols-4 gap-2 font-mono text-[10px] text-[var(--tx-mut)]">
          <span>
            CTRL <b data-testid="ppu-ctrl">${hex2(ppu.ctrl)}</b>
          </span>
          <span>
            MASK <b data-testid="ppu-mask">${hex2(ppu.mask)}</b>
          </span>
          <span>
            STAT <b data-testid="ppu-status">${hex2(ppu.status)}</b>
          </span>
          <span>
            SL <b data-testid="ppu-scanline">{ppu.scanline}</b>
          </span>
        </div>
      </div>
    </Module>
  );
}
