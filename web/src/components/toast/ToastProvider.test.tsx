// web/src/components/toast/ToastProvider.test.tsx
import { describe, it, expect, beforeEach, afterEach, vi } from "vitest";
import { renderHook, act } from "@testing-library/react";
import type { ReactNode } from "react";
import { ToastProvider, useToast } from "./ToastProvider";

function wrapper({ children }: { children: ReactNode }) {
  return <ToastProvider>{children}</ToastProvider>;
}

describe("ToastProvider / useToast", () => {
  beforeEach(() => {
    vi.useFakeTimers();
  });
  afterEach(() => {
    vi.runOnlyPendingTimers();
    vi.useRealTimers();
  });

  it("starts with no toasts", () => {
    const { result } = renderHook(() => useToast(), { wrapper });
    expect(result.current.toasts).toEqual([]);
  });

  it("addToast appends a toast with message and type", () => {
    const { result } = renderHook(() => useToast(), { wrapper });
    act(() => {
      result.current.addToast("Saved", "success");
    });
    expect(result.current.toasts).toHaveLength(1);
    expect(result.current.toasts[0].message).toBe("Saved");
    expect(result.current.toasts[0].type).toBe("success");
  });

  it("defaults the type to info", () => {
    const { result } = renderHook(() => useToast(), { wrapper });
    act(() => {
      result.current.addToast("Hello");
    });
    expect(result.current.toasts[0].type).toBe("info");
  });

  it("auto-removes a toast after 3000ms", () => {
    const { result } = renderHook(() => useToast(), { wrapper });
    act(() => {
      result.current.addToast("Bye", "warning");
    });
    expect(result.current.toasts).toHaveLength(1);
    act(() => {
      vi.advanceTimersByTime(2999);
    });
    expect(result.current.toasts).toHaveLength(1);
    act(() => {
      vi.advanceTimersByTime(1);
    });
    expect(result.current.toasts).toHaveLength(0);
  });

  it("assigns unique ascending ids across multiple toasts", () => {
    const { result } = renderHook(() => useToast(), { wrapper });
    act(() => {
      result.current.addToast("a");
      result.current.addToast("b");
      result.current.addToast("c");
    });
    const ids = result.current.toasts.map((t) => t.id);
    expect(new Set(ids).size).toBe(3);
    expect(ids[0]).toBeLessThan(ids[1]);
    expect(ids[1]).toBeLessThan(ids[2]);
  });

  it("removes only the expired toast, keeping later ones", () => {
    const { result } = renderHook(() => useToast(), { wrapper });
    act(() => {
      result.current.addToast("first");
    });
    act(() => {
      vi.advanceTimersByTime(1500);
    });
    act(() => {
      result.current.addToast("second");
    });
    act(() => {
      vi.advanceTimersByTime(1500);
    });
    expect(result.current.toasts).toHaveLength(1);
    expect(result.current.toasts[0].message).toBe("second");
    act(() => {
      vi.advanceTimersByTime(1500);
    });
    expect(result.current.toasts).toHaveLength(0);
  });

  it("throws when useToast is used outside the provider", () => {
    expect(() => renderHook(() => useToast())).toThrow();
  });
});
