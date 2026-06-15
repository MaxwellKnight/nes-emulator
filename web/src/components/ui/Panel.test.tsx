import { describe, it, expect } from "vitest";
import { render, screen } from "@testing-library/react";
import { Panel } from "./Panel";

describe("Panel", () => {
  it("renders the title", () => {
    render(<Panel title="Registers">body</Panel>);
    expect(screen.getByText("Registers")).toBeInTheDocument();
  });

  it("renders its children", () => {
    render(
      <Panel title="Stats">
        <span>inner content</span>
      </Panel>,
    );
    expect(screen.getByText("inner content")).toBeInTheDocument();
  });

  it("exposes the title via a heading element", () => {
    render(<Panel title="Memory">x</Panel>);
    expect(screen.getByRole("heading", { name: "Memory" })).toBeInTheDocument();
  });
});
