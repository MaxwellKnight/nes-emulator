// web/src/components/FlagsBadges.tsx
import type { Flags } from "../wasm/types";

const FLAG_ORDER: Array<{ key: keyof Flags; label: string }> = [
  { key: "n", label: "N" },
  { key: "v", label: "V" },
  { key: "u", label: "U" },
  { key: "b", label: "B" },
  { key: "d", label: "D" },
  { key: "i", label: "I" },
  { key: "z", label: "Z" },
  { key: "c", label: "C" },
];

export function FlagsBadges({ flags }: { flags: Flags }): JSX.Element {
  return (
    <div className="flex flex-wrap gap-1.5" role="group" aria-label="CPU flags">
      {FLAG_ORDER.map(({ key, label }) => {
        const set = flags[key];
        return (
          <span
            key={key}
            data-testid={`flag-${key}`}
            data-set={String(set)}
            title={`${label} flag is ${set ? "set" : "clear"}`}
            className={[
              "inline-flex min-w-[2.25rem] flex-col items-center rounded px-1.5 py-1 font-mono text-[11px] leading-tight",
              set
                ? "bg-[var(--accent)] text-white"
                : "bg-[var(--panel-2)] text-[var(--text-muted)]",
            ].join(" ")}
          >
            <span className="font-semibold">{label}</span>
            <span>{set ? "1" : "0"}</span>
          </span>
        );
      })}
    </div>
  );
}
