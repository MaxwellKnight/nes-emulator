// web/src/components/StatisticsPanel.tsx
import { useEmulator } from "../emulator/EmulatorProvider";
import { Panel } from "./ui/Panel";

export function StatisticsPanel(): JSX.Element {
  const { snapshot } = useEmulator();
  const instructionCount = snapshot?.stats.instructionCount ?? 0;
  const cycleCount = snapshot?.stats.cycleCount ?? 0;
  const items: Array<{ id: string; label: string; value: number }> = [
    { id: "stat-instructions", label: "Instructions", value: instructionCount },
    { id: "stat-cycles", label: "Cycles", value: cycleCount },
  ];
  return (
    <Panel title="Statistics">
      <div className="grid grid-cols-2 gap-2">
        {items.map((item) => (
          <div
            key={item.id}
            className="flex flex-col items-center rounded bg-[var(--panel-2)] px-2 py-2"
          >
            <span className="text-[10px] font-semibold uppercase text-[var(--text-muted)]">
              {item.label}
            </span>
            <span
              data-testid={item.id}
              className="font-mono text-[15px] text-[var(--heading)]"
            >
              {item.value.toLocaleString("en-US")}
            </span>
          </div>
        ))}
      </div>
    </Panel>
  );
}
