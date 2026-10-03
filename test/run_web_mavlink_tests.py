#!/usr/bin/env python3
"""Run MAVLink viewer tests against a WITH_UNIVERSE=ON CMake build.

Usage: python3 test/run_web_mavlink_tests.py build [--no-network]
The network tests use loopback sockets and verify both WebSocket transport modes.
"""

import argparse
import json
import pathlib
import shlex
import subprocess
import sys
import tempfile

sys.dont_write_bytecode = True
from run_byte_packet_tests import link_command


def main():
    repo = pathlib.Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build_directory", type=pathlib.Path)
    parser.add_argument("--no-network", action="store_true")
    args = parser.parse_args()
    build = args.build_directory.resolve()
    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next(item for item in entries if item["file"].endswith("/WebMavlinkProtocol.cpp"))
    with tempfile.TemporaryDirectory(prefix="openkai-web-mavlink-tests-") as temp:
        obj = str(pathlib.Path(temp) / "tests.o")
        command = shlex.split(entry["command"])
        command[command.index("-o") + 1] = obj
        command[command.index("-c") + 1] = str(repo / "test/UI/WebMavlinkStream.cpp")
        if "-MF" in command:
            command[command.index("-MF") + 1] = obj + ".d"
        subprocess.run(command + ["-UNDEBUG"], cwd=entry["directory"], check=True)
        command = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        command[next(i for i, arg in enumerate(command) if arg.endswith("/src/main.cpp.o"))] = obj
        executable = str(pathlib.Path(temp) / "web-mavlink-tests")
        command[command.index("-o") + 1] = executable
        subprocess.run(command, cwd=build, check=True)
        subprocess.run([executable] + (["--no-network"] if args.no_network else []), cwd=repo, check=True, timeout=30)


if __name__ == "__main__":
    main()
