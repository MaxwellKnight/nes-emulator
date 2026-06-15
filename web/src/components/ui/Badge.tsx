import React from "react";

export interface BadgeProps {
  active?: boolean;
  className?: string;
  title?: string;
  children: React.ReactNode;
}

export function Badge({
  active = false,
  className = "",
  title,
  children,
}: BadgeProps): JSX.Element {
  return (
    <span
      data-active={active ? "true" : "false"}
      title={title}
      className={`inline-flex min-w-[2rem] items-center justify-center rounded px-2 py-1 font-mono text-xs ${
        active
          ? "bg-[var(--accent)] text-white"
          : "bg-[var(--panel-2)] text-[var(--text-dim)]"
      } ${className}`.trim()}
    >
      {children}
    </span>
  );
}
