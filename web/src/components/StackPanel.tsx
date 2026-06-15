// web/src/components/StackPanel.tsx
import { useEmulator } from "../emulator/EmulatorProvider";
import { MonoValue } from "./ui/MonoValue";

const STACK_BASE = 0x0100;
const WINDOW_BEFORE = 4;
const WINDOW_AFTER = 4;

function hex2(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(2, "0")}`;
}

function hex4(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(4, "0")}`;
}

function stackRowId(address: number): string {
  return `stack-row-0x${address.toString(16).toLowerCase().padStart(4, "0")}`;
}

const SECTION_LABEL =
  "font-sans text-[8.5px] font-semibold uppercase leading-none tracking-[0.1em] text-[var(--tx-dim)]";

/**
 * The Stack section of the CPU State tile: a window of stack chips around SP.
 * Rendered without its own tile chrome so it composes into the CPU State tile.
 */
export function StackPanel(): JSX.Element | null {
  const { snapshot, dbg } = useEmulator();
  if (!snapshot || !dbg) {
    return null;
  }
  const sp = snapshot.registers.sp;
  const lo = Math.max(0x00, sp - WINDOW_BEFORE);
  const hi = Math.min(0xff, sp + WINDOW_AFTER);

  const rows: Array<{ address: number; value: number; current: boolean }> = [];
  for (let offset = hi; offset >= lo; offset--) {
    const address = STACK_BASE + offset;
    rows.push({
      address,
      value: dbg.readMemory(address) & 0xff,
      current: offset === sp,
    });
  }

  return (
    <div>
      <div className={`${SECTION_LABEL} mt-[10px] mb-1`}>
        Stack ·{" "}
        <span
          data-testid="stack-pointer-label"
          className="font-mono text-[var(--tx-mut)]"
        >
          {hex4(STACK_BASE + sp)}
        </span>
      </div>
      <div className="flex flex-wrap gap-1">
        {rows.map((row) => (
          <div
            key={row.address}
            data-testid={stackRowId(row.address)}
            data-current={String(row.current)}
            title={hex4(row.address)}
            className={[
              "flex items-center gap-1 rounded-[4px] px-[5px] py-[2px] font-mono text-[10px] transition-colors duration-[var(--dur)]",
              row.current
                ? "bg-[var(--grn)]/25 text-[var(--tx)] shadow-[var(--grn-glow)]"
                : "bg-[var(--b3)] text-[var(--tx-mut)]",
            ].join(" ")}
          >
            <span className="text-[8px] text-[var(--tx-dim)]">
              {hex4(row.address)}
            </span>
            <MonoValue value={hex2(row.value)} />
          </div>
        ))}
      </div>
    </div>
  );
}
