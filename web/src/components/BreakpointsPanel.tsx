// web/src/components/BreakpointsPanel.tsx
import { useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { useToast } from "./toast/ToastProvider";
import { Module } from "./ui/Module";

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

function idFor(addr: number): string {
  return addr.toString(16).toLowerCase().padStart(4, "0");
}

export interface BreakpointsPanelProps {
  revealDelay?: number;
  className?: string;
}

export function BreakpointsPanel({
  revealDelay,
  className,
}: BreakpointsPanelProps): JSX.Element {
  const { breakpoints, running, actions } = useEmulator();
  const { addToast } = useToast();
  const [input, setInput] = useState("");
  const [removing, setRemoving] = useState<number | null>(null);

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
    // Brief slide-out before the context actually drops the breakpoint.
    setRemoving(addr);
    addToast(`Breakpoint removed from ${hex4(addr)}`, "info");
    window.setTimeout(() => {
      actions.removeBreakpoint(addr);
      setRemoving((cur) => (cur === addr ? null : cur));
    }, 180);
  };

  return (
    <Module
      title="Breakpoints"
      status={
        sorted.length > 0 ? (
          <span data-testid="breakpoint-count">{sorted.length} set</span>
        ) : undefined
      }
      revealDelay={revealDelay}
      className={className}
      bodyClassName="nes-scroll overflow-auto"
    >
      <div className="mb-[8px] flex gap-[7px]">
        <input
          data-testid="breakpoint-input"
          value={input}
          disabled={running}
          onChange={(e) => setInput(e.target.value)}
          onKeyDown={(e) => {
            if (e.key === "Enter") handleAdd();
          }}
          placeholder="address (hex)"
          className="press min-w-0 flex-1 rounded-[var(--radius-sm)] border border-[var(--bd-strong)] bg-[var(--b2)] px-[7px] py-[3px] font-mono text-[10px] text-[var(--tx)] outline-none placeholder:text-[var(--tx-dim)] focus:border-[var(--acc)] disabled:cursor-not-allowed disabled:opacity-40"
        />
        <button
          type="button"
          data-testid="breakpoint-add"
          disabled={running}
          onClick={handleAdd}
          className="press rounded-[var(--radius-sm)] bg-[var(--acc)] px-[9px] py-[3px] text-[10px] font-medium text-white hover:bg-[var(--acc-hi)] disabled:cursor-not-allowed disabled:opacity-40"
        >
          + Add
        </button>
      </div>
      {sorted.length === 0 ? (
        <p className="font-mono text-[10px] text-[var(--tx-dim)]">
          No breakpoints set
        </p>
      ) : (
        <ul className="flex flex-col gap-[2px]">
          {sorted.map((addr) => (
            <li
              key={addr}
              data-testid={`breakpoint-item-0x${idFor(addr)}`}
              className={[
                "flex items-center justify-between py-[2px]",
                removing === addr ? "bp-row-out" : "",
              ]
                .filter(Boolean)
                .join(" ")}
            >
              <span className="flex items-center gap-[5px] font-mono text-[11px] text-[var(--red)]">
                <span className="text-[8px] leading-none">●</span>
                {hex4(addr)}
              </span>
              <button
                type="button"
                data-testid={`breakpoint-remove-0x${idFor(addr)}`}
                title={`Remove breakpoint at ${hex4(addr)}`}
                disabled={running}
                onClick={() => handleRemove(addr)}
                className="press rounded px-[5px] text-[11px] text-[var(--tx-dim)] hover:bg-[var(--red)]/15 hover:text-[var(--red)] disabled:cursor-not-allowed disabled:opacity-40"
              >
                ✕
              </button>
            </li>
          ))}
        </ul>
      )}
    </Module>
  );
}
