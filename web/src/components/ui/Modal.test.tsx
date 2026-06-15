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
});
