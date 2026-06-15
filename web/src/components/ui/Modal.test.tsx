import { describe, it, expect, vi } from "vitest";
import { render, screen } from "@testing-library/react";
import userEvent from "@testing-library/user-event";
import { Modal } from "./Modal";

describe("Modal", () => {
  it("renders nothing when closed", () => {
    const { container } = render(
      <Modal open={false} onClose={() => {}} title="Edit">
        body
      </Modal>,
    );
    expect(container).toBeEmptyDOMElement();
    expect(screen.queryByText("Edit")).not.toBeInTheDocument();
  });

  it("renders title and children when open", () => {
    render(
      <Modal open onClose={() => {}} title="Edit Memory">
        <span>field</span>
      </Modal>,
    );
    expect(screen.getByText("Edit Memory")).toBeInTheDocument();
    expect(screen.getByText("field")).toBeInTheDocument();
  });

  it("calls onClose when the close control is clicked", async () => {
    const onClose = vi.fn();
    render(
      <Modal open onClose={onClose} title="Help">
        body
      </Modal>,
    );
    await userEvent.click(screen.getByRole("button", { name: /close/i }));
    expect(onClose).toHaveBeenCalledTimes(1);
  });

  it("calls onClose when the backdrop is clicked", async () => {
    const onClose = vi.fn();
    render(
      <Modal open onClose={onClose} title="Help">
        body
      </Modal>,
    );
    await userEvent.click(screen.getByTestId("modal-backdrop"));
    expect(onClose).toHaveBeenCalledTimes(1);
  });

  it("moves focus into the dialog when opened", () => {
    render(
      <Modal open onClose={() => {}} title="Edit Memory">
        <input data-testid="field" />
      </Modal>,
    );
    const dialog = screen.getByRole("dialog");
    expect(dialog.contains(document.activeElement)).toBe(true);
  });

  it("traps Tab focus within the dialog", async () => {
    render(
      <Modal open onClose={() => {}} title="Edit Memory">
        <input data-testid="field" />
      </Modal>,
    );
    const dialog = screen.getByRole("dialog");
    // Tabbing from the last focusable element wraps back into the dialog.
    const focusables = dialog.querySelectorAll<HTMLElement>(
      "a[href], button, input, select, textarea",
    );
    const last = focusables[focusables.length - 1];
    last.focus();
    await userEvent.tab();
    expect(dialog.contains(document.activeElement)).toBe(true);
  });
});
