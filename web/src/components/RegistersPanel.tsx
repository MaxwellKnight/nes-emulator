// web/src/components/RegistersPanel.tsx
import { useEmulator } from "../emulator/EmulatorProvider";
import { FlagsBadges } from "./FlagsBadges";
import { MonoValue } from "./ui/MonoValue";

function hex2(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(2, "0")}`;
}

function hex4(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(4, "0")}`;
}

const SECTION_LABEL =
  "font-sans text-[8.5px] font-semibold uppercase leading-none tracking-[0.1em] text-[var(--tx-dim)]";

function Reg({
  id,
  label,
  value,
}: {
  id: string;
  label: string;
  value: string;
}): JSX.Element {
  return (
    <div
      data-testid={id}
      className="flex min-w-[62px] items-center justify-between py-px font-mono text-[11px]"
    >
      <span className="text-[var(--tx-mut)]">{label}</span>
      <MonoValue value={value} className="font-semibold text-[var(--acc-hi)]" />
    </div>
  );
}

/**
 * The Registers section of the CPU State tile: A/X/Y and SP/PC plus the LED
 * flag grid. Rendered without its own tile chrome so it composes into the
 * combined CPU State tile.
 */
export function RegistersPanel(): JSX.Element | null {
  const { snapshot } = useEmulator();
  if (!snapshot) {
    return null;
  }
  const { registers, flags } = snapshot;

  return (
    <div>
      <div className="flex gap-4">
        <div className="flex flex-col">
          <Reg id="reg-a" label="A" value={hex2(registers.a)} />
          <Reg id="reg-x" label="X" value={hex2(registers.x)} />
          <Reg id="reg-y" label="Y" value={hex2(registers.y)} />
        </div>
        <div className="flex flex-col">
          <Reg id="reg-sp" label="SP" value={hex2(registers.sp)} />
          <Reg id="reg-pc" label="PC" value={hex4(registers.pc)} />
        </div>
      </div>
      <div className={`${SECTION_LABEL} mt-[7px] mb-1`}>Flags</div>
      <FlagsBadges flags={flags} />
    </div>
  );
}
