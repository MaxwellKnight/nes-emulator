// web/src/components/toast/Toaster.tsx
import { useToast } from "./ToastProvider";
import type { ToastType } from "./ToastProvider";

function toastClass(type: ToastType): string {
  switch (type) {
    case "success":
      return "border-[var(--grn)]/40 bg-[var(--grn)]/15 text-[var(--grn)]";
    case "warning":
      return "border-[var(--amb)]/40 bg-[var(--amb)]/15 text-[var(--amb)]";
    case "danger":
      return "border-[var(--red)]/40 bg-[var(--red)]/15 text-[var(--red)]";
    default:
      return "border-[var(--acc)]/40 bg-[var(--acc)]/15 text-[var(--acc-hi)]";
  }
}

function dotClass(type: ToastType): string {
  switch (type) {
    case "success":
      return "bg-[var(--grn)]";
    case "warning":
      return "bg-[var(--amb)]";
    case "danger":
      return "bg-[var(--red)]";
    default:
      return "bg-[var(--acc)]";
  }
}

export function Toaster(): JSX.Element {
  const { toasts } = useToast();
  return (
    <div
      aria-live="polite"
      className="pointer-events-none fixed bottom-4 right-4 z-50 flex flex-col items-end gap-2"
    >
      {toasts.map((toast) => (
        <div
          key={toast.id}
          role="status"
          className={`toast-in pointer-events-auto flex items-center gap-2 rounded-full border px-3.5 py-2 font-sans text-[12px] shadow-[0_10px_30px_rgba(0,0,0,0.4)] backdrop-blur-sm ${toastClass(
            toast.type,
          )}`}
        >
          <span
            aria-hidden
            className={`h-[7px] w-[7px] shrink-0 rounded-full ${dotClass(toast.type)}`}
          />
          {toast.message}
        </div>
      ))}
    </div>
  );
}
