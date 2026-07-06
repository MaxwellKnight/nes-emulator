// web/src/wasm/loader.ts
import type { WasmModule } from "./bridge";

type EmscriptenFactory = (opts: {
  locateFile: (path: string) => string;
}) => Promise<WasmModule>;

/**
 * Dynamically import the Emscripten ESM glue and instantiate the module.
 * locateFile points the loader at the bundled .wasm asset URL so Vite
 * resolves it correctly in dev and build. Load failures are wrapped in a
 * typed Error.
 */
export async function loadEmulatorModule(): Promise<WasmModule> {
  try {
    const glue = (await import("./generated/cpu_wasm.js")) as unknown as {
      default: EmscriptenFactory;
    };
    const factory = glue.default;
    const module = await factory({
      locateFile: () =>
        new URL("./generated/cpu_wasm.wasm", import.meta.url).href,
    });
    return module;
  } catch (error) {
    const message = error instanceof Error ? error.message : String(error);
    throw new Error(`Failed to load emulator module: ${message}`);
  }
}
