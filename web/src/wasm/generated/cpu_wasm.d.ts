// Type declarations for the Emscripten ESM glue produced by CMake
// (cpu_wasm.js / cpu_wasm.wasm). The real files are gitignored and only
// exist after `docker compose run --rm dev` rebuilds the WASM artifacts.
// This stub lets `tsc` and Vitest compile the bridge before the artifact exists.
export interface EmscriptenModuleFactoryOptions {
  locateFile?: (path: string, scriptDirectory: string) => string;
}

export interface CpuWasmModule {
  cwrap(
    name: string,
    ret: string | null,
    args: string[],
  ): (...a: number[]) => number | string | void;
  ccall(
    name: string,
    ret: string | null,
    args: string[],
    values: Array<number | string>,
  ): number | string | void;
}

declare const factory: (
  options?: EmscriptenModuleFactoryOptions,
) => Promise<CpuWasmModule>;
export default factory;
