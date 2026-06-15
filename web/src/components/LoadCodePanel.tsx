// web/src/components/LoadCodePanel.tsx
import { useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { parseOpcodes } from "../wasm/opcodes";
import { useToast } from "./toast/ToastProvider";
import { Panel } from "./ui/Panel";
import { Button } from "./ui/Button";

const EXAMPLE_PROGRAM = `; Load 5 into X, then count down to 0
A2 05      ; LDX #$05
CA         ; DEX
D0 FD      ; BNE -3
00         ; BRK`;

export function LoadCodePanel(): JSX.Element {
  const { actions, running } = useEmulator();
  const { addToast } = useToast();
  const [text, setText] = useState("");

  const handleLoad = () => {
    if (text.trim().length === 0) {
      addToast("Please enter opcodes", "warning");
      return;
    }
    let byteCount: number;
    try {
      byteCount = parseOpcodes(text).length;
    } catch (err) {
      const message =
        err instanceof Error ? err.message : "Failed to load opcodes";
      addToast(message, "danger");
      return;
    }
    if (byteCount === 0) {
      addToast("No valid opcodes found", "warning");
      return;
    }
    actions.loadOpcodes(text);
    addToast(`Loaded ${byteCount} bytes successfully`, "success");
  };

  const handleCopy = () => {
    void navigator.clipboard.writeText(EXAMPLE_PROGRAM);
    addToast("Example program copied to clipboard", "info");
  };

  return (
    <Panel title="Load Code">
      <textarea
        data-testid="loadcode-textarea"
        value={text}
        onChange={(e) => setText(e.target.value)}
        disabled={running}
        placeholder="Paste hex opcodes, e.g. A9 01 8D 00 02"
        rows={5}
        className="w-full resize-y rounded bg-[var(--panel-2)] px-2 py-1.5 font-mono text-[12px] text-[var(--text)] outline-none focus:ring-1 focus:ring-[var(--accent)] disabled:cursor-not-allowed disabled:opacity-50"
      />
      <div className="mt-2">
        <Button data-testid="loadcode-button" onClick={handleLoad} disabled={running}>
          Load Opcodes
        </Button>
      </div>
      <div className="mt-4">
        <div className="mb-1 flex items-center justify-between">
          <span className="text-[11px] font-semibold uppercase text-[var(--text-muted)]">
            Example program
          </span>
          <button
            type="button"
            data-testid="loadcode-copy"
            title="Copy example program to clipboard"
            onClick={handleCopy}
            className="rounded px-2 py-0.5 text-[11px] text-[var(--accent-soft)] hover:bg-[var(--panel-2)]"
          >
            Copy
          </button>
        </div>
        <pre className="overflow-auto rounded bg-[var(--panel-2)] p-2 font-mono text-[11px] text-[var(--text-dim)]">
          {EXAMPLE_PROGRAM}
        </pre>
      </div>
    </Panel>
  );
}
