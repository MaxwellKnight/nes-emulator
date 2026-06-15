// web/src/components/StackPanel.tsx
import { useEmulator } from "../emulator/EmulatorProvider";
import { MonoValue } from "./ui/MonoValue";

const STACK_BASE = 0x0100;
const WINDOW_AFTER = 9; // SP + 9 entries shown left-to-right

function hex2(value: number): string {
  return value.toString(16).toUpperCase().padStart(2, "0");
}

function hex4(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(4, "0")}`;
}

function stackRowId(address: number): string {
  return `stack-row-0x${address.toString(16).toLowerCase().padStart(4, "0")}`;
}

const SECTION_LABEL =
  "font-sans text-[9px] font-semibold uppercase leading-none tracking-[0.16em] text-[var(--dim)]";

/**
 * The Stack block of the CPU readout strip: a horizontal window of stack bytes
 * starting at SP (top-of-stack) and walking upward. The top-of-stack chip is
 * highlighted green. Rendered without its own chrome so it composes into the
 * readout strip; fills the remaining width.
 */
export function StackPanel(): JSX.Element | null {
  const { snapshot, dbg } = useEmulator();
  if (!snapshot || !dbg) {
    return null;
  }
  const sp = snapshot.registers.sp;
  const hi = Math.min(0xff, sp + WINDOW_AFTER);

  const rows: Array<{ address: number; value: number; current: boolean }> = [];
  for (let offset = sp; offset <= hi; offset++) {
    const address = STACK_BASE + offset;
    rows.push({
      address,
      value: dbg.readMemory(address) & 0xff,
      current: offset === sp,
    });
  }

  return (
    <div className="flex flex-1 flex-col justify-center gap-[9px] px-[20px]">
      <span className={SECTION_LABEL}>
        Stack ·{" "}
        <span
          data-testid="stack-pointer-label"
          className="font-mono normal-case tracking-normal text-[var(--mut)]"
        >
          {hex4(STACK_BASE + sp)}
        </span>
      </span>
      <div className="flex gap-[5px]">
        {rows.map((row) => (
          <div
            key={row.address}
            data-testid={stackRowId(row.address)}
            data-current={String(row.current)}
            title={hex4(row.address)}
            className={[
              "min-w-[26px] rounded-[5px] border px-0 py-[5px] text-center font-mono text-[10px] transition-colors duration-[var(--dur)]",
              row.current
                ? "border-[var(--grn)] text-[var(--grn)]"
                : "border-[var(--bd)] bg-[var(--b2)] text-[var(--mut)]",
            ].join(" ")}
          >
            <span className="sr-only">{hex4(row.address)} </span>
            <MonoValue value={hex2(row.value)} />
          </div>
        ))}
      </div>
    </div>
  );
}
