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
  primary:
    "bg-[var(--acc)] text-white hover:bg-[var(--acc2)] active:bg-[var(--acc-press)]",
  secondary:
    "border border-[var(--bd-strong)] bg-[var(--b2)] text-[var(--tx)] hover:bg-[var(--b3)]",
  success: "bg-[var(--grn)] text-black hover:opacity-90",
  info: "bg-[var(--acc-hi)] text-black hover:opacity-90",
  danger: "bg-[var(--red)] text-white hover:opacity-90",
  ghost:
    "bg-transparent text-[var(--tx-mut)] hover:bg-[var(--b2)] hover:text-[var(--tx)]",
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
      className={`press inline-flex items-center justify-center gap-1.5 rounded-md px-3 py-1.5 text-xs font-medium disabled:cursor-not-allowed disabled:opacity-40 ${VARIANT_CLASSES[variant]} ${className}`.trim()}
      {...rest}
    />
  );
}
