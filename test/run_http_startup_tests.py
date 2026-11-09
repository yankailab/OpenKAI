#!/usr/bin/env python3
"""Check HTTP module bind retries with a WITH_UNIVERSE/NET/UI, USE_MAVLINK build.

Usage: python3 test/run_http_startup_tests.py build
Requires loopback sockets; covers occupied ports, recovery, stop and restart.
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
    args = parser.parse_args()
    build = args.build_directory.resolve()
    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next(item for item in entries if item["file"].endswith("/_WebGeometry.cpp"))
    with tempfile.TemporaryDirectory(prefix="openkai-http-startup-tests-") as temp:
        obj = str(pathlib.Path(temp) / "tests.o")
        command = shlex.split(entry["command"])
        command[command.index("-o") + 1] = obj
        command[command.index("-c") + 1] = str(repo / "test/Net/HttpStartup.cpp")
        if "-MF" in command:
            command[command.index("-MF") + 1] = obj + ".d"
        subprocess.run(command + ["-UNDEBUG"], cwd=entry["directory"], check=True)
        extra_objects = []
        # The GLIM viewer transport itself has no dependency on the GLIM SDK.
        if not any(item["file"].endswith("/_WebGLIM.cpp") for item in entries):
            extra = str(pathlib.Path(temp) / "web-glim.o")
            command[command.index("-o") + 1] = extra
            command[command.index("-c") + 1] = str(repo / "src/UI/Viewer/Web/_WebGLIM.cpp")
            if "-MF" in command:
                command[command.index("-MF") + 1] = extra + ".d"
            subprocess.run(command, cwd=entry["directory"], check=True)
            extra_objects.append(extra)
        command = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        command[next(i for i, arg in enumerate(command) if arg.endswith("/src/main.cpp.o"))] = obj
        command[1:1] = extra_objects
        executable = str(pathlib.Path(temp) / "http-startup-tests")
        command[command.index("-o") + 1] = executable
        subprocess.run(command, cwd=build, check=True)
        subprocess.run([executable], cwd=repo, check=True, timeout=45)


if __name__ == "__main__":
    main()
