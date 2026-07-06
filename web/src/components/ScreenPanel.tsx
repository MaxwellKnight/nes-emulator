import { useEffect, useRef } from "react";
import { Module } from "./ui/Module";

export const SCREEN_WIDTH = 256;
export const SCREEN_HEIGHT = 240;

export interface ScreenPanelProps {
  /** RGBA framebuffer (256*240*4). When null, nothing is blitted. */
  framebuffer?: Uint8ClampedArray | null;
  revealDelay?: number;
  className?: string;
}

export function ScreenPanel({
  framebuffer = null,
  revealDelay,
  className,
}: ScreenPanelProps): JSX.Element {
  const canvasRef = useRef<HTMLCanvasElement>(null);

  useEffect(() => {
    if (!framebuffer) return;
    const canvas = canvasRef.current;
    if (!canvas) return;
    const ctx = canvas.getContext("2d");
    if (!ctx) return;
    ctx.imageSmoothingEnabled = false;
    const image = new ImageData(
      new Uint8ClampedArray(framebuffer.buffer as ArrayBuffer, framebuffer.byteOffset, framebuffer.byteLength),
      SCREEN_WIDTH,
      SCREEN_HEIGHT,
    );
    ctx.putImageData(image, 0, 0);
  }, [framebuffer]);

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
        <canvas
          ref={canvasRef}
          data-testid="screen-canvas"
          width={SCREEN_WIDTH}
          height={SCREEN_HEIGHT}
          style={{ imageRendering: "pixelated" }}
          className="absolute inset-0 h-full w-full"
        />
        {/* CRT scanline overlay */}
        <div
          aria-hidden
          className="pointer-events-none absolute inset-0 bg-[repeating-linear-gradient(transparent_0_2px,rgba(0,0,0,0.18)_2px_4px)] opacity-70"
        />
        <div className="absolute inset-x-[9px] bottom-[6px] flex justify-between font-mono text-[9px] text-[var(--mut)] opacity-85">
          <span>256 × 240</span>
          <span>● 60fps</span>
        </div>
      </div>
    </Module>
  );
}
