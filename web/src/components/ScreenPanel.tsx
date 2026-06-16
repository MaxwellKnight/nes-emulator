import { Module } from "./ui/Module";

export interface ScreenPanelProps {
  revealDelay?: number;
  className?: string;
}

export function ScreenPanel({
  revealDelay,
  className,
}: ScreenPanelProps): JSX.Element {
  return (
    <Module
      title="Screen"
      status={<span>NTSC · 256×240</span>}
      revealDelay={revealDelay}
      className={className}
      bodyClassName="p-[11px]"
    >
      <div
        data-testid="screen-well"
        style={{ aspectRatio: "256 / 240" }}
        className="relative mx-auto my-auto h-full max-h-full w-auto max-w-full overflow-hidden rounded-[var(--radius-sm)] bg-black"
      >
        {/* a faint idle field so the well reads as a display, not an empty box */}
        <div className="absolute inset-0 bg-[radial-gradient(120%_90%_at_50%_-10%,rgba(126,133,242,0.10),transparent_60%)]" />
        {/* CRT scanline overlay */}
        <div
          aria-hidden
          className="pointer-events-none absolute inset-0 bg-[repeating-linear-gradient(transparent_0_2px,rgba(0,0,0,0.18)_2px_4px)] opacity-70"
        />
        <div className="absolute inset-0 flex flex-col items-center justify-center gap-1 px-2 text-center">
          <span className="font-sans text-[10px] uppercase tracking-[0.11em] text-[var(--dim)]">
            video output arrives with the PPU
          </span>
        </div>
        <div className="absolute inset-x-[9px] bottom-[6px] flex justify-between font-mono text-[9px] text-[var(--mut)] opacity-85">
          <span>256 × 240</span>
          <span>● 60fps</span>
        </div>
      </div>
    </Module>
  );
}
