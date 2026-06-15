// web/src/components/DisassemblyRow.tsx
import type { DisassembledInstruction } from "../wasm/types";
import { formatOperand, type OperandKind } from "../wasm/operand-format";

const KIND_COLOR: Record<OperandKind, string> = {
  none: "text-[var(--text-dim)]",
  immediate: "text-[var(--accent-soft)]",
  relative: "text-[var(--success)]",
  zeropage: "text-[var(--text)]",
  absolute: "text-[var(--heading)]",
  indexed: "text-[var(--accent)]",
  indirect: "text-[var(--danger)]",
};

function hex4(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(4, "0")}`;
}

function hex2(value: number): string {
  return value.toString(16).toUpperCase().padStart(2, "0");
}

function opcodeBytes(instr: DisassembledInstruction): string {
  const parts: string[] = [`0x${hex2(instr.opcode)}`];
  if (instr.bytes >= 2) parts.push(`0x${hex2(instr.operand & 0xff)}`);
  if (instr.bytes >= 3) parts.push(`0x${hex2((instr.operand >> 8) & 0xff)}`);
  return parts.join(" ");
}

export interface DisassemblyRowProps {
  instr: DisassembledInstruction;
  isCurrent: boolean;
  hasBreakpoint: boolean;
  onToggle: (address: number) => void;
}

export function DisassemblyRow({
  instr,
  isCurrent,
  hasBreakpoint,
  onToggle,
}: DisassemblyRowProps): JSX.Element {
  const operand = formatOperand(instr);
  const idHex = `0x${instr.address.toString(16).toLowerCase().padStart(4, "0")}`;
  return (
    <tr
      data-testid={`disasm-row-${idHex}`}
      data-current={String(isCurrent)}
      data-breakpoint={String(hasBreakpoint)}
      onClick={() => onToggle(instr.address)}
      title={`Toggle breakpoint at ${hex4(instr.address)}`}
      className={[
        "cursor-pointer font-mono text-[12px] leading-tight",
        isCurrent ? "bg-[var(--accent)]/20" : "hover:bg-[var(--panel-2)]",
        hasBreakpoint ? "border-l-2 border-[var(--danger)]" : "border-l-2 border-transparent",
      ].join(" ")}
    >
      <td className="px-2 py-0.5 text-[var(--text-muted)]">{hex4(instr.address)}</td>
      <td className="px-2 py-0.5 text-[var(--text-dim)]">{opcodeBytes(instr)}</td>
      <td className="px-2 py-0.5 font-semibold text-[var(--heading)]">{instr.mnemonic}</td>
      <td
        data-testid={`disasm-operand-${idHex}`}
        data-kind={operand.kind}
        className={`px-2 py-0.5 ${KIND_COLOR[operand.kind]}`}
      >
        {operand.text}
      </td>
      <td className="px-2 py-0.5 text-right text-[var(--text-muted)]">
        {instr.bytes} B, {instr.cycles} cyc
      </td>
    </tr>
  );
}
