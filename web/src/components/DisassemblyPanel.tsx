// web/src/components/DisassemblyPanel.tsx
import { useEffect, useRef } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { useDisassembly } from "../emulator/useDisassembly";
import { useFillRows } from "../emulator/useFillRows";
import { DisassemblyRow } from "./DisassemblyRow";
import { Module } from "./ui/Module";

export interface DisassemblyPanelProps {
  revealDelay?: number;
  className?: string;
}

// row height in px used to measure how many instructions fill the body
const ROW_PX = 23;
const BEFORE = 6;

export function DisassemblyPanel({
  revealDelay,
  className,
}: DisassemblyPanelProps): JSX.Element {
  const { snapshot, breakpoints, running, actions } = useEmulator();

  const scrollRef = useRef<HTMLDivElement>(null);
  const currentRef = useRef<HTMLDivElement>(null);

  // Measure the scroll body and disassemble enough instructions to fill it,
  // plus a comfortable overflow so the PC line can sit mid-panel.
  const fillRows = useFillRows(scrollRef, ROW_PX, 24);
  const instructions = useDisassembly(BEFORE, fillRows + BEFORE);

  const pc = snapshot?.registers.pc ?? -1;
  const breakpointSet = new Set(breakpoints);

  // Keep the current-PC row in view as PC moves. Guarded because some
  // environments (jsdom) don't implement scrollIntoView.
  useEffect(() => {
    const el = currentRef.current;
    if (el && typeof el.scrollIntoView === "function") {
      el.scrollIntoView({ block: "nearest" });
    }
  }, [pc, instructions]);

  const livePill = (
    <span className="inline-flex items-center gap-[5px] text-[var(--grn)]">
      <span className="h-[5px] w-[5px] rounded-full bg-[var(--grn)]" /> live
    </span>
  );

  return (
    <Module
      title="Disassembly"
      status={livePill}
      revealDelay={revealDelay}
      className={className}
      bodyClassName="p-0"
    >
      {instructions.length === 0 ? (
        <p className="p-[11px] font-mono text-[11px] text-[var(--mut)]">
          No disassembly available
        </p>
      ) : (
        <>
          <div
            ref={scrollRef}
            data-running={String(running)}
            className={[
              "nes-scroll min-h-0 flex-1 overflow-auto px-[11px] py-[8px]",
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
    </Module>
  );
}
