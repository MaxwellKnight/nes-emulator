// web/src/emulator/useMemory.test.tsx
import { describe, it, expect } from "vitest";
import { renderHook, act, waitFor } from "@testing-library/react";
import type { ReactNode } from "react";
import { EmulatorProvider, useEmulator } from "./EmulatorProvider";
import { ToastProvider } from "../components/toast/ToastProvider";
import { useMemory } from "./useMemory";
import { createMockModule } from "../wasm/testing/mockModule";
import { MEMORY_PAGES, type MemoryPage } from "../wasm/types";

function getPage(id: MemoryPage["id"]): MemoryPage {
  const page = MEMORY_PAGES.find((p) => p.id === id);
  if (!page) throw new Error(`missing page ${id}`);
  return page;
}

function makeWrapper(mock: ReturnType<typeof createMockModule>) {
  return function Wrapper({ children }: { children: ReactNode }) {
    return (
      <ToastProvider>
        <EmulatorProvider loadModule={() => Promise.resolve(mock)}>
          {children}
        </EmulatorProvider>
      </ToastProvider>
    );
  };
}

function renderUseMemory(
  page: MemoryPage,
  mock: ReturnType<typeof createMockModule>,
  rowCount?: number,
) {
  return renderHook(
    () => {
      const emu = useEmulator();
      const mem = useMemory(page, rowCount);
      return { emu, mem };
    },
    { wrapper: makeWrapper(mock) },
  );
}

describe("useMemory", () => {
  it("returns 16 rows of 16 bytes for a 256-byte page", async () => {
    const mock = createMockModule();
    const zp = getPage("zeropage");
    const { result } = renderUseMemory(zp, mock);
    await waitFor(() => expect(result.current.emu.status).toBe("ready"));
    expect(result.current.mem.rows).toHaveLength(16);
    expect(result.current.mem.rows[0].bytes).toHaveLength(16);
    expect(result.current.mem.rows[0].address).toBe(0x0000);
    expect(result.current.mem.rows[15].address).toBe(0x00f0);
  });

  it("reflects seeded bytes and computes ascii for printable chars", async () => {
    const mock = createMockModule();
    mock._state.memory[0x0200] = 0x41; // 'A'
    mock._state.memory[0x0201] = 0x42; // 'B'
    mock._state.memory[0x0202] = 0x01; // non-printable -> '.'
    const ram = getPage("ram");
    const { result } = renderUseMemory(ram, mock);
    await waitFor(() => expect(result.current.emu.status).toBe("ready"));
    const firstRow = result.current.mem.rows[0];
    expect(firstRow.address).toBe(0x0200);
    expect(firstRow.bytes[0]).toBe(0x41);
    expect(firstRow.bytes[1]).toBe(0x42);
    expect(firstRow.bytes[2]).toBe(0x01);
    expect(firstRow.ascii.startsWith("AB.")).toBe(true);
  });

  it("yields exactly 6 bytes for the vectors page", async () => {
    const mock = createMockModule();
    mock._state.memory[0xfffa] = 0x11;
    mock._state.memory[0xffff] = 0x99;
    const vectors = getPage("vectors");
    const { result } = renderUseMemory(vectors, mock);
    await waitFor(() => expect(result.current.emu.status).toBe("ready"));
    const allBytes = result.current.mem.rows.flatMap((r) => r.bytes);
    expect(allBytes).toHaveLength(6);
    expect(result.current.mem.rows[0].address).toBe(0xfffa);
    expect(allBytes[0]).toBe(0x11);
    expect(allBytes[5]).toBe(0x99);
  });

  it("fill mode renders exactly rowCount contiguous rows, past the page boundary", async () => {
    const mock = createMockModule();
    mock._state.memory[0x0100] = 0xaa; // first byte of the page after zeropage
    const zp = getPage("zeropage");
    // 20 rows of 16 bytes = 320 bytes, which runs past the 256-byte page.
    const { result } = renderUseMemory(zp, mock, 20);
    await waitFor(() => expect(result.current.emu.status).toBe("ready"));
    expect(result.current.mem.rows).toHaveLength(20);
    expect(result.current.mem.rows[0].address).toBe(0x0000);
    // row 16 continues contiguously into $0100 (not clamped to the page)
    expect(result.current.mem.rows[16].address).toBe(0x0100);
    expect(result.current.mem.rows[16].bytes[0]).toBe(0xaa);
  });

  it("recomputes rows when the snapshot changes after a write", async () => {
    const mock = createMockModule();
    const zp = getPage("zeropage");
    const { result } = renderUseMemory(zp, mock);
    await waitFor(() => expect(result.current.emu.status).toBe("ready"));
    expect(result.current.mem.rows[0].bytes[5]).toBe(0x00);
    act(() => {
      result.current.emu.actions.writeMemory(0x0005, 0x7f);
    });
    await waitFor(() =>
      expect(result.current.mem.rows[0].bytes[5]).toBe(0x7f),
    );
  });
});
