// web/src/components/HelpModal.test.tsx
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { describe, it, expect, vi } from "vitest";
import { HelpModal } from "./HelpModal";

describe("HelpModal", () => {
  it("renders all five help sections when open", () => {
    render(<HelpModal open onClose={() => {}} />);
    for (const heading of [
      "Getting Started",
      "Loading Code",
      "Controls",
      "Breakpoints",
      "Memory Navigation",
    ]) {
      expect(screen.getByRole("heading", { name: heading })).toBeInTheDocument();
    }
  });

  it("renders nothing when closed", () => {
    render(<HelpModal open={false} onClose={() => {}} />);
    expect(screen.queryByRole("heading", { name: "Getting Started" })).toBeNull();
  });

  it("invokes onClose from the close control", async () => {
    const user = userEvent.setup();
    const onClose = vi.fn();
    render(<HelpModal open onClose={onClose} />);
    await user.click(screen.getByTestId("help-close"));
    expect(onClose).toHaveBeenCalled();
  });
});
