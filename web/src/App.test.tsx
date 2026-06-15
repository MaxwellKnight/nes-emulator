import { render, screen } from "@testing-library/react";
import App from "./App";

describe("App", () => {
  it("renders the NES Studio heading", () => {
    render(<App />);
    expect(
      screen.getByRole("heading", { name: "NES Studio" }),
    ).toBeInTheDocument();
  });
});
