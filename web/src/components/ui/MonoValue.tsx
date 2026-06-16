import { useEffect, useRef, useState } from "react";

export interface MonoValueProps {
  value: string;
  className?: string;
  "data-testid"?: string;
}

/**
 * A monospace value that briefly flashes an accent background when it changes.
 * Used for register / stack / memory values so live updates read as "alive".
 */
export function MonoValue({
  value,
  className = "",
  ...rest
}: MonoValueProps): JSX.Element {
  const [flash, setFlash] = useState(false);
  const prev = useRef(value);
  const mounted = useRef(false);

  useEffect(() => {
    // Don't flash on first paint, only on genuine changes.
    if (!mounted.current) {
      mounted.current = true;
      prev.current = value;
      return;
    }
    if (prev.current !== value) {
      prev.current = value;
      setFlash(false);
      // re-trigger the animation on the next frame
      const id = requestAnimationFrame(() => setFlash(true));
      const t = setTimeout(() => setFlash(false), 600);
      return () => {
        cancelAnimationFrame(id);
        clearTimeout(t);
      };
    }
  }, [value]);

  return (
    <span
      {...rest}
      className={`rounded-[3px] font-mono ${flash ? "value-flash" : ""} ${className}`.trim()}
    >
      {value}
    </span>
  );
}
