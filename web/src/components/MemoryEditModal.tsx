// web/src/components/MemoryEditModal.tsx
import { useEffect, useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { useToast } from "./toast/ToastProvider";
import { Modal } from "./ui/Modal";
import { Button } from "./ui/Button";

function hex4(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(4, "0")}`;
}

function parseByte(raw: string, radix: number): number | null {
  const text = raw.trim();
  if (text.length === 0) return null;
  const re = radix === 16 ? /^[0-9A-Fa-f]+$/ : /^[0-9]+$/;
  if (!re.test(text)) return null;
  const value = parseInt(text, radix);
  if (Number.isNaN(value) || value < 0 || value > 0xff) return null;
  return value;
}

export interface MemoryEditModalProps {
  address: number;
  open: boolean;
  onClose: () => void;
}

export function MemoryEditModal({ address, open, onClose }: MemoryEditModalProps): JSX.Element {
  const { dbg, actions } = useEmulator();
  const { addToast } = useToast();
  const [hex, setHex] = useState("00");
  const [dec, setDec] = useState("0");

  useEffect(() => {
    if (open) {
      const current = (dbg?.readMemory(address) ?? 0) & 0xff;
      setHex(current.toString(16).toUpperCase().padStart(2, "0"));
      setDec(String(current));
    }
  }, [open, address, dbg]);

  const handleHex = (raw: string) => {
    setHex(raw);
    const v = parseByte(raw, 16);
    if (v !== null) setDec(String(v));
  };

  const handleDec = (raw: string) => {
    setDec(raw);
    const v = parseByte(raw, 10);
    if (v !== null) setHex(v.toString(16).toUpperCase().padStart(2, "0"));
  };

  const value = parseByte(hex, 16);
  const decValue = parseByte(dec, 10);
  const binary = value === null ? "--------" : value.toString(2).padStart(8, "0");

  const handleSave = () => {
    if (value === null || decValue === null || decValue !== value) {
      addToast("Invalid byte value (0-255)", "danger");
      return;
    }
    actions.writeMemory(address, value);
    addToast(`Memory at ${hex4(address)} set to $${value.toString(16).toUpperCase().padStart(2, "0")}`, "success");
    onClose();
  };

  return (
    <Modal open={open} onClose={onClose} title={`Edit Memory ${hex4(address)}`}>
      <div data-testid="memory-edit-modal" data-address={`0x${address.toString(16).toLowerCase().padStart(4, "0")}`}>
        <p className="mb-3 text-[12px] text-[var(--text-muted)]">
          Address {hex4(address)} · Page ${(address & 0xff00).toString(16).toUpperCase().padStart(4, "0")} · Offset $
          {(address & 0xff).toString(16).toUpperCase().padStart(2, "0")}
        </p>
        <div className="flex flex-col gap-3">
          <label className="flex flex-col gap-1 text-[11px] text-[var(--text-muted)]">
            Hex
            <input
              data-testid="memedit-hex"
              value={hex}
              onChange={(e) => handleHex(e.target.value)}
              className="rounded bg-[var(--panel-2)] px-2 py-1 font-mono text-[13px] text-[var(--text)] outline-none focus:ring-1 focus:ring-[var(--accent)]"
            />
          </label>
          <label className="flex flex-col gap-1 text-[11px] text-[var(--text-muted)]">
            Decimal
            <input
              data-testid="memedit-dec"
              value={dec}
              onChange={(e) => handleDec(e.target.value)}
              className="rounded bg-[var(--panel-2)] px-2 py-1 font-mono text-[13px] text-[var(--text)] outline-none focus:ring-1 focus:ring-[var(--accent)]"
            />
          </label>
          <label className="flex flex-col gap-1 text-[11px] text-[var(--text-muted)]">
            Binary (derived)
            <input
              data-testid="memedit-bin"
              value={binary}
              readOnly
              className="rounded bg-[var(--panel)] px-2 py-1 font-mono text-[13px] text-[var(--text-dim)] outline-none"
            />
          </label>
        </div>
        <div className="mt-4 flex justify-end gap-2">
          <Button variant="ghost" onClick={onClose}>
            Cancel
          </Button>
          <Button data-testid="memedit-save" onClick={handleSave}>
            Save
          </Button>
        </div>
      </div>
    </Modal>
  );
}
