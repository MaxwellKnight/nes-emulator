import { useEmulator } from "../emulator/EmulatorProvider";
import { Module } from "./ui/Module";
import { RegistersPanel } from "./RegistersPanel";
import { FlagsBlock } from "./FlagsBlock";
import { StackPanel } from "./StackPanel";

export interface CpuStatePanelProps {
  revealDelay?: number;
  className?: string;
}

function Sep(): JSX.Element {
  return <div aria-hidden className="my-[16px] w-px flex-none bg-[var(--bd)]" />;
}

/**
 * The CPU State readout strip — a full-width horizontal instrument readout that
 * sits directly under the toolbar. Register gauge cells (A/X/Y/SP/PC) divided
 * by hairline rules, then the Flags LED bank, then the Stack row. Fills its
 * fixed band naturally — no stretching, no voids.
 */
export function CpuStatePanel({
  revealDelay,
  className,
}: CpuStatePanelProps): JSX.Element {
  const { snapshot } = useEmulator();
  const status = snapshot?.registers.status ?? 0;
  const bits = status
    .toString(2)
    .padStart(8, "0")
    .replace(/(\d{4})(\d{4})/, "$1 $2");

  const statusReadout = (
    <span data-testid="cpu-status-byte">
      P ${status.toString(16).toUpperCase().padStart(2, "0")} · {bits}
    </span>
  );

  return (
    <Module
      title="CPU State"
      status={snapshot ? statusReadout : undefined}
      revealDelay={revealDelay}
      className={className}
      bodyClassName="flex-row items-stretch px-[4px] py-0"
    >
      <RegistersPanel />
      <Sep />
      <FlagsBlock />
      <Sep />
      <StackPanel />
    </Module>
  );
}
