import { useEffect, useState, type RefObject } from "react";

/**
 * Measures a scroll container's client height with a ResizeObserver and returns
 * how many fixed-height rows fit inside it: `Math.floor(height / rowPx)`.
 *
 * Used by the Memory and Disassembly panels so their bodies render *exactly*
 * enough rows to fill the measured height — no empty bottoms — and re-fill
 * whenever the window (and therefore the panel) resizes.
 */
export function useFillRows(
  ref: RefObject<HTMLElement | null>,
  rowPx: number,
  fallback = 16,
): number {
  const [rows, setRows] = useState(fallback);

  useEffect(() => {
    const el = ref.current;
    if (!el) return;

    const measure = () => {
      const height = el.clientHeight;
      if (height > 0 && rowPx > 0) {
        setRows(Math.max(1, Math.floor(height / rowPx)));
      }
    };

    measure();

    if (typeof ResizeObserver === "undefined") {
      return;
    }
    const observer = new ResizeObserver(measure);
    observer.observe(el);
    return () => observer.disconnect();
  }, [ref, rowPx]);

  return rows;
}
