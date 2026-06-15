import React from "react";

export interface AppShellProps {
  toolbar: React.ReactNode;
  screen: React.ReactNode;
  disassembly: React.ReactNode;
  sidebar: React.ReactNode;
  memory: React.ReactNode;
}

export function AppShell({
  toolbar,
  screen,
  disassembly,
  sidebar,
  memory,
}: AppShellProps): JSX.Element {
  return (
    <div className="flex h-screen flex-col bg-[var(--bg)] text-[var(--text)]">
      <div data-testid="shell-toolbar" className="shrink-0">
        {toolbar}
      </div>
      <div className="grid min-h-0 flex-1 grid-cols-[minmax(0,1fr)_360px] gap-3 p-3">
        <div className="grid min-h-0 grid-rows-[auto_minmax(0,1fr)] gap-3">
          <div
            data-testid="shell-center"
            className="grid min-h-0 grid-cols-[minmax(0,1fr)_minmax(0,1.4fr)] gap-3"
          >
            <div className="min-h-0">{screen}</div>
            <div className="min-h-0 overflow-auto">{disassembly}</div>
          </div>
          <div data-testid="shell-memory" className="min-h-0 overflow-auto">
            {memory}
          </div>
        </div>
        <aside
          data-testid="shell-sidebar"
          className="flex min-h-0 flex-col gap-3 overflow-auto"
        >
          {sidebar}
        </aside>
      </div>
    </div>
  );
}
