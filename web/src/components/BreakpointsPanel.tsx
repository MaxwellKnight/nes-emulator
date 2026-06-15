// web/src/components/BreakpointsPanel.tsx
import { useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { useToast } from "./toast/ToastProvider";
import { Panel } from "./ui/Panel";
import { Button } from "./ui/Button";

function hex4(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(4, "0")}`;
}

function parseAddress(raw: string): number | null {
  const text = raw.trim().replace(/^0x/i, "").replace(/^\$/, "");
  if (text.length === 0) return null;
  if (!/^[0-9A-Fa-f]+$/.test(text)) return null;
  const value = parseInt(text, 16);
  if (Number.isNaN(value) || value < 0 || value > 0xffff) return null;
  return value;
}

export function BreakpointsPanel(): JSX.Element {
  const { breakpoints, actions } = useEmulator();
  const { addToast } = useToast();
  const [input, setInput] = useState("");

  const sorted = [...breakpoints].sort((a, b) => a - b);

  const handleAdd = () => {
    const addr = parseAddress(input);
    if (addr === null) {
      addToast("Invalid breakpoint address", "danger");
      return;
    }
    actions.addBreakpoint(addr);
    addToast(`Breakpoint added at ${hex4(addr)}`, "success");
    setInput("");
  };

  const handleRemove = (addr: number) => {
    actions.removeBreakpoint(addr);
    addToast(`Breakpoint removed from ${hex4(addr)}`, "info");
  };

  return (
    <Panel title="Breakpoints">
      <div className="mb-3 flex gap-2">
        <input
          data-testid="breakpoint-input"
          value={input}
          onChange={(e) => setInput(e.target.value)}
          onKeyDown={(e) => {
            if (e.key === "Enter") handleAdd();
          }}
          placeholder="e.g. C000"
          className="min-w-0 flex-1 rounded bg-[var(--panel-2)] px-2 py-1 font-mono text-[12px] text-[var(--text)] outline-none focus:ring-1 focus:ring-[var(--accent)]"
        />
        <Button data-testid="breakpoint-add" onClick={handleAdd}>
          Add
        </Button>
      </div>
      {sorted.length === 0 ? (
        <p className="text-[12px] text-[var(--text-muted)]">No breakpoints set</p>
      ) : (
        <ul className="flex flex-col gap-1">
          {sorted.map((addr) => (
            <li
              key={addr}
              data-testid={`breakpoint-item-0x${addr.toString(16).toLowerCase().padStart(4, "0")}`}
              className="flex items-center justify-between rounded bg-[var(--panel-2)] px-2 py-1"
            >
              <span className="font-mono text-[12px] text-[var(--danger)]">{hex4(addr)}</span>
              <button
                type="button"
                data-testid={`breakpoint-remove-0x${addr.toString(16).toLowerCase().padStart(4, "0")}`}
                title={`Remove breakpoint at ${hex4(addr)}`}
                onClick={() => handleRemove(addr)}
                className="rounded px-2 py-0.5 text-[11px] text-[var(--text-muted)] hover:bg-[var(--danger)]/20 hover:text-[var(--danger)]"
              >
                Remove
              </button>
            </li>
          ))}
        </ul>
      )}
    </Panel>
  );
}
