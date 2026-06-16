// web/src/emulator/useFillRows.test.tsx
import { describe, it, expect, beforeEach, afterEach, vi } from "vitest";
import { renderHook } from "@testing-library/react";
import { useRef } from "react";
import { useFillRows } from "./useFillRows";

describe("useFillRows", () => {
  let originalRO: typeof ResizeObserver | undefined;

  beforeEach(() => {
    originalRO = globalThis.ResizeObserver;
  });
  afterEach(() => {
    globalThis.ResizeObserver = originalRO as typeof ResizeObserver;
    vi.restoreAllMocks();
  });

  it("returns floor(height / rowPx) from the measured element", () => {
    // A stub element whose clientHeight is fixed at 200px.
    const el = document.createElement("div");
    Object.defineProperty(el, "clientHeight", { value: 200, configurable: true });

    const { result } = renderHook(() => {
      const ref = useRef<HTMLDivElement>(el);
      return useFillRows(ref, 20);
    });

    // 200 / 20 = 10 rows
    expect(result.current).toBe(10);
  });

  it("never returns fewer than one row", () => {
    const el = document.createElement("div");
    Object.defineProperty(el, "clientHeight", { value: 5, configurable: true });

    const { result } = renderHook(() => {
      const ref = useRef<HTMLDivElement>(el);
      return useFillRows(ref, 20);
    });

    expect(result.current).toBe(1);
  });

  it("falls back to the provided default when the element is unmeasured", () => {
    const el = document.createElement("div");
    Object.defineProperty(el, "clientHeight", { value: 0, configurable: true });

    const { result } = renderHook(() => {
      const ref = useRef<HTMLDivElement>(el);
      return useFillRows(ref, 20, 7);
    });

    expect(result.current).toBe(7);
  });

  it("observes the element via ResizeObserver", () => {
    const observe = vi.fn();
    const disconnect = vi.fn();
    globalThis.ResizeObserver = vi.fn().mockImplementation(() => ({
      observe,
      disconnect,
      unobserve: vi.fn(),
    })) as unknown as typeof ResizeObserver;

    const el = document.createElement("div");
    Object.defineProperty(el, "clientHeight", { value: 100, configurable: true });

    const { unmount } = renderHook(() => {
      const ref = useRef<HTMLDivElement>(el);
      return useFillRows(ref, 20);
    });

    expect(observe).toHaveBeenCalledWith(el);
    unmount();
    expect(disconnect).toHaveBeenCalled();
  });
});
