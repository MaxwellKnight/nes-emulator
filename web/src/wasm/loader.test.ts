// web/src/wasm/loader.test.ts
import { describe, it, expect, vi, beforeEach } from "vitest";

// Mock the generated ESM glue. The default export is the Emscripten factory.
const factory = vi.fn();

vi.mock("./generated/cpu_wasm.js", () => ({
  default: factory,
}));

import { loadEmulatorModule } from "./loader";

beforeEach(() => {
  factory.mockReset();
});

describe("loadEmulatorModule", () => {
  it("calls the factory with a locateFile that resolves the .wasm URL", async () => {
    const fakeModule = { cwrap: vi.fn(), ccall: vi.fn() };
    factory.mockResolvedValue(fakeModule);

    const result = await loadEmulatorModule();

    expect(factory).toHaveBeenCalledTimes(1);
    const opts = factory.mock.calls[0][0] as {
      locateFile: (path: string) => string;
    };
    expect(typeof opts.locateFile).toBe("function");

    const resolved = opts.locateFile("cpu_wasm.wasm");
    expect(resolved).toContain("cpu_wasm.wasm");

    expect(result).toBe(fakeModule);
  });

  it("wraps a factory failure in an Error", async () => {
    factory.mockRejectedValue(new Error("boom"));

    await expect(loadEmulatorModule()).rejects.toThrowError(
      /Failed to load emulator module/
    );
  });
});
