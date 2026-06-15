import { Panel } from "./ui/Panel";

export function ScreenPanel(): JSX.Element {
  return (
    <Panel title="Screen" className="h-full">
      <div className="flex h-full items-center justify-center">
        <div
          data-testid="screen-well"
          style={{ aspectRatio: "256 / 240" }}
          className="flex w-full max-w-[440px] flex-col items-center justify-center gap-2 rounded-md bg-[var(--screen-well)] shadow-inner ring-1 ring-[var(--border)]"
        >
          <span className="text-xs text-[var(--text-muted)]">
            video output arrives with the PPU
          </span>
          <span className="font-mono text-[11px] text-[var(--text-dim)]">
            256 x 240
          </span>
        </div>
      </div>
    </Panel>
  );
}
