// web/src/components/DisassemblyPanel.tsx
import { useEffect, useRef } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { useDisassembly } from "../emulator/useDisassembly";
import { DisassemblyRow } from "./DisassemblyRow";
import { Tile } from "./ui/Tile";

export interface DisassemblyPanelProps {
  revealDelay?: number;
  className?: string;
}

export function DisassemblyPanel({
  revealDelay,
  className,
}: DisassemblyPanelProps): JSX.Element {
  const { snapshot, breakpoints, running, actions } = useEmulator();
  const instructions = useDisassembly();
  const pc = snapshot?.registers.pc ?? -1;
  const breakpointSet = new Set(breakpoints);

  const scrollRef = useRef<HTMLDivElement>(null);
  const currentRef = useRef<HTMLDivElement>(null);

  // Keep the current-PC row in view as PC moves. Guarded because some
  // environments (jsdom) don't implement scrollIntoView.
  useEffect(() => {
    const el = currentRef.current;
    if (el && typeof el.scrollIntoView === "function") {
      el.scrollIntoView({ block: "nearest" });
    }
  }, [pc, instructions]);

  const livePill = (
    <span className="inline-flex items-center gap-1 rounded-full bg-[var(--grn)]/15 px-[7px] py-[2px] font-sans text-[9px] text-[var(--grn)]">
      <span className="text-[7px] leading-none">●</span> live
    </span>
  );

  return (
    <Tile
      title="Disassembly"
      hero
      headerRight={livePill}
      revealDelay={revealDelay}
      className={className}
      bodyClassName="p-0"
    >
      {instructions.length === 0 ? (
        <p className="p-[10px] font-mono text-[11px] text-[var(--tx-mut)]">
          No disassembly available
        </p>
      ) : (
        <>
          <div
            ref={scrollRef}
            data-running={String(running)}
            className={[
              "nes-scroll h-full overflow-auto px-[10px] py-[8px]",
              running ? "cursor-default select-none opacity-90 saturate-[0.7]" : "",
            ].join(" ")}
          >
            {instructions.map((instr) => {
              const isCurrent = instr.address === pc;
              return (
                <div key={instr.address} ref={isCurrent ? currentRef : undefined}>
                  <DisassemblyRow
                    instr={instr}
                    isCurrent={isCurrent}
                    hasBreakpoint={breakpointSet.has(instr.address)}
                    disabled={running}
                    onToggle={actions.toggleBreakpoint}
                  />
                </div>
              );
            })}
          </div>
          <div className="scroll-fade" />
        </>
      )}
    </Tile>
  );
}
