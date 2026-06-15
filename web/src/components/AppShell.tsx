import { useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { Toolbar } from "./Toolbar";
import { ScreenPanel } from "./ScreenPanel";
import { CpuStatePanel } from "./CpuStatePanel";
import { DisassemblyPanel } from "./DisassemblyPanel";
import { BreakpointsPanel } from "./BreakpointsPanel";
import { MemoryPanel } from "./MemoryPanel";
import { HelpModal } from "./HelpModal";
import { LoadCodeModal } from "./LoadCodeModal";
import { Toaster } from "./toast/Toaster";
import { Button } from "./ui/Button";

export function AppShell(): JSX.Element {
  const { status } = useEmulator();
  const [helpOpen, setHelpOpen] = useState(false);
  const [loadCodeOpen, setLoadCodeOpen] = useState(false);

  if (status === "loading") {
    return (
      <div
        data-testid="app-loading"
        className="flex h-[100dvh] items-center justify-center bg-[var(--b0)] font-sans text-[var(--tx-mut)]"
      >
        <span className="run-dot-pulse mr-2 inline-block h-[9px] w-[9px] rounded-full bg-[var(--acc)]" />
        Loading emulator…
      </div>
    );
  }

  if (status === "error") {
    return (
      <div
        data-testid="app-error"
        className="flex h-[100dvh] items-center justify-center bg-[var(--b0)] p-6"
      >
        <section className="tile-reveal max-w-sm rounded-[var(--radius)] border border-[var(--bd-strong)] bg-[var(--b1)] p-6 shadow-[var(--glow)]">
          <h1 className="mb-2 font-sans text-[15px] font-semibold text-[var(--tx)]">
            Failed to load emulator
          </h1>
          <p className="mb-4 text-[12px] text-[var(--tx-mut)]">
            The WebAssembly core could not be loaded.
          </p>
          <Button variant="primary" onClick={() => window.location.reload()}>
            Retry
          </Button>
        </section>
      </div>
    );
  }

  return (
    <div className="flex h-[100dvh] flex-col overflow-hidden bg-[var(--b0)] text-[var(--tx)]">
      <Toolbar
        onHelp={() => setHelpOpen(true)}
        onLoadCode={() => setLoadCodeOpen(true)}
      />

      <main
        data-testid="bento-grid"
        className="grid min-h-0 flex-1 gap-[10px] p-[10px]"
        style={{
          gridTemplateColumns: "286px 1.5fr 1.35fr",
          gridTemplateRows: "212px 1fr",
        }}
      >
        {/* col 1, row 1 */}
        <ScreenPanel />
        {/* col 1, row 2 */}
        <CpuStatePanel
          revealDelay={50}
          className="col-start-1 row-start-2"
        />
        {/* col 2, spans both rows — the glowing hero */}
        <DisassemblyPanel
          revealDelay={100}
          className="col-start-2 row-span-2 row-start-1"
        />
        {/* col 3, row 1 */}
        <MemoryPanel
          revealDelay={150}
          className="col-start-3 row-start-1"
        />
        {/* col 3, row 2 */}
        <BreakpointsPanel
          revealDelay={200}
          className="col-start-3 row-start-2"
        />
      </main>

      <HelpModal open={helpOpen} onClose={() => setHelpOpen(false)} />
      <LoadCodeModal
        open={loadCodeOpen}
        onClose={() => setLoadCodeOpen(false)}
      />
      <Toaster />
    </div>
  );
}
