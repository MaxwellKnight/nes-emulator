// web/src/components/FlagsBlock.tsx
import { useEmulator } from "../emulator/EmulatorProvider";
import { FlagsBadges } from "./FlagsBadges";

const SECTION_LABEL =
  "font-sans text-[9px] font-semibold uppercase leading-none tracking-[0.16em] text-[var(--dim)]";

/**
 * The Flags LED bank block of the CPU readout strip: an uppercase label above
 * the eight-LED status bank (N V – B D I Z C). Lit = accent fill, smooth
 * transition, no glow.
 */
export function FlagsBlock(): JSX.Element | null {
  const { snapshot } = useEmulator();
  if (!snapshot) {
    return null;
  }
  return (
    <div className="flex flex-none flex-col justify-center gap-[9px] px-[20px]">
      <span className={SECTION_LABEL}>Flags</span>
      <FlagsBadges flags={snapshot.flags} />
    </div>
  );
}
