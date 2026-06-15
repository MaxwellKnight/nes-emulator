// web/src/components/MemoryPanel.tsx
import { useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { useMemory } from "../emulator/useMemory";
import { useToast } from "./toast/ToastProvider";
import { MEMORY_PAGES, type MemoryPage, type MemoryPageId } from "../wasm/types";
import { Tile } from "./ui/Tile";
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

export interface MemoryPanelProps {
  revealDelay?: number;
  className?: string;
}

export function MemoryPanel({
  revealDelay,
  className,
}: MemoryPanelProps): JSX.Element {
  const { snapshot, running } = useEmulator();
  const { addToast } = useToast();
  const [page, setPage] = useState<MemoryPage>(() => findPage("zeropage"));
  const [jump, setJump] = useState("");
  const [editAddress, setEditAddress] = useState<number | null>(null);

  const { rows } = useMemory(page);

  const selectedPreset =
    MEMORY_PAGES.find((p) => p.start === page.start)?.id ?? "";

  const pc = snapshot?.registers.pc ?? -1;
  const sp = snapshot?.registers.sp ?? -1;
  const spAddress = 0x0100 + sp;

  const handlePageChange = (id: MemoryPageId) => {
    const next = findPage(id);
    setPage(next);
    addToast(`Memory view changed to ${next.label}`, "info");
  };

  const handleJump = () => {
    const addr = parseAddress(jump);
    if (addr === null) {
      addToast("Invalid memory address", "danger");
      return;
    }
    const aligned = addr & 0xff00;
    const navigated = aligned !== page.start;
    if (navigated) {
      const preset = MEMORY_PAGES.find((p) => p.start === aligned);
      setPage(
        preset ?? {
          id: "ram",
          label: `Page ${hex4(aligned)}`,
          start: aligned,
          size: 0x100,
        },
      );
      addToast(`Jumped to memory address ${hex4(aligned)}`, "success");
    }
    setJump("");
  };

  const inputClass =
    "press rounded-[var(--radius-sm)] border border-[var(--bd-strong)] bg-[var(--b2)] px-[7px] py-[3px] font-mono text-[10px] text-[var(--tx)] outline-none focus:border-[var(--acc)]";

  return (
    <Tile
      title="Memory"
      revealDelay={revealDelay}
      className={className}
      bodyClassName="flex flex-col"
    >
      <div className="mb-[7px] flex shrink-0 gap-[7px]">
        <select
          data-testid="memory-page-select"
          value={selectedPreset}
          onChange={(e) => handlePageChange(e.target.value as MemoryPageId)}
          className={`${inputClass} flex-1`}
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
          placeholder="$ jump →"
          className={`${inputClass} w-[88px] text-[var(--tx-mut)] placeholder:text-[var(--tx-dim)]`}
        />
        <button
          type="button"
          data-testid="memory-jump-button"
          onClick={handleJump}
          className="press rounded-[var(--radius-sm)] bg-[var(--acc)] px-[9px] py-[3px] text-[10px] font-medium text-white hover:bg-[var(--acc-hi)]"
        >
          Go
        </button>
      </div>

      <div
        data-running={String(running)}
        className={[
          "nes-scroll relative min-h-0 flex-1 overflow-auto",
          running ? "cursor-default select-none opacity-90 saturate-[0.7]" : "",
        ].join(" ")}
      >
        <table className="border-collapse font-mono text-[10px] tracking-[0.4px]">
          <thead>
            <tr className="text-[var(--tx-dim)]">
              <th className="px-1 py-px text-left font-normal" />
              {COLUMNS.map((c) => (
                <th
                  key={c}
                  data-testid={`memory-col-header-${c.toString(16).toUpperCase()}`}
                  className="px-[3px] py-px font-normal"
                >
                  {c.toString(16).toUpperCase()}
                </th>
              ))}
              <th className="px-2 py-px text-left font-normal">ASCII</th>
            </tr>
          </thead>
          <tbody>
            {rows.map((row) => (
              <tr key={row.address}>
                <td className="px-1 py-px text-[var(--tx-dim)]">
                  {row.address.toString(16).toUpperCase().padStart(4, "0")}
                </td>
                {row.bytes.map((byte, i) => {
                  const address = row.address + i;
                  const isPc = address === pc;
                  const isSp = page.start === 0x0100 && address === spAddress;
                  return (
                    <td
                      key={address}
                      data-testid={cellId(address)}
                      data-pc={String(isPc)}
                      data-sp={String(isSp)}
                      title={
                        running
                          ? undefined
                          : `Click to edit memory at ${hex4(address)}`
                      }
                      onClick={
                        running ? undefined : () => setEditAddress(address)
                      }
                      className={[
                        "px-[3px] py-px text-center transition-colors duration-[var(--dur)]",
                        running ? "cursor-default" : "cursor-pointer",
                        isPc
                          ? "rounded-[2px] bg-[var(--red)] font-semibold text-white"
                          : isSp
                            ? "rounded-[2px] bg-[var(--grn)] font-semibold text-black"
                            : !running
                              ? "text-[var(--tx)] hover:bg-[var(--b2)]"
                              : "text-[var(--tx)]",
                      ]
                        .filter(Boolean)
                        .join(" ")}
                    >
                      {hex2(byte & 0xff)}
                    </td>
                  );
                })}
                <td className="px-2 py-px text-[var(--tx-dim)]">{row.ascii}</td>
              </tr>
            ))}
          </tbody>
        </table>
        <div className="scroll-fade" />
      </div>

      <MemoryEditModal
        address={editAddress ?? 0}
        open={editAddress !== null}
        onClose={() => setEditAddress(null)}
      />
    </Tile>
  );
}
