// web/src/components/RegistersPanel.tsx
import { useEmulator } from "../emulator/EmulatorProvider";
import { MonoValue } from "./ui/MonoValue";

function hex2(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(2, "0")}`;
}

function hex4(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(4, "0")}`;
}

function Gauge({
  id,
  label,
  value,
  accent = false,
}: {
  id: string;
  label: string;
  value: string;
  accent?: boolean;
}): JSX.Element {
  return (
    <div
      data-testid={id}
      className="flex min-w-[74px] flex-col justify-center border-l border-[var(--bd)] px-[20px] first:border-l-0"
    >
      <span className="mb-[7px] font-sans text-[9px] font-semibold uppercase leading-none tracking-[0.16em] text-[var(--dim)]">
        {label}
      </span>
      <MonoValue
        value={value}
        className={[
          "text-[23px] font-semibold leading-none tracking-[0.5px]",
          accent ? "text-[var(--acc2)]" : "text-[var(--tx)]",
        ].join(" ")}
      />
    </div>
  );
}

/**
 * The register gauge cells of the CPU readout strip: A / X / Y / SP / PC.
 * Each gauge = tiny uppercase label above a large mono value; PC is accented.
 * Rendered without its own chrome so it composes into the readout strip.
 */
export function RegistersPanel(): JSX.Element | null {
  const { snapshot } = useEmulator();
  if (!snapshot) {
    return null;
  }
  const { registers } = snapshot;

  return (
    <div className="flex flex-none">
      <Gauge id="reg-a" label="A" value={hex2(registers.a)} />
      <Gauge id="reg-x" label="X" value={hex2(registers.x)} />
      <Gauge id="reg-y" label="Y" value={hex2(registers.y)} />
      <Gauge id="reg-sp" label="SP" value={hex2(registers.sp)} />
      <Gauge id="reg-pc" label="PC" value={hex4(registers.pc)} accent />
    </div>
  );
}
