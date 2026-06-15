import { Tile } from "./ui/Tile";
import { RegistersPanel } from "./RegistersPanel";
import { StackPanel } from "./StackPanel";

export interface CpuStatePanelProps {
  revealDelay?: number;
  className?: string;
}

/**
 * The CPU State bento tile — merges Registers (with the LED flag grid) and the
 * Stack window into a single section, matching the bento mockup.
 */
export function CpuStatePanel({
  revealDelay,
  className,
}: CpuStatePanelProps): JSX.Element {
  return (
    <Tile
      title="CPU State"
      revealDelay={revealDelay}
      className={className}
      bodyClassName="nes-scroll overflow-auto"
    >
      <RegistersPanel />
      <StackPanel />
    </Tile>
  );
}
