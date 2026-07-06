"""Stream a live agent to the browser over Server-Sent Events (stdlib only).

The server runs an agent on its own deterministic core and streams one action byte
per frame. NES Studio re-simulates those actions on its WASM core, so the browser
shows exactly what the agent is doing, frame for frame, at tiny bandwidth. There is
no WebSocket dependency: SSE is plain HTTP and works with the built-in http.server.

    python -m nesenv.live "Super Mario Bros.nes" --agent scripted --port 8000

Then click "Spawn Agent" in NES Studio. Swap the policy in `agent_action` for a
trained network to watch it learn; the streaming protocol does not change.

Protocol (text/event-stream):
    event: rom    data: <base64 ROM>     (once, on connect)
    event: reset  data:                  (re-boot, at the start of a life cycle)
    event: step   data: <button mask>    (one per frame)
"""

from __future__ import annotations

import argparse
import base64
import random
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

from .core import A, B, RIGHT, START, Nes
from .smb import OPER_MODE, PLAYER_PAGE, PLAYER_X

# Module-level config set by main(); each connection reads it.
ROM = b""
AGENT = "scripted"


def gameplay_started(nes: Nes, frame: int) -> bool:
    return nes.peek(OPER_MODE) == 1 and frame > 250


# Reboot a life if the agent makes no forward progress for this many frames
# (covers both getting stuck on an obstacle and dying), so the stream never freezes.
STALL_RESET = 480  # ~8 seconds


def agent_action(nes: Nes, frame: int, state: dict) -> int:
    """Decide a button mask for this frame. This is a fixed heuristic, not a
    learner: replace it with a trained policy to watch an agent actually improve;
    everything downstream (the stream, the browser) stays the same."""
    if AGENT == "random":
        # mostly run right, sometimes jump, so a random run still looks alive
        return random.choice([RIGHT, RIGHT | A, RIGHT | B, RIGHT | A | B, 0, RIGHT])
    # scripted: run right (B), jump on a rhythm and when progress stalls. The jump
    # phase is offset per attempt (state["offset"]) so a reset retries differently.
    progress = nes.peek(PLAYER_PAGE) * 256 + nes.peek(PLAYER_X)
    state["pstall"] = state.get("pstall", 0) + 1 if progress <= state.get("pmax", 0) else 0
    state["pmax"] = max(state.get("pmax", 0), progress)
    mask = RIGHT | B  # hold B to run, so run-jumps clear bigger gaps
    if ((frame + state.get("offset", 0)) % 24) < 13 or state["pstall"] > 4:
        mask |= A
    return mask


def drive(send) -> None:
    """Run the agent forever, calling send(event, data) for each frame."""
    nes = Nes()
    nes.load(ROM)
    send("rom", base64.b64encode(ROM).decode())

    while True:
        send("reset", "")
        nes.reset()
        state: dict = {"offset": random.randint(0, 23)}
        max_progress = 0
        stall = 0
        started = False
        for frame in range(100_000):
            if frame < 80:
                mask = 0
            elif frame < 90:
                mask = START
            elif not gameplay_started(nes, frame):
                mask = 0
            else:
                started = True
                mask = agent_action(nes, frame, state)

            send("step", str(mask))
            nes.step(mask)
            time.sleep(1 / 60)  # pace to roughly real time

            if started:
                progress = nes.peek(PLAYER_PAGE) * 256 + nes.peek(PLAYER_X)
                if progress > max_progress:
                    max_progress, stall = progress, 0
                else:
                    stall += 1
                if stall > STALL_RESET:
                    break  # stuck or died: reboot and try again


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *args):  # quieter
        pass

    def do_GET(self):
        if self.path.rstrip("/") not in ("", "/stream"):
            self.send_error(404)
            return
        self.send_response(200)
        self.send_header("Content-Type", "text/event-stream")
        self.send_header("Cache-Control", "no-cache")
        self.send_header("Access-Control-Allow-Origin", "*")
        self.end_headers()

        def send(event: str, data: str) -> None:
            self.wfile.write(f"event: {event}\ndata: {data}\n\n".encode())
            self.wfile.flush()

        try:
            drive(send)
        except (BrokenPipeError, ConnectionResetError):
            pass  # viewer closed the tab


def main() -> int:
    global ROM, AGENT
    p = argparse.ArgumentParser(description="Stream a live NES agent over SSE.")
    p.add_argument("rom")
    p.add_argument("--agent", choices=["scripted", "random"], default="scripted")
    p.add_argument("--port", type=int, default=8000)
    args = p.parse_args()

    ROM = open(args.rom, "rb").read()
    AGENT = args.agent
    server = ThreadingHTTPServer(("127.0.0.1", args.port), Handler)
    print(f"live agent ({args.agent}) streaming on http://127.0.0.1:{args.port}/stream")
    print("open NES Studio and click 'Spawn Agent'. Ctrl-C to stop.")
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        pass
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
