// web/src/components/StackPanel.tsx
import { useEmulator } from "../emulator/EmulatorProvider";
import { Panel } from "./ui/Panel";

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

export function StackPanel(): JSX.Element {
  const { snapshot, dbg } = useEmulator();
  if (!snapshot || !dbg) {
    return <Panel title="Stack">{null}</Panel>;
  }
  const sp = snapshot.registers.sp;
  const lo = Math.max(0x00, sp - WINDOW_BEFORE);
  const hi = Math.min(0xff, sp + WINDOW_AFTER);

  const rows: Array<{ address: number; value: number; current: boolean }> = [];
  for (let offset = hi; offset >= lo; offset--) {
    const address = STACK_BASE + offset;
    rows.push({ address, value: dbg.readMemory(address) & 0xff, current: offset === sp });
  }

  return (
    <Panel title="Stack">
      <div className="mb-2 text-[11px] text-[var(--text-muted)]">
        SP:{" "}
        <span data-testid="stack-pointer-label" className="font-mono text-[var(--heading)]">
          {hex4(STACK_BASE + sp)}
        </span>
      </div>
      <div className="flex flex-col gap-0.5">
        {rows.map((row) => (
          <div
            key={row.address}
            data-testid={stackRowId(row.address)}
            data-current={String(row.current)}
            className={[
              "flex items-center justify-between rounded px-2 py-1 font-mono text-[12px]",
              row.current
                ? "bg-[var(--success)]/20 text-[var(--heading)]"
                : "bg-[var(--panel-2)] text-[var(--text)]",
            ].join(" ")}
          >
            <span className="text-[var(--text-muted)]">{hex4(row.address)}</span>
            <span>{hex2(row.value)}</span>
          </div>
        ))}
      </div>
    </Panel>
  );
}
