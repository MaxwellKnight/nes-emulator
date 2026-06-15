// web/src/components/HelpModal.tsx
import { Modal } from "./ui/Modal";
import { Button } from "./ui/Button";

interface HelpSection {
  title: string;
  body: string;
}

const SECTIONS: HelpSection[] = [
  {
    title: "Getting Started",
    body: "Load a program via Load Code, then use Step to execute one instruction at a time or Run for continuous execution. The registers, flags, stack and memory panels update after every step.",
  },
  {
    title: "Loading Code",
    body: "Paste hex opcodes into the Load Code panel and press Load Opcodes. Code is written starting at $0C00 and the program counter is set there. The example program counts X down from 5 to 0 and halts on BRK.",
  },
  {
    title: "Controls",
    body: "Run executes continuously until a breakpoint, BRK ($00) or Stop. Step runs a single instruction. Stop halts execution and re-enables the panels. Reset returns the CPU to its initial state.",
  },
  {
    title: "Breakpoints",
    body: "Add a breakpoint by hex address in the Breakpoints panel, or click any disassembly row to toggle one. Execution auto-stops when a breakpoint is hit. Remove breakpoints from the list or by clicking the row again.",
  },
  {
    title: "Memory Navigation",
    body: "Switch pages with the selector (Zero Page, Stack, RAM, Vectors) or jump to an address (page-aligned to $XX00). The PC cell is highlighted red; on the Stack page the SP cell is green. Click any cell to edit its value in hex, decimal or view its binary.",
  },
];

export interface HelpModalProps {
  open: boolean;
  onClose: () => void;
}

export function HelpModal({ open, onClose }: HelpModalProps): JSX.Element {
  return (
    <Modal open={open} onClose={onClose} title="Help">
      <div className="flex flex-col gap-4">
        {SECTIONS.map((section) => (
          <section key={section.title}>
            <h3 className="mb-1 text-[13px] font-semibold text-[var(--heading)]">
              {section.title}
            </h3>
            <p className="text-[12px] leading-relaxed text-[var(--text-muted)]">{section.body}</p>
          </section>
        ))}
      </div>
      <div className="mt-4 flex justify-end">
        <Button data-testid="help-close" onClick={onClose}>
          Close
        </Button>
      </div>
    </Modal>
  );
}
