// web/src/components/DisassemblyPanel.tsx
import { useEmulator } from "../emulator/EmulatorProvider";
import { useDisassembly } from "../emulator/useDisassembly";
import { DisassemblyRow } from "./DisassemblyRow";
import { Panel } from "./ui/Panel";

export function DisassemblyPanel(): JSX.Element {
  const { snapshot, breakpoints, actions } = useEmulator();
  const instructions = useDisassembly();
  const pc = snapshot?.registers.pc ?? -1;
  const breakpointSet = new Set(breakpoints);

  return (
    <Panel title="Disassembly">
      {instructions.length === 0 ? (
        <p className="text-[12px] text-[var(--text-muted)]">No disassembly available</p>
      ) : (
        <div className="overflow-auto">
          <table className="w-full border-collapse">
            <tbody>
              {instructions.map((instr) => (
                <DisassemblyRow
                  key={instr.address}
                  instr={instr}
                  isCurrent={instr.address === pc}
                  hasBreakpoint={breakpointSet.has(instr.address)}
                  onToggle={actions.toggleBreakpoint}
                />
              ))}
            </tbody>
          </table>
        </div>
      )}
    </Panel>
  );
}
