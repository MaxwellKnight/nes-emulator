import React, { useEffect, useRef, useState } from "react";
import { useEmulator } from "../emulator/EmulatorProvider";
import { useToast } from "./toast/ToastProvider";
import { ThemeToggle } from "./ui/ThemeToggle";
import { DEFAULT_GAME, GAMES, romUrl, type Game } from "../games/catalog";

export interface ToolbarProps {
  romName?: string;
  onHelp?: () => void;
  onLoadCode?: () => void;
}

const CONTACT_LINK_CLASS =
  "press flex h-6 w-6 items-center justify-center rounded-md text-[var(--tx-dim)] hover:bg-[var(--b2)] hover:text-[var(--tx)]";

function fmt(n: number): string {
  return n.toLocaleString("en-US");
}

interface SegButtonProps extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  primary?: boolean;
}

function SegButton({
  primary = false,
  className = "",
  children,
  ...rest
}: SegButtonProps): JSX.Element {
  return (
    <button
      type="button"
      className={[
        "press border-r border-[var(--bd)] px-[12px] py-[5px] text-[10px] font-medium last:border-r-0 disabled:cursor-not-allowed disabled:opacity-40",
        primary
          ? "bg-[var(--acc)] font-semibold text-white hover:bg-[var(--acc2)]"
          : "text-[var(--tx)] hover:bg-[var(--b3)]",
        className,
      ]
        .filter(Boolean)
        .join(" ")}
      {...rest}
    >
      {children}
    </button>
  );
}

export function Toolbar({
  romName,
  onHelp,
  onLoadCode,
}: ToolbarProps): JSX.Element {
  const { snapshot, running, actions } = useEmulator();
  const { addToast } = useToast();
  const fileInputRef = useRef<HTMLInputElement>(null);
  const gamesMenuRef = useRef<HTMLDivElement>(null);
  const [loadedRomName, setLoadedRomName] = useState<string | null>(null);
  const [gamesOpen, setGamesOpen] = useState(false);

  const instructionCount = snapshot?.stats.instructionCount ?? 0;
  const cycleCount = snapshot?.stats.cycleCount ?? 0;
  // Falls back to the default game because EmulatorProvider auto-loads it on
  // ready (see catalog.DEFAULT_GAME); Load ROM / the Games menu overwrite it.
  const displayedRomName = loadedRomName ?? romName ?? DEFAULT_GAME.title;

  // Close the Games menu on an outside click or Escape.
  useEffect(() => {
    if (!gamesOpen) return;
    function onDocClick(e: MouseEvent): void {
      if (!gamesMenuRef.current?.contains(e.target as Node)) setGamesOpen(false);
    }
    function onKey(e: KeyboardEvent): void {
      if (e.key === "Escape") setGamesOpen(false);
    }
    document.addEventListener("mousedown", onDocClick);
    document.addEventListener("keydown", onKey);
    return () => {
      document.removeEventListener("mousedown", onDocClick);
      document.removeEventListener("keydown", onKey);
    };
  }, [gamesOpen]);

  function openFilePicker(): void {
    fileInputRef.current?.click();
  }

  async function loadGame(game: Game): Promise<void> {
    setGamesOpen(false);
    try {
      const res = await fetch(romUrl(game));
      if (!res.ok) throw new Error(`HTTP ${res.status}`);
      const bytes = new Uint8Array(await res.arrayBuffer());
      const status = actions.loadRom(bytes);
      if (status === 0) {
        setLoadedRomName(game.title);
        addToast(`${game.title} loaded — press ▶ Run to play`, "success");
      } else if (status === 2) {
        addToast("Unsupported mapper", "danger");
      } else {
        addToast("Invalid ROM file", "danger");
      }
    } catch {
      addToast(`Could not load ${game.title}`, "danger");
    }
  }

  function onFileChange(event: React.ChangeEvent<HTMLInputElement>): void {
    const file = event.target.files?.[0];
    if (!file) {
      return;
    }
    const reader = new FileReader();
    reader.onload = () => {
      const buffer = reader.result as ArrayBuffer;
      const bytes = new Uint8Array(buffer);
      const status = actions.loadRom(bytes);
      if (status === 0) {
        setLoadedRomName(file.name);
        addToast(`ROM loaded: ${file.name}`, "success");
      } else if (status === 2) {
        addToast("Unsupported mapper", "danger");
      } else {
        addToast("Invalid ROM file", "danger");
      }
    };
    reader.readAsArrayBuffer(file);
    event.target.value = "";
  }

  return (
    <header
      data-testid="app-toolbar"
      className="flex shrink-0 items-center gap-[10px] border-b border-[var(--bd)] bg-[var(--b1)] px-[13px] py-[9px]"
    >
      {/* identity */}
      <span
        data-testid="running-dot"
        data-running={String(running)}
        aria-hidden
        className={[
          "h-[7px] w-[7px] shrink-0 rounded-full transition-colors duration-[var(--dur)]",
          running ? "run-dot-pulse bg-[var(--grn)]" : "bg-[var(--dim)]",
        ].join(" ")}
      />
      <span className="font-sans text-[13px] font-bold leading-none tracking-[0.4px] text-[var(--tx)]">
        NES STUDIO
      </span>
      <span className="font-sans text-[11px] text-[var(--tx-mut)]">
        6502 Debugger
      </span>
      {displayedRomName ? (
        <div
          data-testid="now-playing"
          title={running ? "Currently playing" : "Loaded — press Run to play"}
          className="ml-1 flex items-center gap-[6px] rounded-full border border-[var(--bd-strong)] bg-[var(--b2)] py-[3px] pl-[8px] pr-[10px]"
        >
          <span aria-hidden className="text-[11px] leading-none">
            🎮
          </span>
          <span className="text-[8.5px] font-semibold uppercase leading-none tracking-[0.7px] text-[var(--tx-dim)]">
            {running ? "Playing" : "Ready"}
          </span>
          <span
            data-testid="now-playing-title"
            className="font-mono text-[11px] font-medium leading-none text-[var(--tx)]"
          >
            {displayedRomName}
          </span>
        </div>
      ) : null}

      {/* segmented transport control */}
      <div
        role="group"
        aria-label="Execution controls"
        className="ml-2 flex overflow-hidden rounded-lg border border-[var(--bd-strong)]"
      >
        <SegButton
          primary
          onClick={actions.run}
          disabled={running}
          aria-label="Run"
          title="Run (start continuous execution)"
        >
          <span aria-hidden>▶</span> Run
        </SegButton>
        <SegButton
          onClick={actions.step}
          disabled={running}
          aria-label="Step"
          title="Step (execute a single instruction)"
        >
          <span aria-hidden>⤓</span> Step
        </SegButton>
        <SegButton
          onClick={actions.stop}
          disabled={!running}
          aria-label="Stop"
          title="Stop (halt execution)"
        >
          <span aria-hidden>■</span> Stop
        </SegButton>
        <SegButton
          onClick={actions.reset}
          aria-label="Reset"
          title="Reset (reset the CPU)"
        >
          <span aria-hidden>↺</span> Reset
        </SegButton>
      </div>

      {/* right cluster */}
      <div className="ml-auto flex items-center gap-[13px]">
        <div className="flex gap-[13px] font-mono text-[10px] text-[var(--tx-mut)]">
          <span>
            cyc{" "}
            <b
              data-testid="stat-cycles"
              className="font-semibold text-[var(--acc-hi)]"
            >
              {fmt(cycleCount)}
            </b>
          </span>
          <span>
            ins{" "}
            <b
              data-testid="stat-instructions"
              className="font-semibold text-[var(--acc-hi)]"
            >
              {fmt(instructionCount)}
            </b>
          </span>
        </div>

        <input
          ref={fileInputRef}
          data-testid="rom-file-input"
          type="file"
          accept=".nes"
          className="hidden"
          onChange={onFileChange}
        />
        {/* Games library: bundled, freely-licensed homebrew (see catalog.ts) */}
        <div ref={gamesMenuRef} className="relative">
          <button
            type="button"
            data-testid="games-open"
            onClick={() => setGamesOpen((o) => !o)}
            aria-haspopup="menu"
            aria-expanded={gamesOpen}
            aria-label="Games"
            title="Load a bundled free game"
            className="press flex items-center gap-[5px] rounded-md border border-[var(--bd-strong)] bg-[var(--b2)] px-[9px] py-[4px] text-[10px] text-[var(--tx)] hover:bg-[var(--b3)]"
          >
            <span aria-hidden>🎮</span> Games
            <span aria-hidden className="text-[8px] opacity-60">
              ▼
            </span>
          </button>
          {gamesOpen ? (
            <div
              role="menu"
              data-testid="games-menu"
              className="absolute right-0 z-50 mt-[6px] w-[220px] overflow-hidden rounded-lg border border-[var(--bd-strong)] bg-[var(--b1)] shadow-lg"
            >
              <div className="border-b border-[var(--bd)] px-[11px] py-[7px] text-[8.5px] font-semibold uppercase tracking-[0.7px] text-[var(--tx-dim)]">
                Free homebrew games
              </div>
              {GAMES.map((game) => (
                <button
                  key={game.id}
                  type="button"
                  role="menuitem"
                  data-testid={`game-${game.id}`}
                  onClick={() => void loadGame(game)}
                  className="flex w-full items-center justify-between gap-2 px-[11px] py-[7px] text-left hover:bg-[var(--b3)]"
                >
                  <span className="flex flex-col">
                    <span className="text-[11px] font-medium text-[var(--tx)]">
                      {game.title}
                    </span>
                    <span className="text-[9px] text-[var(--tx-mut)]">
                      {game.author}
                    </span>
                  </span>
                  <span className="shrink-0 rounded-full border border-[var(--bd)] px-[6px] py-[1px] font-mono text-[8.5px] text-[var(--tx-dim)]">
                    {game.license}
                  </span>
                </button>
              ))}
            </div>
          ) : null}
        </div>
        <button
          type="button"
          onClick={openFilePicker}
          aria-label="Load ROM"
          title="Load a .nes ROM file from disk"
          className="press rounded-md border border-[var(--bd-strong)] bg-[var(--b2)] px-[9px] py-[4px] text-[10px] text-[var(--tx)] hover:bg-[var(--b3)]"
        >
          Load ROM
        </button>
        <button
          type="button"
          data-testid="loadcode-open"
          onClick={onLoadCode}
          aria-label="Load Code"
          title="Load and assemble hex opcodes"
          className="press rounded-md border border-[var(--bd-strong)] bg-[var(--b2)] px-[9px] py-[4px] text-[10px] text-[var(--tx)] hover:bg-[var(--b3)]"
        >
          Load Code
        </button>

        <ThemeToggle />
        <button
          type="button"
          onClick={onHelp}
          aria-label="Help"
          title="Help"
          className={CONTACT_LINK_CLASS}
        >
          ?
        </button>

        <span aria-hidden className="text-[var(--tx-dim)] opacity-40">
          |
        </span>

        <div className="flex items-center gap-1">
          <a
            href="https://github.com/MaxwellKnight/nes-emulator"
            target="_blank"
            rel="noreferrer"
            aria-label="GitHub"
            title="GitHub"
            className={CONTACT_LINK_CLASS}
          >
            <svg
              viewBox="0 0 16 16"
              width="14"
              height="14"
              fill="currentColor"
              aria-hidden
            >
              <path d="M8 0C3.58 0 0 3.58 0 8c0 3.54 2.29 6.53 5.47 7.59.4.07.55-.17.55-.38 0-.19-.01-.82-.01-1.49-2.01.37-2.53-.49-2.69-.94-.09-.23-.48-.94-.82-1.13-.28-.15-.68-.52-.01-.53.63-.01 1.08.58 1.23.82.72 1.21 1.87.87 2.33.66.07-.52.28-.87.51-1.07-1.78-.2-3.64-.89-3.64-3.95 0-.87.31-1.59.82-2.15-.08-.2-.36-1.02.08-2.12 0 0 .67-.21 2.2.82.64-.18 1.32-.27 2-.27.68 0 1.36.09 2 .27 1.53-1.04 2.2-.82 2.2-.82.44 1.1.16 1.92.08 2.12.51.56.82 1.27.82 2.15 0 3.07-1.87 3.75-3.65 3.95.29.25.54.73.54 1.48 0 1.07-.01 1.93-.01 2.2 0 .21.15.46.55.38A8.01 8.01 0 0 0 16 8c0-4.42-3.58-8-8-8z" />
            </svg>
          </a>
          <a
            href="https://www.linkedin.com/in/maxwell-knight/"
            target="_blank"
            rel="noreferrer"
            aria-label="LinkedIn"
            title="LinkedIn"
            className={CONTACT_LINK_CLASS}
          >
            <svg
              viewBox="0 0 16 16"
              width="14"
              height="14"
              fill="currentColor"
              aria-hidden
            >
              <path d="M13.63 13.63h-2.37V9.9c0-.89-.02-2.03-1.24-2.03-1.24 0-1.43.97-1.43 1.97v3.79H6.22V6h2.28v1.04h.03c.32-.6 1.1-1.24 2.26-1.24 2.41 0 2.86 1.59 2.86 3.65v4.18zM3.56 4.96a1.38 1.38 0 1 1 0-2.76 1.38 1.38 0 0 1 0 2.76zm1.19 8.67H2.36V6h2.39v7.63zM14.82 0H1.18C.53 0 0 .52 0 1.16v13.68C0 15.48.53 16 1.18 16h13.64c.65 0 1.18-.52 1.18-1.16V1.16C16 .52 15.47 0 14.82 0z" />
            </svg>
          </a>
          <a
            href="mailto:maxwell.knight98@gmail.com"
            aria-label="Email"
            title="Email"
            className={CONTACT_LINK_CLASS}
          >
            <svg
              viewBox="0 0 16 16"
              width="14"
              height="14"
              fill="currentColor"
              aria-hidden
            >
              <path d="M1.5 2.5h13c.55 0 1 .45 1 1v9c0 .55-.45 1-1 1h-13c-.55 0-1-.45-1-1v-9c0-.55.45-1 1-1zm.2 1.3 6.3 4.2 6.3-4.2H1.7zm-.2 1.06v7.34h13V4.86l-6.5 4.34L1.5 4.86z" />
            </svg>
          </a>
        </div>
        <span className="hidden font-sans text-[10px] text-[var(--tx-dim)] sm:inline">
          by Maxwell Knight
        </span>
      </div>
    </header>
  );
}
