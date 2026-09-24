#!/usr/bin/env python3
"""Check Ctrl+C and startup-failure terminal cleanup using a real pseudo-terminal."""

import fcntl
import json
import os
from pathlib import Path
import pty
import select
import subprocess
import sys
import tempfile
import termios
import time


ENTER_SCREEN = b"\x1b[?1049h"
LEAVE_SCREEN = b"\x1b[?1049l"
CONSOLE_NAME = "__console_shutdown_ready__"


def controlling_terminal():
    os.setsid()
    fcntl.ioctl(0, termios.TIOCSCTTY, 0)


def check(binary, name, modules, interrupt=False):
    with tempfile.TemporaryDirectory(prefix="openkai-console-") as directory:
        config = Path(directory) / "config.json"
        config.write_text(json.dumps({
            "APP": {"class": "ModuleMgr", "bStdErr": False},
            **modules,
        }))
        master, slave = pty.openpty()
        original = termios.tcgetattr(slave)
        output = bytearray()
        process = subprocess.Popen(
            [binary, str(config)], stdin=slave, stdout=slave, stderr=slave,
            env=dict(os.environ, TERM="xterm-256color"),
            preexec_fn=controlling_terminal,
        )
        try:
            deadline = time.monotonic() + 5
            interrupted = False
            while process.poll() is None:
                assert time.monotonic() < deadline, f"{name}: shutdown timed out"
                if select.select([master], [], [], 0.05)[0]:
                    output.extend(os.read(master, 65536))
                if interrupt and not interrupted and CONSOLE_NAME.encode() in output:
                    # Send the terminal's interrupt character, just like Ctrl+C.
                    os.write(master, b"\x03")
                    interrupted = True
            while select.select([master], [], [], 0)[0]:
                output.extend(os.read(master, 65536))

            assert process.returncode == 0, f"{name}: exit {process.returncode}: {output!r}"
            assert termios.tcgetattr(slave) == original, f"{name}: terminal modes changed"
            if interrupt:
                assert interrupted, f"{name}: console never rendered"
                assert ENTER_SCREEN in output, f"{name}: curses never entered its screen"
                assert output.rfind(LEAVE_SCREEN) > output.rfind(ENTER_SCREEN), \
                    f"{name}: alternate screen was not restored"
                assert output.find(b"SIGINT") > output.rfind(LEAVE_SCREEN), \
                    f"{name}: shutdown message printed before terminal restoration"
            print(f"PASS: {name}")
        finally:
            if process.poll() is None:
                process.kill()
                process.wait()
            os.close(master)
            os.close(slave)


def main():
    binary = str(Path(sys.argv[1] if len(sys.argv) > 1 else "build/OpenKAI").resolve())
    for fps in (1, 1000):
        check(binary, f"Ctrl+C at {fps} FPS", {
            CONSOLE_NAME: {
                "class": "_Console", "thread": {"FPS": fps}, "vBASE": [CONSOLE_NAME],
            },
        }, interrupt=True)
    check(binary, "failure before curses initialization", {
        "invalid_console": {"class": "_Console"},
    })
    check(binary, "failure after curses initialization", {
        CONSOLE_NAME: {"class": "_Console", "thread": {"FPS": 30}},
        "zz_invalid_console": {"class": "_Console"},
    })


if __name__ == "__main__":
    main()
