#!/usr/bin/env python3
"""Run Airy frame, filtering, timestamp and configuration tests without hardware.

Usage: python3 test/run_airy_tests.py [build-directory]
Build OpenKAI with WITH_SENSOR=ON and CMAKE_EXPORT_COMPILE_COMMANDS=ON first.
The independent wire parser tests are in run_airy_protocol_tests.py.
"""

import argparse
import json
import os
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
    parser.add_argument("build_directory", nargs="?", type=pathlib.Path, default=repo / "build")
    options = parser.parse_args()
    build = options.build_directory.resolve()
    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next((item for item in entries
                  if item["file"].endswith("/src/Sensor/LiDAR/RoboSense/Airy/_Airy.cpp")), None)
    if entry is None:
        parser.error("Build OpenKAI with WITH_SENSOR=ON first")
    environment = os.environ.copy()
    environment["CCACHE_DISABLE"] = "1"
    with tempfile.TemporaryDirectory(prefix="openkai-airy-tests-") as temp:
        obj = str(pathlib.Path(temp) / "tests.o")
        command = list(entry["arguments"]) if "arguments" in entry else shlex.split(entry["command"])
        command[command.index("-o") + 1] = obj
        command[command.index("-c") + 1] = str(repo / "test/Sensor/Airy.cpp")
        if "-MF" in command:
            command[command.index("-MF") + 1] = obj + ".d"
        command += ["-DOPENKAI_AIRY_TEST", "-UNDEBUG"]
        subprocess.run(command, cwd=entry["directory"], env=environment, check=True)
        command = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        command[next(i for i, arg in enumerate(command) if arg.endswith("/src/main.cpp.o"))] = obj
        executable = str(pathlib.Path(temp) / "airy-tests")
        command[command.index("-o") + 1] = executable
        subprocess.run(command, cwd=build, env=environment, check=True)
        subprocess.run([executable], cwd=repo, env=environment, check=True, timeout=30)


if __name__ == "__main__":
    main()
