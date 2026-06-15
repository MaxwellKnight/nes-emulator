import { render, screen, waitFor } from "@testing-library/react";
import { describe, it, expect } from "vitest";
import { App } from "./App";
import { createMockModule } from "./wasm/testing/mockModule";
import type { WasmModule } from "./wasm/bridge";

const loadMock = (): Promise<WasmModule> =>
  Promise.resolve(createMockModule({ disassembly: "3072|234|NOP|0||1|2#" }));

describe("App integration", () => {
  it("shows a loading state before the module resolves", () => {
    render(<App loadModule={() => new Promise<WasmModule>(() => {})} />);
    expect(screen.getByTestId("app-loading")).toBeInTheDocument();
  });

  it("renders the toolbar and the cockpit modules once the emulator is ready", async () => {
    render(<App loadModule={loadMock} />);
    await waitFor(() => {
      expect(screen.getByTestId("app-toolbar")).toBeInTheDocument();
    });
    expect(screen.getByTestId("cockpit")).toBeInTheDocument();
    expect(screen.getByTestId("cockpit-grid")).toBeInTheDocument();
    // module legends are cut into each module's top border
    expect(screen.getByText("CPU State")).toBeInTheDocument();
    expect(screen.getByText("Disassembly")).toBeInTheDocument();
    expect(screen.queryByTestId("app-loading")).toBeNull();
  });

  it("shows an error panel when the module fails to load", async () => {
    render(<App loadModule={() => Promise.reject(new Error("boom"))} />);
    await waitFor(() => {
      expect(screen.getByTestId("app-error")).toBeInTheDocument();
    });
  });
});
