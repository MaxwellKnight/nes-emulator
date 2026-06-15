import React from "react";

export interface ModuleProps {
  /** Uppercase legend label cut into the top border (Outfit, with accent square). */
  title: string;
  /** Optional content rendered on the right edge of the top border (e.g. ● live). */
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

/**
 * A "module": a framed panel whose title is cut into its top border as a
 * legend (a small uppercase Outfit label with a tiny accent square, painted
 * over the border using the page background). No header bar, no border-bottom
 * divider — uniform on every module. An optional right-side status sits on the
 * border too. Hierarchy comes from the 1px border + typography, never glow.
 *
 * min-height:0 + overflow-hidden so internal scrollers work inside the fixed
 * viewport.
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
      className={["module tile-reveal", className].filter(Boolean).join(" ")}
      style={
        revealDelay !== undefined
          ? { animationDelay: `${revealDelay}ms` }
          : undefined
      }
    >
      <span className="module-legend">{title}</span>
      {status !== undefined && status !== null ? (
        <span className="module-status">{status}</span>
      ) : null}
      <div
        className={[
          "relative flex min-h-0 flex-1 flex-col overflow-hidden rounded-[var(--radius)]",
          flush ? "" : "p-[11px]",
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
