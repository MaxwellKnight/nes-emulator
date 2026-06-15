import React, { useRef } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { Button } from "./ui/Button";
import { ThemeToggle } from "./ui/ThemeToggle";

export interface ToolbarProps {
  romName?: string;
  onHelp?: () => void;
}

export function Toolbar({ romName, onHelp }: ToolbarProps): JSX.Element {
  const { snapshot, running, actions } = useEmulator();
  const fileInputRef = useRef<HTMLInputElement>(null);

  const instructionCount = snapshot?.stats.instructionCount ?? 0;
  const cycleCount = snapshot?.stats.cycleCount ?? 0;

  function openFilePicker(): void {
    fileInputRef.current?.click();
  }

  function onFileChange(event: React.ChangeEvent<HTMLInputElement>): void {
    const file = event.target.files?.[0];
    if (!file) {
      return;
    }
    const reader = new FileReader();
    reader.onload = () => {
      const buffer = reader.result as ArrayBuffer;
      actions.loadROM(new Uint8Array(buffer));
    };
    reader.readAsArrayBuffer(file);
    event.target.value = "";
  }

  return (
    <header data-testid="app-toolbar" className="flex items-center gap-3 border-b border-[var(--border)] bg-[var(--panel)] px-4 py-2">
      <div className="flex items-center gap-2">
        <span className="text-sm font-semibold text-[var(--heading)]">
          6502 Debugger
        </span>
        {romName ? (
          <span className="font-mono text-xs text-[var(--text-muted)]">
            {romName}
          </span>
        ) : null}
      </div>

      <div className="flex items-center gap-1.5">
        <Button
          variant="primary"
          onClick={actions.run}
          disabled={running}
        >
          Run
        </Button>
        <Button variant="secondary" onClick={actions.step} disabled={running}>
          Step
        </Button>
        <Button variant="danger" onClick={actions.stop} disabled={!running}>
          Stop
        </Button>
        <Button variant="secondary" onClick={actions.reset}>
          Reset
        </Button>
      </div>

      <div className="flex items-center gap-3 font-mono text-xs text-[var(--text-muted)]">
        <span>instr: {instructionCount}</span>
        <span>cyc: {cycleCount}</span>
      </div>

      <div className="ml-auto flex items-center gap-1.5">
        <input
          ref={fileInputRef}
          data-testid="rom-file-input"
          type="file"
          accept=".nes"
          className="hidden"
          onChange={onFileChange}
        />
        <Button variant="secondary" onClick={openFilePicker}>
          Load ROM
        </Button>
        <Button variant="ghost" onClick={onHelp}>
          Help
        </Button>
        <ThemeToggle />
      </div>
    </header>
  );
}
