import { describe, it, expect, vi } from "vitest";
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { Button } from "./Button";

describe("Button", () => {
  it("renders its children", () => {
    render(<Button>Run</Button>);
    expect(screen.getByRole("button", { name: "Run" })).toBeInTheDocument();
  });

  it("applies a variant class for each variant", () => {
    const { rerender } = render(<Button variant="primary">P</Button>);
    expect(screen.getByRole("button")).toHaveAttribute("data-variant", "primary");
    rerender(<Button variant="danger">D</Button>);
    expect(screen.getByRole("button")).toHaveAttribute("data-variant", "danger");
    rerender(<Button variant="ghost">G</Button>);
    expect(screen.getByRole("button")).toHaveAttribute("data-variant", "ghost");
  });

  it("defaults to the secondary variant", () => {
    render(<Button>S</Button>);
    expect(screen.getByRole("button")).toHaveAttribute("data-variant", "secondary");
  });

  it("can be disabled and does not fire onClick when disabled", async () => {
    const onClick = vi.fn();
    render(
      <Button disabled onClick={onClick}>
        X
      </Button>,
    );
    const btn = screen.getByRole("button");
    expect(btn).toBeDisabled();
    await userEvent.click(btn);
    expect(onClick).not.toHaveBeenCalled();
  });

  it("fires onClick when enabled", async () => {
    const onClick = vi.fn();
    render(<Button onClick={onClick}>Go</Button>);
    await userEvent.click(screen.getByRole("button"));
    expect(onClick).toHaveBeenCalledTimes(1);
  });
});
