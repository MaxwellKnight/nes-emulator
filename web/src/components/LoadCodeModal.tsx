// web/src/components/LoadCodeModal.tsx
import { useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { parseOpcodes } from "../wasm/opcodes";
import { useToast } from "./toast/ToastProvider";
import { Modal } from "./ui/Modal";
import { Button } from "./ui/Button";

const EXAMPLE_PROGRAM = `; Load 5 into X, then count down to 0
A2 05      ; LDX #$05
CA         ; DEX
D0 FD      ; BNE -3
00         ; BRK`;

export interface LoadCodeModalProps {
  open: boolean;
  onClose: () => void;
}

/**
 * The "Load Code" popover, opened from the command bar. Wraps the opcode
 * textarea, an example program with copy-to-clipboard, and the load action.
 */
export function LoadCodeModal({ open, onClose }: LoadCodeModalProps): JSX.Element {
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
    onClose();
  };

  const handleCopy = () => {
    void navigator.clipboard.writeText(EXAMPLE_PROGRAM);
    addToast("Example program copied to clipboard", "info");
  };

  return (
    <Modal open={open} onClose={onClose} title="Load Code">
      <div data-testid="loadcode-modal">
        <textarea
          data-testid="loadcode-textarea"
          value={text}
          onChange={(e) => setText(e.target.value)}
          disabled={running}
          placeholder="Paste hex opcodes, e.g. A9 01 8D 00 02"
          rows={5}
          className="w-full resize-y rounded-[var(--radius-sm)] border border-[var(--bd)] bg-[var(--b2)] px-2 py-1.5 font-mono text-[12px] text-[var(--tx)] outline-none focus:border-[var(--acc)] disabled:cursor-not-allowed disabled:opacity-50"
        />
        <div className="mt-2 flex justify-end gap-2">
          <Button variant="ghost" onClick={onClose}>
            Cancel
          </Button>
          <Button
            variant="primary"
            data-testid="loadcode-button"
            onClick={handleLoad}
            disabled={running}
          >
            Load Opcodes
          </Button>
        </div>
        <div className="mt-4">
          <div className="mb-1 flex items-center justify-between">
            <span className="font-sans text-[9px] font-semibold uppercase tracking-[0.11em] text-[var(--tx-mut)]">
              Example program
            </span>
            <button
              type="button"
              data-testid="loadcode-copy"
              title="Copy example program to clipboard"
              onClick={handleCopy}
              className="press rounded px-2 py-0.5 text-[11px] text-[var(--acc-hi)] hover:bg-[var(--b2)]"
            >
              Copy
            </button>
          </div>
          <pre className="nes-scroll overflow-auto rounded-[var(--radius-sm)] bg-[var(--b2)] p-2 font-mono text-[11px] text-[var(--tx-dim)]">
            {EXAMPLE_PROGRAM}
          </pre>
        </div>
      </div>
    </Modal>
  );
}
