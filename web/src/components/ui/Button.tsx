import React from "react";

export type ButtonVariant =
  | "primary"
  | "secondary"
  | "success"
  | "info"
  | "danger"
  | "ghost";

export interface ButtonProps
  extends React.ButtonHTMLAttributes<HTMLButtonElement> {
  variant?: ButtonVariant;
}

const VARIANT_CLASSES: Record<ButtonVariant, string> = {
  primary: "bg-[var(--accent)] text-white hover:bg-[var(--accent-soft)]",
  secondary:
    "bg-[var(--panel-2)] text-[var(--text)] border border-[var(--border)] hover:bg-[var(--panel)]",
  success: "bg-[var(--success)] text-black hover:opacity-90",
  info: "bg-[var(--accent-soft)] text-black hover:opacity-90",
  danger: "bg-[var(--danger)] text-white hover:opacity-90",
  ghost:
    "bg-transparent text-[var(--text-muted)] hover:text-[var(--text)] hover:bg-[var(--panel-2)]",
};

export function Button({
  variant = "secondary",
  className = "",
  type = "button",
  ...rest
}: ButtonProps): JSX.Element {
  return (
    <button
      type={type}
      data-variant={variant}
      className={`inline-flex items-center justify-center gap-1.5 rounded-md px-3 py-1.5 text-xs font-medium transition-colors disabled:cursor-not-allowed disabled:opacity-40 ${VARIANT_CLASSES[variant]} ${className}`.trim()}
      {...rest}
    />
  );
}
