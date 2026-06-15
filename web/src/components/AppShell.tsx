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
        className="flex h-[100dvh] items-center justify-center bg-[var(--b0)] font-sans text-[var(--mut)]"
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
        <section className="tile-reveal max-w-sm rounded-[var(--radius)] border border-[var(--bd2)] bg-[var(--b1)] p-6">
          <h1 className="mb-2 font-sans text-[15px] font-semibold text-[var(--tx)]">
            Failed to load emulator
          </h1>
          <p className="mb-4 text-[12px] text-[var(--mut)]">
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
        data-testid="cockpit"
        className="flex min-h-0 flex-1 flex-col gap-[12px] p-[12px]"
      >
        {/* CPU readout strip — full width, fixed band */}
        <CpuStatePanel revealDelay={0} className="h-[118px] flex-none" />

        {/* main grid — fills remaining height */}
        <div
          data-testid="cockpit-grid"
          className="grid min-h-0 flex-1 gap-[12px]"
          style={{ gridTemplateColumns: "1.35fr 1fr 1.18fr" }}
        >
          {/* left column: Screen (hero) over Breakpoints (compact) */}
          <div className="flex min-h-0 flex-col gap-[12px]">
            <ScreenPanel revealDelay={50} className="min-h-0 flex-1" />
            <BreakpointsPanel revealDelay={100} className="h-[148px] flex-none" />
          </div>

          {/* middle: Disassembly (full height, fills rows) */}
          <DisassemblyPanel revealDelay={150} className="min-h-0" />

          {/* right: Memory (full height, fills rows) */}
          <MemoryPanel revealDelay={200} className="min-h-0" />
        </div>
      </main>

      <HelpModal open={helpOpen} onClose={() => setHelpOpen(false)} />
      <LoadCodeModal open={loadCodeOpen} onClose={() => setLoadCodeOpen(false)} />
      <Toaster />
    </div>
  );
}
