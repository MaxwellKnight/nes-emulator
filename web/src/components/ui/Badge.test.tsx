import { describe, it, expect } from "vitest";
import { render, screen } from "@testing-library/react";
import { Badge } from "./Badge";

describe("Badge", () => {
  it("renders its children", () => {
    render(<Badge>N</Badge>);
    expect(screen.getByText("N")).toBeInTheDocument();
  });

  it("marks itself active via data-active when active", () => {
    render(<Badge active>Z</Badge>);
    expect(screen.getByText("Z")).toHaveAttribute("data-active", "true");
  });

  it("is inactive by default", () => {
    render(<Badge>C</Badge>);
    expect(screen.getByText("C")).toHaveAttribute("data-active", "false");
  });
});
