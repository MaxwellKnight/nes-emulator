import { mkdirSync, writeFileSync, existsSync } from "node:fs";
import { fileURLToPath } from "node:url";
import { dirname, resolve } from "node:path";

// The Emscripten ESM glue (src/wasm/generated/cpu_wasm.js) is a build artifact
// and is gitignored, so it is absent on a fresh clone / in CI. Vite needs a
// physical module at that path to resolve the dynamic import in loader.ts at
// transform time (vi.mock then overrides it in loader.test.ts). This global
// setup writes a minimal stub when the real artifact has not been built.
// Authored as plain JS so tsc (allowJs off) ignores it and no Node types are
// pulled into the browser app's typecheck.
export default function setup() {
  const here = dirname(fileURLToPath(import.meta.url));
  const generatedDir = resolve(here, "../wasm/generated");
  const stubPath = resolve(generatedDir, "cpu_wasm.js");
  if (!existsSync(stubPath)) {
    mkdirSync(generatedDir, { recursive: true });
    writeFileSync(
      stubPath,
      [
        "// Auto-generated test stub for the Emscripten ESM glue.",
        "// Written by vitest globalSetup when the real WASM artifact is absent",
        "// (fresh clone / CI). vi.mock() overrides it in loader tests.",
        "export default function cpuWasmFactory(_opts) {",
        '  throw new Error("cpu_wasm.js stub: real WASM artifact not built yet");',
        "}",
        "",
      ].join("\n"),
    );
  }
}
