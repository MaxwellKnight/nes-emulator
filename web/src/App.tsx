import { ToastProvider } from "./components/toast/ToastProvider";
import { EmulatorProvider } from "./emulator/EmulatorProvider";
import { AppShell } from "./components/AppShell";
import type { WasmModule } from "./wasm/bridge";

export interface AppProps {
  loadModule?: () => Promise<WasmModule>;
}

export function App({ loadModule }: AppProps = {}): JSX.Element {
  return (
    <ToastProvider>
      <EmulatorProvider loadModule={loadModule}>
        <AppShell />
      </EmulatorProvider>
    </ToastProvider>
  );
}

export default App;
