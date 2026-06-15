import { useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { Toolbar } from "./Toolbar";
import { ScreenPanel } from "./ScreenPanel";
import { DisassemblyPanel } from "./DisassemblyPanel";
import { RegistersPanel } from "./RegistersPanel";
import { StackPanel } from "./StackPanel";
import { BreakpointsPanel } from "./BreakpointsPanel";
import { StatisticsPanel } from "./StatisticsPanel";
import { LoadCodePanel } from "./LoadCodePanel";
import { MemoryPanel } from "./MemoryPanel";
import { HelpModal } from "./HelpModal";
import { Toaster } from "./toast/Toaster";
import { Panel } from "./ui/Panel";
import { Button } from "./ui/Button";

export function AppShell(): JSX.Element {
  const { status, running } = useEmulator();
  const [helpOpen, setHelpOpen] = useState(false);

  if (status === "loading") {
    return (
      <div
        data-testid="app-loading"
        className="flex h-screen items-center justify-center bg-[var(--bg)] text-[var(--text-muted)]"
      >
        Loading emulator…
      </div>
    );
  }

  if (status === "error") {
    return (
      <div
        data-testid="app-error"
        className="flex h-screen items-center justify-center bg-[var(--bg)]"
      >
        <Panel title="Failed to load emulator">
          <p className="mb-3 text-[12px] text-[var(--text-muted)]">
            The WebAssembly core could not be loaded.
          </p>
          <Button onClick={() => window.location.reload()}>Retry</Button>
        </Panel>
      </div>
    );
  }

  const dimmed = running ? "pointer-events-none opacity-60" : "";

  return (
    <div className="flex min-h-screen flex-col bg-[var(--bg)] text-[var(--text)]">
      <Toolbar onHelp={() => setHelpOpen(true)} />
      <main className="grid flex-1 grid-cols-1 gap-4 p-4 lg:grid-cols-[2fr_1fr]">
        <div className="flex flex-col gap-4">
          <ScreenPanel />
          <div className={dimmed}>
            <DisassemblyPanel />
          </div>
        </div>
        <aside className="flex flex-col gap-4">
          <RegistersPanel />
          <StackPanel />
          <BreakpointsPanel />
          <StatisticsPanel />
          <LoadCodePanel />
        </aside>
        <div className={`lg:col-span-2 ${dimmed}`}>
          <MemoryPanel />
        </div>
      </main>
      <HelpModal open={helpOpen} onClose={() => setHelpOpen(false)} />
      <Toaster />
    </div>
  );
}
