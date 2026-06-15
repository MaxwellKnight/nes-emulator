// web/src/components/toast/Toaster.tsx
import { useToast } from "./ToastProvider";
import type { ToastType } from "./ToastProvider";

function toastColor(type: ToastType): string {
  switch (type) {
    case "success":
      return "bg-green-700 text-white";
    case "warning":
      return "bg-yellow-600 text-white";
    case "danger":
      return "bg-red-700 text-white";
    default:
      return "bg-[var(--panel)] text-[var(--text)]";
  }
}

export function Toaster(): JSX.Element {
  const { toasts } = useToast();
  return (
    <div
      aria-live="polite"
      className="pointer-events-none fixed bottom-4 right-4 z-50 flex flex-col gap-2"
    >
      {toasts.map((toast) => (
        <div
          key={toast.id}
          role="status"
          className={`pointer-events-auto rounded px-3 py-2 text-[12px] shadow ${toastColor(toast.type)}`}
        >
          {toast.message}
        </div>
      ))}
    </div>
  );
}
