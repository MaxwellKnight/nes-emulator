import { describe, it, expect } from "vitest";
import { render, screen } from "@testing-library/react";
import { ScreenPanel } from "./ScreenPanel";

describe("ScreenPanel", () => {
  it("renders the placeholder caption", () => {
    render(<ScreenPanel />);
    expect(
      screen.getByText("video output arrives with the PPU"),
    ).toBeInTheDocument();
  });

  it("renders the resolution label", () => {
    render(<ScreenPanel />);
    expect(screen.getByText("256 x 240")).toBeInTheDocument();
  });

  it("renders a well that holds the 256:240 aspect placeholder", () => {
    render(<ScreenPanel />);
    const well = screen.getByTestId("screen-well");
    expect(well).toBeInTheDocument();
    expect(well).toHaveStyle({ aspectRatio: "256 / 240" });
  });
});
