import React from "react";

export interface PanelProps {
  title: string;
  className?: string;
  children: React.ReactNode;
}

export function Panel({
  title,
  className = "",
  children,
}: PanelProps): JSX.Element {
  return (
    <section
      className={`flex flex-col rounded-lg border border-[var(--border)] bg-[var(--panel)] ${className}`.trim()}
    >
      <header className="border-b border-[var(--border)] px-4 py-2.5">
        <h2 className="text-[11px] font-semibold uppercase tracking-wide text-[var(--text-muted)]">
          {title}
        </h2>
      </header>
      <div className="flex-1 p-4">{children}</div>
    </section>
  );
}
