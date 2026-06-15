import { Tile } from "./ui/Tile";

export function ScreenPanel(): JSX.Element {
  return (
    <Tile title="Screen" revealDelay={0} flush bodyClassName="p-2">
      <div
        data-testid="screen-well"
        style={{ aspectRatio: "256 / 240" }}
        className="relative mx-auto h-full max-h-full w-auto max-w-full overflow-hidden rounded-[var(--radius-sm)] bg-[var(--screen-well)] shadow-[inset_0_0_0_1px_var(--bd),inset_0_0_40px_rgba(0,0,0,0.6)]"
      >
        {/* subtle idle scene so the well reads as a display, not an empty box */}
        <div className="absolute inset-0 bg-[radial-gradient(120%_90%_at_50%_-10%,rgba(124,131,240,0.12),transparent_60%)]" />
        {/* CRT scanline overlay */}
        <div
          aria-hidden
          className="pointer-events-none absolute inset-0 bg-[repeating-linear-gradient(transparent_0_2px,rgba(0,0,0,0.18)_2px_3px)] opacity-70"
        />
        <div className="absolute inset-0 flex flex-col items-center justify-center gap-1 px-2 text-center">
          <span className="font-sans text-[10px] uppercase tracking-[0.11em] text-[var(--tx-dim)]">
            video output arrives with the PPU
          </span>
        </div>
        <div className="absolute inset-x-[7px] bottom-[5px] flex justify-between font-mono text-[9px] text-[var(--tx-mut)] opacity-85">
          <span>256 × 240</span>
          <span>NTSC</span>
        </div>
      </div>
    </Tile>
  );
}
