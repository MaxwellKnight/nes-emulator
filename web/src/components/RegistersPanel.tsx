// web/src/components/RegistersPanel.tsx
import { useEmulator } from "../emulator/EmulatorProvider";
import { Panel } from "./ui/Panel";
import { FlagsBadges } from "./FlagsBadges";

function hex2(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(2, "0")}`;
}

function hex4(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(4, "0")}`;
}

export function RegistersPanel(): JSX.Element {
  const { snapshot } = useEmulator();
  if (!snapshot) {
    return <Panel title="Registers">{null}</Panel>;
  }
  const { registers, flags } = snapshot;
  const rows: Array<{ id: string; label: string; value: string }> = [
    { id: "reg-a", label: "A", value: hex2(registers.a) },
    { id: "reg-x", label: "X", value: hex2(registers.x) },
    { id: "reg-y", label: "Y", value: hex2(registers.y) },
    { id: "reg-sp", label: "SP", value: hex2(registers.sp) },
    { id: "reg-pc", label: "PC", value: hex4(registers.pc) },
  ];
  return (
    <Panel title="Registers">
      <div className="grid grid-cols-5 gap-2">
        {rows.map((row) => (
          <div
            key={row.id}
            data-testid={row.id}
            className="flex flex-col items-center rounded bg-[var(--panel-2)] px-2 py-1.5"
          >
            <span className="text-[10px] font-semibold uppercase text-[var(--text-muted)]">
              {row.label}
            </span>
            <span className="font-mono text-[13px] text-[var(--heading)]">{row.value}</span>
          </div>
        ))}
      </div>
      <div className="mt-3">
        <FlagsBadges flags={flags} />
      </div>
    </Panel>
  );
}
