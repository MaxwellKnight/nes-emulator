import { useEffect, useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { useController } from "../emulator/useController";
import { Toolbar } from "./Toolbar";
import { ScreenPanel } from "./ScreenPanel";
import { PlayMode } from "./PlayMode";
import { PpuViewer } from "./PpuViewer";
import { OamViewer } from "./OamViewer";
import { CpuStatePanel } from "./CpuStatePanel";
import { DisassemblyPanel } from "./DisassemblyPanel";
import { BreakpointsPanel } from "./BreakpointsPanel";
import { MemoryPanel } from "./MemoryPanel";
import { HelpModal } from "./HelpModal";
import { LoadCodeModal } from "./LoadCodeModal";
import { Toaster } from "./toast/Toaster";
import { Button } from "./ui/Button";

export function AppShell(): JSX.Element {
  const { status, framebuffer, dbg, actions, running } = useEmulator();
  const [helpOpen, setHelpOpen] = useState(false);
  const [loadCodeOpen, setLoadCodeOpen] = useState(false);
  const [ppuOpen, setPpuOpen] = useState(false);
  const [play, setPlay] = useState(false);

  // Capture the keyboard for player 1 whenever the core is running — in the
  // cockpit (Toolbar "Run") as well as in the full-screen Play overlay. Without
  // this, a game launched from the cockpit could not be controlled at all.
  useController(actions.setController, running || play);
  useEffect(() => {
    if (play) actions.run();
    else actions.stop();
  }, [play, actions]);

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
          {/* left column: Screen (hero) over Breakpoints/PPU (compact) */}
          <div className="flex min-h-0 flex-col gap-[12px]">
            <ScreenPanel
              framebuffer={framebuffer}
              revealDelay={50}
              className="min-h-0 flex-1"
            />
            <div className="flex items-center justify-between">
              <button
                type="button"
                data-testid="play-toggle"
                onClick={() => setPlay(true)}
                className="press rounded-md border border-[var(--acc)] bg-[var(--acc)]/20 px-[12px] py-[4px] text-[10px] font-medium text-[var(--tx)] hover:bg-[var(--acc)]/30"
              >
                ▶ Play
              </button>
              <button
                type="button"
                data-testid="ppu-debug-toggle"
                aria-pressed={ppuOpen}
                onClick={() => setPpuOpen((v) => !v)}
                className="press rounded-md border border-[var(--bd-strong)] bg-[var(--b2)] px-[9px] py-[4px] text-[10px] text-[var(--tx)] hover:bg-[var(--b3)]"
              >
                {ppuOpen ? "Hide PPU Debug" : "Show PPU Debug"}
              </button>
            </div>
            {ppuOpen ? (
              <>
                <PpuViewer dbg={dbg} revealDelay={120} className="flex-none" />
                <OamViewer dbg={dbg} revealDelay={150} className="flex-none" />
              </>
            ) : (
              <BreakpointsPanel
                revealDelay={100}
                className="h-[148px] flex-none"
              />
            )}
          </div>

          {/* middle: Disassembly (full height, fills rows) */}
          <DisassemblyPanel revealDelay={150} className="min-h-0" />

          {/* right: Memory (full height, fills rows) */}
          <MemoryPanel revealDelay={200} className="min-h-0" />
        </div>
      </main>

      {play ? (
        <PlayMode framebuffer={framebuffer} onExit={() => setPlay(false)} />
      ) : null}

      <HelpModal open={helpOpen} onClose={() => setHelpOpen(false)} />
      <LoadCodeModal open={loadCodeOpen} onClose={() => setLoadCodeOpen(false)} />
      <Toaster />
    </div>
  );
}
