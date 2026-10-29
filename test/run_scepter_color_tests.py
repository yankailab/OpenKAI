#!/usr/bin/env python3
"""Test Scepter color, publication during blocked capture, and command admission.

Usage: python3 test/run_scepter_color_tests.py [build-directory]
Build OpenKAI with USE_SCEPTER_SDK=ON and compile_commands.json enabled first.
The mocked SDK requires no camera or network access.
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
    parser.add_argument("build_directory", nargs="?", type=pathlib.Path, default=repo / "build")
    build = parser.parse_args().build_directory.resolve()
    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next((item for item in entries if item["file"].endswith("/src/Vision/RGBD/_Scepter.cpp")), None)
    if entry is None:
        parser.error("Build OpenKAI with USE_SCEPTER_SDK=ON first")
    with tempfile.TemporaryDirectory(prefix="openkai-scepter-color-") as temp:
        objects = []
        # Compile the driver directly so --wrap also intercepts SDK calls when
        # the existing build normally places the driver in libOpenKAI.so.
        for index, source in enumerate(["test/Sensor/ScepterColor.cpp", "src/Vision/RGBD/_Scepter.cpp"]):
            obj = str(pathlib.Path(temp) / f"scepter-{index}.o")
            command = list(entry["arguments"]) if "arguments" in entry else shlex.split(entry["command"])
            command[command.index("-o") + 1] = obj
            command[command.index("-c") + 1] = str(repo / source)
            if "-MF" in command:
                command[command.index("-MF") + 1] = obj + ".d"
            subprocess.run(command + ["-UNDEBUG"], cwd=entry["directory"], check=True)
            objects.append(obj)
        command = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")
                   and not arg.endswith("/src/Vision/RGBD/_Scepter.cpp.o")]
        index = next(i for i, arg in enumerate(command) if arg.endswith("/src/main.cpp.o"))
        command[index:index + 1] = objects
        executable = str(pathlib.Path(temp) / "scepter-color-tests")
        command[command.index("-o") + 1] = executable
        for name in ["scGetFrameReady", "scGetFrame", "scConvertDepthFrameToPointCloudVector",
                     "scStopStream", "scCloseDevice"]:
            command.append("-Wl,--wrap=" + name)
        subprocess.run(command, cwd=build, check=True)
        subprocess.run([executable], cwd=repo, check=True, timeout=30)


if __name__ == "__main__":
    main()
