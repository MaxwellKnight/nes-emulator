// web/src/components/DisassemblyRow.tsx
import type { DisassembledInstruction } from "../wasm/types";
import { formatOperand, type OperandKind } from "../wasm/operand-format";

const KIND_COLOR: Record<OperandKind, string> = {
  none: "text-[var(--tx-dim)]",
  immediate: "text-[#7fb5ff]",
  relative: "text-[var(--grn)]",
  zeropage: "text-[var(--tx)]",
  absolute: "text-[var(--grn)]",
  indexed: "text-[var(--acc-hi)]",
  indirect: "text-[var(--red)]",
};

function hex4(value: number): string {
  return `$${value.toString(16).toUpperCase().padStart(4, "0")}`;
}

function hex2(value: number): string {
  return value.toString(16).toUpperCase().padStart(2, "0");
}

function opcodeBytes(instr: DisassembledInstruction): string {
  const parts: string[] = [hex2(instr.opcode)];
  if (instr.bytes >= 2) parts.push(hex2(instr.operand & 0xff));
  if (instr.bytes >= 3) parts.push(hex2((instr.operand >> 8) & 0xff));
  return parts.join(" ");
}

export interface DisassemblyRowProps {
  instr: DisassembledInstruction;
  isCurrent: boolean;
  hasBreakpoint: boolean;
  disabled?: boolean;
  onToggle: (address: number) => void;
}

export function DisassemblyRow({
  instr,
  isCurrent,
  hasBreakpoint,
  disabled = false,
  onToggle,
}: DisassemblyRowProps): JSX.Element {
  const operand = formatOperand(instr);
  const idHex = `0x${instr.address.toString(16).toLowerCase().padStart(4, "0")}`;
  return (
    <div
      data-testid={`disasm-row-${idHex}`}
      data-current={String(isCurrent)}
      data-breakpoint={String(hasBreakpoint)}
      onClick={disabled ? undefined : () => onToggle(instr.address)}
      title={
        disabled ? undefined : `Toggle breakpoint at ${hex4(instr.address)}`
      }
      className={[
        "group flex items-center gap-[11px] rounded-[4px] px-[6px] py-[3px] font-mono text-[11px] leading-tight transition-[background-color,box-shadow] duration-[var(--dur)]",
        disabled ? "cursor-default" : "cursor-pointer",
        isCurrent
          ? "bg-[var(--acc)]/15 shadow-[inset_2px_0_0_var(--acc),var(--glow)]"
          : !disabled
            ? "hover:bg-[var(--b2)]"
            : "",
      ]
        .filter(Boolean)
        .join(" ")}
    >
      <span
        aria-hidden
        className={[
          "h-[7px] w-[7px] shrink-0 rounded-full transition-[background-color,box-shadow] duration-[var(--dur)]",
          hasBreakpoint
            ? "bg-[var(--red)] shadow-[var(--red-glow)]"
            : "bg-transparent group-hover:bg-[var(--bd-strong)]",
        ].join(" ")}
      />
      <span className="w-[38px] shrink-0 text-[var(--tx-dim)]">
        {instr.address.toString(16).toUpperCase().padStart(4, "0")}
      </span>
      <span className="w-[64px] shrink-0 text-[#4f566b]">
        {opcodeBytes(instr)}
      </span>
      <span className="w-[34px] shrink-0 font-semibold text-[var(--acc-hi)]">
        {instr.mnemonic}
      </span>
      <span
        data-testid={`disasm-operand-${idHex}`}
        data-kind={operand.kind}
        className={KIND_COLOR[operand.kind]}
      >
        {operand.text}
      </span>
    </div>
  );
}
