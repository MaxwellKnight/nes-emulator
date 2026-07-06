import { useEffect, useRef } from "react";
import { CONTROL_HINTS } from "../emulator/useController";
import { SCREEN_HEIGHT, SCREEN_WIDTH } from "./ScreenPanel";

export interface PlayModeProps {
  framebuffer?: Uint8ClampedArray | null;
  onExit: () => void;
}

/**
 * Full-viewport gameplay overlay: a large, pixel-perfect NES screen with the
 * keyboard control legend. Used when the user enters Play mode; the debugger
 * cockpit is hidden behind it.
 */
export function PlayMode({ framebuffer, onExit }: PlayModeProps): JSX.Element {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    if (!framebuffer) return;
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext("2d");
    if (!ctx) return;
    ctx.imageSmoothingEnabled = false;
    ctx.putImageData(
      new ImageData(
        new Uint8ClampedArray(
          framebuffer.buffer as ArrayBuffer,
          framebuffer.byteOffset,
          framebuffer.byteLength,
        ),
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
      ),
      0,
      0,
    );
  }, [framebuffer]);

  // Esc exits play mode.
  useEffect(() => {
    const onKey = (e: KeyboardEvent) => {
      if (e.code === "Escape") onExit();
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [onExit]);

  return (
    <div
      data-testid="play-mode"
      className="fixed inset-0 z-50 flex flex-col items-center justify-center gap-5 bg-[var(--b0)]/95 backdrop-blur-sm"
    >
      <button
        type="button"
        data-testid="play-exit"
        onClick={onExit}
        className="press absolute right-5 top-5 rounded-md border border-[var(--bd-strong)] bg-[var(--b2)] px-3 py-[6px] text-[11px] text-[var(--tx)] hover:bg-[var(--b3)]"
      >
        Exit Play · Esc
      </button>

      <div
        style={{ aspectRatio: "256 / 240" }}
        className="relative h-[min(80vh,calc(80vw*240/256))] overflow-hidden rounded-[var(--radius)] border border-[var(--bd-strong)] bg-black shadow-2xl"
      >
        <canvas
          ref={canvasRef}
          data-testid="play-canvas"
          width={SCREEN_WIDTH}
          height={SCREEN_HEIGHT}
          style={{ imageRendering: "pixelated" }}
          className="absolute inset-0 h-full w-full"
        />
        <div
          aria-hidden
          className="pointer-events-none absolute inset-0 bg-[repeating-linear-gradient(transparent_0_3px,rgba(0,0,0,0.14)_3px_6px)] opacity-60"
        />
      </div>

      <div className="flex flex-wrap items-center justify-center gap-x-5 gap-y-2 font-mono text-[12px] text-[var(--tx-mut)]">
        {CONTROL_HINTS.map((h) => (
          <span key={h.label} className="flex items-center gap-2">
            <kbd className="rounded border border-[var(--bd-strong)] bg-[var(--b2)] px-2 py-[2px] text-[var(--tx)]">
              {h.keys}
            </kbd>
            {h.label}
          </span>
        ))}
      </div>
    </div>
  );
}
