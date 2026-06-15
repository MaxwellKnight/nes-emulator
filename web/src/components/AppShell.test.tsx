import { describe, it, expect } from "vitest";
import { render, screen } from "@testing-library/react";
import { AppShell } from "./AppShell";

describe("AppShell", () => {
  it("renders every named slot", () => {
    render(
      <AppShell
        toolbar={<div>TOOLBAR</div>}
        screen={<div>SCREEN</div>}
        disassembly={<div>DISASSEMBLY</div>}
        sidebar={<div>SIDEBAR</div>}
        memory={<div>MEMORY</div>}
      />,
    );
    expect(screen.getByText("TOOLBAR")).toBeInTheDocument();
    expect(screen.getByText("SCREEN")).toBeInTheDocument();
    expect(screen.getByText("DISASSEMBLY")).toBeInTheDocument();
    expect(screen.getByText("SIDEBAR")).toBeInTheDocument();
    expect(screen.getByText("MEMORY")).toBeInTheDocument();
  });

  it("places the toolbar in a region above the center area", () => {
    render(
      <AppShell
        toolbar={<div>TOOLBAR</div>}
        screen={<div>SCREEN</div>}
        disassembly={<div>DISASSEMBLY</div>}
        sidebar={<div>SIDEBAR</div>}
        memory={<div>MEMORY</div>}
      />,
    );
    const toolbarRegion = screen.getByTestId("shell-toolbar");
    const centerRegion = screen.getByTestId("shell-center");
    expect(toolbarRegion).toContainElement(screen.getByText("TOOLBAR"));
    expect(centerRegion).toContainElement(screen.getByText("SCREEN"));
    expect(centerRegion).toContainElement(screen.getByText("DISASSEMBLY"));
  });

  it("groups screen and disassembly in the center and renders the sidebar and memory regions", () => {
    render(
      <AppShell
        toolbar={<div>TOOLBAR</div>}
        screen={<div>SCREEN</div>}
        disassembly={<div>DISASSEMBLY</div>}
        sidebar={<div>SIDEBAR</div>}
        memory={<div>MEMORY</div>}
      />,
    );
    expect(screen.getByTestId("shell-sidebar")).toContainElement(
      screen.getByText("SIDEBAR"),
    );
    expect(screen.getByTestId("shell-memory")).toContainElement(
      screen.getByText("MEMORY"),
    );
  });
});
