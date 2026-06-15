// web/src/components/FlagsBadges.test.tsx
import { render, screen } from "@testing-library/react";
import { describe, it, expect } from "vitest";
import { FlagsBadges } from "./FlagsBadges";
import type { Flags } from "../wasm/types";

const allClear: Flags = { n: false, v: false, u: false, b: false, d: false, i: false, z: false, c: false };

describe("FlagsBadges", () => {
  it("renders all eight flag badges in N V U B D I Z C order", () => {
    render(<FlagsBadges flags={allClear} />);
    const badges = screen.getAllByTestId(/^flag-/);
    expect(badges).toHaveLength(8);
    expect(badges.map((b) => b.getAttribute("data-testid"))).toEqual([
      "flag-n", "flag-v", "flag-u", "flag-b", "flag-d", "flag-i", "flag-z", "flag-c",
    ]);
  });

  it("shows 0 and unset styling when a flag is clear", () => {
    render(<FlagsBadges flags={allClear} />);
    const z = screen.getByTestId("flag-z");
    expect(z).toHaveTextContent("Z");
    expect(z).toHaveTextContent("0");
    expect(z.getAttribute("data-set")).toBe("false");
  });

  it("shows 1 and set styling when a flag is set", () => {
    render(<FlagsBadges flags={{ ...allClear, n: true, c: true }} />);
    const n = screen.getByTestId("flag-n");
    expect(n).toHaveTextContent("1");
    expect(n.getAttribute("data-set")).toBe("true");
    expect(screen.getByTestId("flag-c").getAttribute("data-set")).toBe("true");
    expect(screen.getByTestId("flag-v").getAttribute("data-set")).toBe("false");
  });
});
