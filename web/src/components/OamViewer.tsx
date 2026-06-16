import type { Debugger } from "../wasm/bridge";
import { Module } from "./ui/Module";

export interface OamViewerProps {
  dbg: Debugger | null;
  revealDelay?: number;
  className?: string;
}

function hex2(n: number): string {
  return n.toString(16).toUpperCase().padStart(2, "0");
}

interface Sprite {
  index: number;
  y: number;
  tile: number;
  attr: number;
  x: number;
  onScreen: boolean;
}

function readSprites(dbg: Debugger | null): Sprite[] {
  if (!dbg) return [];
  let oam: Uint8Array;
  try {
    oam = dbg.getOam();
  } catch {
    return [];
  }
  const sprites: Sprite[] = [];
  for (let i = 0; i < 64; i++) {
    const y = oam[i * 4];
    sprites.push({
      index: i,
      y,
      tile: oam[i * 4 + 1],
      attr: oam[i * 4 + 2],
      x: oam[i * 4 + 3],
      // Y in $EF-$FF parks a sprite off the bottom of the visible area.
      onScreen: y < 0xef,
    });
  }
  return sprites;
}

/**
 * Object Attribute Memory inspector: the 64 hardware sprites with their tile,
 * position, palette, priority and flip flags. Sprite 0 (which drives the
 * sprite-0-hit raster split) is highlighted.
 */
export function OamViewer({
  dbg,
  revealDelay,
  className,
}: OamViewerProps): JSX.Element {
  const sprites = readSprites(dbg);
  const visible = sprites.filter((s) => s.onScreen);

  return (
    <Module
      title="OAM · Sprites"
      status={<span>{visible.length}/64 on screen</span>}
      revealDelay={revealDelay}
      className={className}
    >
      <div
        data-testid="oam-viewer"
        className="max-h-[176px] overflow-y-auto rounded-[var(--radius-sm)] border border-[var(--bd)]"
      >
        <table className="w-full border-collapse font-mono text-[10px]">
          <thead className="sticky top-0 bg-[var(--b2)] text-[var(--mut)]">
            <tr className="text-left">
              <th className="px-2 py-1 font-medium">#</th>
              <th className="px-2 py-1 font-medium">X</th>
              <th className="px-2 py-1 font-medium">Y</th>
              <th className="px-2 py-1 font-medium">Tile</th>
              <th className="px-2 py-1 font-medium">Pal</th>
              <th className="px-2 py-1 font-medium">Pri</th>
              <th className="px-2 py-1 font-medium">Flip</th>
            </tr>
          </thead>
          <tbody>
            {sprites.map((s) => {
              const pal = s.attr & 0x03;
              const behind = (s.attr & 0x20) !== 0;
              const flipH = (s.attr & 0x40) !== 0;
              const flipV = (s.attr & 0x80) !== 0;
              const flip =
                (flipH ? "H" : "") + (flipV ? "V" : "") || "–";
              return (
                <tr
                  key={s.index}
                  data-testid={`oam-row-${s.index}`}
                  className={[
                    "border-t border-[var(--bd)]",
                    s.index === 0 ? "bg-[var(--accent)]/15 text-[var(--tx)]" : "",
                    s.onScreen ? "text-[var(--tx)]" : "text-[var(--mut)] opacity-60",
                  ].join(" ")}
                >
                  <td className="px-2 py-[2px]">
                    {s.index === 0 ? "★" : ""}
                    {s.index}
                  </td>
                  <td className="px-2 py-[2px]">${hex2(s.x)}</td>
                  <td className="px-2 py-[2px]">${hex2(s.y)}</td>
                  <td className="px-2 py-[2px]">${hex2(s.tile)}</td>
                  <td className="px-2 py-[2px]">{4 + pal}</td>
                  <td className="px-2 py-[2px]">{behind ? "bg" : "fg"}</td>
                  <td className="px-2 py-[2px]">{flip}</td>
                </tr>
              );
            })}
          </tbody>
        </table>
      </div>
    </Module>
  );
}
