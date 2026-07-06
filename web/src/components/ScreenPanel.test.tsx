import { describe, it, expect, vi, beforeEach } from "vitest";
import { render, screen } from "@testing-library/react";
import { ScreenPanel } from "./ScreenPanel";

// jsdom canvases have no 2D context; provide a mock and capture putImageData.
const putImageData = vi.fn();
const getContextMock = vi.fn(() => ({
  putImageData,
  imageSmoothingEnabled: true,
})) as unknown as HTMLCanvasElement["getContext"];

beforeEach(() => {
  putImageData.mockClear();
  HTMLCanvasElement.prototype.getContext = getContextMock;
  // jsdom lacks ImageData; provide a minimal constructor stand-in.
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

describe("ScreenPanel", () => {
  it("renders a 256x240 canvas", () => {
    render(<ScreenPanel framebuffer={null} />);
    const canvas = screen.getByTestId("screen-canvas") as HTMLCanvasElement;
    expect(canvas.tagName).toBe("CANVAS");
    expect(canvas.width).toBe(256);
    expect(canvas.height).toBe(240);
  });

  it("uses pixelated, integer-scaled rendering on the canvas", () => {
    render(<ScreenPanel framebuffer={null} />);
    const canvas = screen.getByTestId("screen-canvas") as HTMLCanvasElement;
    expect(canvas.style.imageRendering).toBe("pixelated");
  });

  it("blits a provided framebuffer via getContext + putImageData", () => {
    const fb = new Uint8ClampedArray(256 * 240 * 4);
    fb[0] = 0xff; // recognizable first pixel
    render(<ScreenPanel framebuffer={fb} />);
    expect(getContextMock).toHaveBeenCalledWith("2d");
    expect(putImageData).toHaveBeenCalledTimes(1);
    const img = putImageData.mock.calls[0][0] as ImageData;
    expect(img.width).toBe(256);
    expect(img.height).toBe(240);
    expect(img.data[0]).toBe(0xff);
  });

  it("does not blit when no framebuffer is given", () => {
    render(<ScreenPanel framebuffer={null} />);
    expect(putImageData).not.toHaveBeenCalled();
  });
});
