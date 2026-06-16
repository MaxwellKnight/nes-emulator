import { describe, it, expect, beforeEach } from "vitest";
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { ThemeToggle } from "./ThemeToggle";

describe("ThemeToggle", () => {
  beforeEach(() => {
    localStorage.clear();
    delete document.documentElement.dataset.theme;
  });

  it("defaults to dark on mount when nothing is saved", () => {
    render(<ThemeToggle />);
    expect(document.documentElement.dataset.theme).toBe("dark");
  });

  it("reads the saved theme from localStorage on mount", () => {
    localStorage.setItem("preferred-theme", "light");
    render(<ThemeToggle />);
    expect(document.documentElement.dataset.theme).toBe("light");
  });

  it("toggles from dark to light on click and persists it", async () => {
    render(<ThemeToggle />);
    expect(document.documentElement.dataset.theme).toBe("dark");
    await userEvent.click(screen.getByRole("button"));
    expect(document.documentElement.dataset.theme).toBe("light");
    expect(localStorage.getItem("preferred-theme")).toBe("light");
  });

  it("toggles back from light to dark on a second click and persists it", async () => {
    render(<ThemeToggle />);
    const btn = screen.getByRole("button");
    await userEvent.click(btn);
    await userEvent.click(btn);
    expect(document.documentElement.dataset.theme).toBe("dark");
    expect(localStorage.getItem("preferred-theme")).toBe("dark");
  });
});
