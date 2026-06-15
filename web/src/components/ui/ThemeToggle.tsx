import { useEffect, useState } from "react";
import { Button } from "./Button";

type Theme = "dark" | "light";

const STORAGE_KEY = "preferred-theme";

function readSavedTheme(): Theme {
  const saved = localStorage.getItem(STORAGE_KEY);
  return saved === "light" ? "light" : "dark";
}

function applyTheme(theme: Theme): void {
  document.documentElement.dataset.theme = theme;
}

export function ThemeToggle(): JSX.Element {
  const [theme, setTheme] = useState<Theme>("dark");

  useEffect(() => {
    const initial = readSavedTheme();
    setTheme(initial);
    applyTheme(initial);
  }, []);

  function toggle(): void {
    const next: Theme = theme === "dark" ? "light" : "dark";
    setTheme(next);
    applyTheme(next);
    localStorage.setItem(STORAGE_KEY, next);
  }

  return (
    <Button
      variant="ghost"
      onClick={toggle}
      aria-label="Toggle theme"
      title={theme === "dark" ? "Switch to light theme" : "Switch to dark theme"}
    >
      {theme === "dark" ? "Light" : "Dark"}
    </Button>
  );
}
