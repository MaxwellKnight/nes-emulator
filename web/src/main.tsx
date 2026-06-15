import { StrictMode } from "react";
import { createRoot } from "react-dom/client";
import App from "./App";
import { loadModuleWithDevFallback } from "./emulator/devModule";

// UI / chrome / headings / labels: Outfit (500/600/700)
import "@fontsource/outfit/500.css";
import "@fontsource/outfit/600.css";
import "@fontsource/outfit/700.css";
// Data / addresses / hex / mnemonics / registers / memory: JetBrains Mono (400/500/600)
import "@fontsource/jetbrains-mono/400.css";
import "@fontsource/jetbrains-mono/500.css";
import "@fontsource/jetbrains-mono/600.css";

import "./styles/global.css";

const rootElement = document.getElementById("root");
if (!rootElement) {
  throw new Error("Root element #root not found");
}

createRoot(rootElement).render(
  <StrictMode>
    <App loadModule={loadModuleWithDevFallback} />
  </StrictMode>,
);
