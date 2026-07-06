// Catalog of freely-licensed homebrew NES games bundled with the app.
//
// The ROM files live in `public/roms/` and are copied verbatim into the build
// output by Vite, so they are fetched at runtime from `/roms/<file>`. Licensing
// details and sources are recorded in `public/roms/LICENSES.md`.

export interface Game {
  /** Stable identifier, also used as the React key. */
  id: string;
  /** Display name. */
  title: string;
  /** Author, credited in the Help dialog (required for the CC-BY title). */
  author: string;
  /** Short license label, e.g. "CC0" or "CC BY 4.0". */
  license: string;
  /** Canonical license URL. */
  licenseUrl: string;
  /** ROM filename served from `/roms/`. */
  file: string;
}

export const GAMES: readonly Game[] = [
  {
    id: "lawn-mower",
    title: "Lawn Mower",
    author: "Shiru",
    license: "CC0",
    licenseUrl: "https://creativecommons.org/publicdomain/zero/1.0/",
    file: "lawn-mower.nes",
  },
  {
    id: "chase",
    title: "Chase",
    author: "Shiru",
    license: "CC0",
    licenseUrl: "https://creativecommons.org/publicdomain/zero/1.0/",
    file: "chase.nes",
  },
  {
    id: "lan-master",
    title: "Lan Master",
    author: "Shiru",
    license: "CC0",
    licenseUrl: "https://creativecommons.org/publicdomain/zero/1.0/",
    file: "lan-master.nes",
  },
  {
    id: "zooming-secretary",
    title: "Zooming Secretary",
    author: "Shiru & PinWizz",
    license: "CC BY 4.0",
    licenseUrl: "https://creativecommons.org/licenses/by/4.0/",
    file: "zooming-secretary.nes",
  },
];

/** The game loaded automatically when the emulator becomes ready. */
export const DEFAULT_GAME: Game = GAMES[0];

/** Public path a game's ROM is served from. */
export function romUrl(game: Game): string {
  return `${import.meta.env.BASE_URL}roms/${game.file}`;
}
