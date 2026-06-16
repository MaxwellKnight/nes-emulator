import { useEffect, useRef } from "react";

// Keyboard -> NES controller button bit (matches the C++ controller layout:
// d0 A, d1 B, d2 Select, d3 Start, d4 Up, d5 Down, d6 Left, d7 Right).
const KEYMAP: Record<string, number> = {
  KeyX: 0x01, // A
  KeyZ: 0x02, // B
  ShiftLeft: 0x04, // Select
  ShiftRight: 0x04,
  Enter: 0x08, // Start
  ArrowUp: 0x10,
  ArrowDown: 0x20,
  ArrowLeft: 0x40,
  ArrowRight: 0x80,
};

export const CONTROL_HINTS: Array<{ keys: string; label: string }> = [
  { keys: "← ↑ ↓ →", label: "D-Pad" },
  { keys: "X", label: "A" },
  { keys: "Z", label: "B" },
  { keys: "Enter", label: "Start" },
  { keys: "Shift", label: "Select" },
];

/**
 * While `enabled`, map the keyboard to player 1's controller, pushing the live
 * button bitmask into the core on every key edge. Game keys are captured
 * (preventDefault) so arrows don't scroll the page during play.
 */
export function useController(
  setController: (state: number, port?: number) => void,
  enabled: boolean,
): void {
  const maskRef = useRef(0);

  useEffect(() => {
    if (!enabled) {
      maskRef.current = 0;
      setController(0);
      return;
    }
    const onDown = (e: KeyboardEvent) => {
      const bit = KEYMAP[e.code];
      if (bit === undefined || e.repeat) return;
      e.preventDefault();
      maskRef.current |= bit;
      setController(maskRef.current);
    };
    const onUp = (e: KeyboardEvent) => {
      const bit = KEYMAP[e.code];
      if (bit === undefined) return;
      e.preventDefault();
      maskRef.current &= ~bit;
      setController(maskRef.current);
    };
    window.addEventListener("keydown", onDown);
    window.addEventListener("keyup", onUp);
    return () => {
      window.removeEventListener("keydown", onDown);
      window.removeEventListener("keyup", onUp);
      maskRef.current = 0;
      setController(0);
    };
  }, [enabled, setController]);
}
