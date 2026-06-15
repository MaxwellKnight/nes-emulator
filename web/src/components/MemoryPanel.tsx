// web/src/components/MemoryPanel.tsx
import { useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { useMemory } from "../emulator/useMemory";
import { useToast } from "./toast/ToastProvider";
import { MEMORY_PAGES, type MemoryPage, type MemoryPageId } from "../wasm/types";
import { Panel } from "./ui/Panel";
import { Button } from "./ui/Button";
import { MemoryEditModal } from "./MemoryEditModal";

const COLUMNS = Array.from({ length: 16 }, (_, i) => i);

function hex4(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(4, "0")}`;
}

function hex2(value: number): string {
  return value.toString(16).toUpperCase().padStart(2, "0");
}

function cellId(address: number): string {
  return `memory-cell-0x${address.toString(16).toLowerCase().padStart(4, "0")}`;
}

function parseAddress(raw: string): number | null {
  const text = raw.trim().replace(/^0x/i, "").replace(/^\$/, "");
  if (text.length === 0) return null;
  if (!/^[0-9A-Fa-f]+$/.test(text)) return null;
  const value = parseInt(text, 16);
  if (Number.isNaN(value) || value < 0 || value > 0xffff) return null;
  return value;
}

function findPage(id: MemoryPageId): MemoryPage {
  return MEMORY_PAGES.find((p) => p.id === id) ?? MEMORY_PAGES[0];
}

export function MemoryPanel(): JSX.Element {
  const { snapshot } = useEmulator();
  const { addToast } = useToast();
  const [pageId, setPageId] = useState<MemoryPageId>("zeropage");
  const [jump, setJump] = useState("");
  const [editAddress, setEditAddress] = useState<number | null>(null);

  const page = findPage(pageId);
  const { rows } = useMemory(page);

  const pc = snapshot?.registers.pc ?? -1;
  const sp = snapshot?.registers.sp ?? -1;
  const spAddress = 0x0100 + sp;

  const handlePageChange = (id: MemoryPageId) => {
    setPageId(id);
    addToast(`Memory view changed to ${findPage(id).label}`, "info");
  };

  const handleJump = () => {
    const addr = parseAddress(jump);
    if (addr === null) {
      addToast("Invalid memory address", "danger");
      return;
    }
    const aligned = addr & 0xff00;
    const target = MEMORY_PAGES.find((p) => p.start === aligned);
    if (target) setPageId(target.id);
    addToast(`Jumped to memory address ${hex4(aligned)}`, "info");
    setJump("");
  };

  return (
    <Panel title="Memory">
      <div className="mb-3 flex flex-wrap items-center gap-2">
        <select
          data-testid="memory-page-select"
          value={pageId}
          onChange={(e) => handlePageChange(e.target.value as MemoryPageId)}
          className="rounded bg-[var(--panel-2)] px-2 py-1 text-[12px] text-[var(--text)] outline-none"
        >
          {MEMORY_PAGES.map((p) => (
            <option key={p.id} value={p.id}>
              {p.label}
            </option>
          ))}
        </select>
        <input
          data-testid="memory-jump-input"
          value={jump}
          onChange={(e) => setJump(e.target.value)}
          onKeyDown={(e) => {
            if (e.key === "Enter") handleJump();
          }}
          placeholder="Jump to $XXXX"
          className="w-32 rounded bg-[var(--panel-2)] px-2 py-1 font-mono text-[12px] text-[var(--text)] outline-none focus:ring-1 focus:ring-[var(--accent)]"
        />
        <Button data-testid="memory-jump-button" onClick={handleJump}>
          Jump
        </Button>
      </div>
      <div className="overflow-auto">
        <table className="border-collapse font-mono text-[12px]">
          <thead>
            <tr className="text-[var(--text-muted)]">
              <th className="px-2 py-1 text-left">Addr</th>
              {COLUMNS.map((c) => (
                <th
                  key={c}
                  data-testid={`memory-col-header-${c.toString(16).toUpperCase()}`}
                  className="px-1.5 py-1"
                >
                  {c.toString(16).toUpperCase()}
                </th>
              ))}
              <th className="px-2 py-1 text-left">ASCII</th>
            </tr>
          </thead>
          <tbody>
            {rows.map((row) => (
              <tr key={row.address}>
                <td className="px-2 py-0.5 text-[var(--text-muted)]">{hex4(row.address)}</td>
                {row.bytes.map((byte, i) => {
                  const address = row.address + i;
                  const isPc = address === pc;
                  const isSp = pageId === "stack" && address === spAddress;
                  return (
                    <td
                      key={address}
                      data-testid={cellId(address)}
                      data-pc={String(isPc)}
                      data-sp={String(isSp)}
                      title={`Click to edit memory at ${hex4(address)}`}
                      onClick={() => setEditAddress(address)}
                      className={[
                        "cursor-pointer px-1.5 py-0.5 text-center",
                        isPc
                          ? "bg-[var(--danger)] text-white"
                          : isSp
                            ? "bg-[var(--success)] text-black"
                            : "text-[var(--text)] hover:bg-[var(--panel-2)]",
                      ].join(" ")}
                    >
                      {hex2(byte & 0xff)}
                    </td>
                  );
                })}
                <td className="px-2 py-0.5 text-[var(--text-dim)]">{row.ascii}</td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
      <MemoryEditModal
        address={editAddress ?? 0}
        open={editAddress !== null}
        onClose={() => setEditAddress(null)}
      />
    </Panel>
  );
}
