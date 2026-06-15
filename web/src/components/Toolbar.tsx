import React, { useRef, useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { useToast } from "./toast/ToastProvider";
import { Button } from "./ui/Button";
import { ThemeToggle } from "./ui/ThemeToggle";

export interface ToolbarProps {
  romName?: string;
  onHelp?: () => void;
}

const CONTACT_LINK_CLASS =
  "text-[var(--text-muted)] transition-colors hover:text-[var(--text)]";

export function Toolbar({ romName, onHelp }: ToolbarProps): JSX.Element {
  const { snapshot, running, actions } = useEmulator();
  const { addToast } = useToast();
  const fileInputRef = useRef<HTMLInputElement>(null);
  const [loadedRomName, setLoadedRomName] = useState<string | null>(null);

  const instructionCount = snapshot?.stats.instructionCount ?? 0;
  const cycleCount = snapshot?.stats.cycleCount ?? 0;
  const displayedRomName = loadedRomName ?? romName;

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
      const bytes = new Uint8Array(buffer);
      actions.loadROM(bytes);
      setLoadedRomName(file.name);
      addToast(`ROM loaded: ${file.name} (${bytes.length} bytes)`, "success");
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
        {displayedRomName ? (
          <span className="font-mono text-xs text-[var(--text-muted)]">
            {displayedRomName}
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

        <div className="ml-1 flex items-center gap-2 border-l border-[var(--border)] pl-3 text-xs text-[var(--text-muted)]">
          <span className="hidden sm:inline">Developed by @Maxwell Knight</span>
          <a
            href="https://github.com/MaxwellKnight/nes-emulator"
            target="_blank"
            rel="noreferrer"
            aria-label="GitHub"
            title="GitHub"
            className={CONTACT_LINK_CLASS}
          >
            GitHub
          </a>
          <a
            href="https://www.linkedin.com/in/maxwell-knight/"
            target="_blank"
            rel="noreferrer"
            aria-label="LinkedIn"
            title="LinkedIn"
            className={CONTACT_LINK_CLASS}
          >
            LinkedIn
          </a>
          <a
            href="mailto:maxwell.knight98@gmail.com"
            aria-label="Email"
            title="Email"
            className={CONTACT_LINK_CLASS}
          >
            Email
          </a>
        </div>
      </div>
    </header>
  );
}
