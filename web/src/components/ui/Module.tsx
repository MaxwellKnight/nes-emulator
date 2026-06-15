import React from "react";

export interface ModuleProps {
  /** Uppercase header label (Outfit, with a small accent square). */
  title: string;
  /** Optional content rendered on the right of the header (e.g. ● live). */
  status?: React.ReactNode;
  /** Page-load staggered reveal delay in ms. */
  revealDelay?: number;
  /** Extra classes for the outer <section> (grid placement, flex sizing, etc.). */
  className?: string;
  /** Extra classes for the inner body. */
  bodyClassName?: string;
  /** Removes default body padding (for full-bleed bodies). */
  flush?: boolean;
  children: React.ReactNode;
}

/** Fixed header height shared by every module so they all line up. */
const HEADER_H = "h-[30px]";

/**
 * A framed panel with a consistent card header: a fixed-height row holding the
 * uppercase title (with a small accent square) and an optional right-aligned
 * status. There is deliberately NO header border-bottom — separation comes from
 * the header's height + typography, never a divider rule or glow. Every module
 * uses the same header height so the grid lines up.
 */
export function Module({
  title,
  status,
  revealDelay,
  className = "",
  bodyClassName = "",
  flush = false,
  children,
}: ModuleProps): JSX.Element {
  return (
    <section
      className={["module tile-reveal overflow-hidden", className]
        .filter(Boolean)
        .join(" ")}
      style={
        revealDelay !== undefined
          ? { animationDelay: `${revealDelay}ms` }
          : undefined
      }
    >
      <header
        className={`flex ${HEADER_H} flex-none items-center justify-between gap-2 px-[12px]`}
      >
        <span className="flex items-center gap-[7px] font-sans text-[9px] font-semibold uppercase leading-none tracking-[0.16em] text-[var(--mut)]">
          <span
            aria-hidden
            className="h-[5px] w-[5px] flex-none rounded-[1px] bg-[var(--acc)]"
          />
          {title}
        </span>
        {status !== undefined && status !== null ? (
          <span className="font-mono text-[9px] leading-none text-[var(--dim)]">
            {status}
          </span>
        ) : null}
      </header>
      <div
        className={[
          "relative flex min-h-0 flex-1 flex-col",
          flush ? "" : "px-[11px] pb-[11px]",
          bodyClassName,
        ]
          .filter(Boolean)
          .join(" ")}
      >
        {children}
      </div>
    </section>
  );
}
