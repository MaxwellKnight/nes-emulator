import { useEffect, useRef, useState } from "react";
import type { Flags } from "../wasm/types";

// LED order matches the 6502 status byte high-to-low: N V (unused) B D I Z C.
// The unused bit is shown as an em-dash, per the mockup.
const FLAG_ORDER: Array<{ key: keyof Flags; label: string }> = [
  { key: "n", label: "N" },
  { key: "v", label: "V" },
  { key: "u", label: "–" },
  { key: "b", label: "B" },
  { key: "d", label: "D" },
  { key: "i", label: "I" },
  { key: "z", label: "Z" },
  { key: "c", label: "C" },
];

function Led({
  flagKey,
  label,
  set,
}: {
  flagKey: keyof Flags;
  label: string;
  set: boolean;
}): JSX.Element {
  // Pop when the LED transitions from off -> on.
  const [pop, setPop] = useState(false);
  const prev = useRef(set);
  useEffect(() => {
    if (set && !prev.current) {
      setPop(true);
      const t = setTimeout(() => setPop(false), 220);
      prev.current = set;
      return () => clearTimeout(t);
    }
    prev.current = set;
  }, [set]);

  return (
    <span
      data-testid={`flag-${flagKey}`}
      data-set={String(set)}
      title={`${label === "–" ? "Unused" : label} flag is ${set ? "set" : "clear"}`}
      className={[
        "flex h-[32px] w-[32px] items-center justify-center rounded-full border border-transparent font-mono text-[13px] font-bold transition-[background-color,color] duration-[var(--dur)]",
        set ? "bg-[var(--acc)] text-white" : "bg-[var(--b3)] text-[var(--dim)]",
        pop ? "led-pop" : "",
      ]
        .filter(Boolean)
        .join(" ")}
    >
      {label}
    </span>
  );
}

export function FlagsBadges({ flags }: { flags: Flags }): JSX.Element {
  return (
    <div className="flex gap-[6px]" role="group" aria-label="CPU flags">
      {FLAG_ORDER.map(({ key, label }) => (
        <Led key={key} flagKey={key} label={label} set={flags[key]} />
      ))}
    </div>
  );
}
