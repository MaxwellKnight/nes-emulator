import React from "react";

export interface TileProps {
  /** Uppercase label rendered in the tile header (Outfit). */
  title: string;
  /** Optional content rendered at the right edge of the header (e.g. a live pill). */
  headerRight?: React.ReactNode;
  /** Hero treatment: accent glow + stronger border (the Disassembly tile). */
  hero?: boolean;
  /** Page-load staggered reveal delay in ms. */
  revealDelay?: number;
  /** Extra classes for the outer <section> (grid placement, etc.). */
  className?: string;
  /** Extra classes for the inner body. */
  bodyClassName?: string;
  /** Removes default body padding (for full-bleed bodies like the screen well). */
  flush?: boolean;
  children: React.ReactNode;
}

/**
 * A bento tile: rounded surface, 1px border, an uppercase Outfit header and a
 * body that owns its own overflow. min-height:0 + overflow-hidden so any
 * internal scroller (Disassembly / Memory) works inside the fixed viewport.
 */
export function Tile({
  title,
  headerRight,
  hero = false,
  revealDelay,
  className = "",
  bodyClassName = "",
  flush = false,
  children,
}: TileProps): JSX.Element {
  return (
    <section
      className={[
        "tile-reveal flex min-h-0 flex-col overflow-hidden rounded-[var(--radius)] border bg-[var(--b1)]",
        hero
          ? "border-[var(--bd-strong)] shadow-[var(--glow)]"
          : "border-[var(--bd)]",
        className,
      ]
        .filter(Boolean)
        .join(" ")}
      style={
        revealDelay !== undefined
          ? { animationDelay: `${revealDelay}ms` }
          : undefined
      }
    >
      <header className="flex shrink-0 items-center justify-between border-b border-[var(--bd)] px-[10px] py-[6px]">
        <h2 className="font-sans text-[9px] font-semibold uppercase leading-none tracking-[0.11em] text-[var(--tx-mut)]">
          {title}
        </h2>
        {headerRight}
      </header>
      <div
        className={[
          "relative min-h-0 flex-1 overflow-hidden",
          flush ? "" : "p-[10px]",
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
