// web/src/wasm/opcodes.ts

/**
 * Parse a textarea of hex opcodes into a byte array.
 *  - Strips ";" line comments.
 *  - Splits on whitespace and commas.
 *  - Each token must be 1-2 hex digits, else throws.
 */
export function parseOpcodes(text: string): number[] {
  const withoutComments = text
    .split("\n")
    .map((line) => {
      const commentIndex = line.indexOf(";");
      return commentIndex === -1 ? line : line.slice(0, commentIndex);
    })
    .join("\n");

  const tokens = withoutComments
    .split(/[\s,]+/)
    .map((token) => token.trim())
    .filter((token) => token.length > 0);

  const bytes: number[] = [];
  for (const token of tokens) {
    if (!/^[0-9A-Fa-f]{1,2}$/.test(token)) {
      throw new Error(`Invalid opcode format: ${token}`);
    }
    bytes.push(parseInt(token, 16));
  }

  return bytes;
}
