import React, { useEffect } from "react";

export interface ModalProps {
  open: boolean;
  onClose: () => void;
  title: string;
  className?: string;
  children: React.ReactNode;
}

export function Modal({
  open,
  onClose,
  title,
  className = "",
  children,
}: ModalProps): JSX.Element | null {
  // ESC closes the dialog while it is open.
  useEffect(() => {
    if (!open) return;
    const onKey = (e: KeyboardEvent) => {
      if (e.key === "Escape") onClose();
    };
    window.addEventListener("keydown", onKey);
    return () => window.removeEventListener("keydown", onKey);
  }, [open, onClose]);

  if (!open) {
    return null;
  }
  return (
    <div className="fixed inset-0 z-50 flex items-center justify-center p-4">
      <div
        data-testid="modal-backdrop"
        className="overlay-in absolute inset-0 bg-black/55 backdrop-blur-sm"
        onClick={onClose}
      />
      <div
        role="dialog"
        aria-modal="true"
        aria-label={title}
        className={`dialog-in relative z-10 w-full max-w-lg rounded-[var(--radius)] border border-[var(--bd-strong)] bg-[var(--b1)] shadow-[0_24px_70px_rgba(0,0,0,0.55),var(--glow)] ${className}`.trim()}
      >
        <header className="flex items-center justify-between border-b border-[var(--bd)] px-4 py-3">
          <h2 className="font-sans text-[11px] font-semibold uppercase tracking-[0.11em] text-[var(--tx-mut)]">
            {title}
          </h2>
          <button
            type="button"
            aria-label="Close"
            onClick={onClose}
            className="press rounded p-1 text-[var(--tx-mut)] hover:bg-[var(--b2)] hover:text-[var(--tx)]"
          >
            ✕
          </button>
        </header>
        <div className="p-4">{children}</div>
      </div>
    </div>
  );
}
