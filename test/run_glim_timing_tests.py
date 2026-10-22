#!/usr/bin/env python3
"""Exercise real GLIM preprocessing and IMU gating without hardware or sockets.

Usage: python3 test/run_glim_timing_tests.py [build-directory]
Requires an existing USE_GLIM=ON build with compile_commands.json.
"""

import argparse
import json
import os
import pathlib
import shlex
import shutil
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
    entry = next((item for item in entries if item["file"].endswith("/src/SLAM/_GLIM.cpp")), None)
    if entry is None:
        parser.error("Build OpenKAI with WITH_SLAM=ON and USE_GLIM=ON first")
    environment = os.environ.copy()
    environment["CCACHE_DISABLE"] = "1"
    with tempfile.TemporaryDirectory(prefix="openkai-glim-timing-") as temp:
        temp = pathlib.Path(temp)
        objects = []
        for index, source in enumerate(("test/SLAM/GLIMTiming.cpp", "src/SLAM/_GLIM.cpp", "src/DataObject/IMUstream.cpp")):
            obj = str(temp / f"timing-{index}.o")
            command = list(entry["arguments"]) if "arguments" in entry else shlex.split(entry["command"])
            command[command.index("-o") + 1] = obj
            command[command.index("-c") + 1] = str(repo / source)
            if "-MF" in command:
                command[command.index("-MF") + 1] = obj + ".d"
            subprocess.run(command + ["-UNDEBUG"], cwd=entry["directory"], env=environment, check=True)
            objects.append(obj)
        command = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        for suffix, obj in zip(("/src/main.cpp.o", "/src/SLAM/_GLIM.cpp.o", "/src/DataObject/IMUstream.cpp.o"), objects):
            command[next(i for i, arg in enumerate(command) if arg.endswith(suffix))] = obj
        executable = str(temp / "glim-timing-tests")
        command[command.index("-o") + 1] = executable
        subprocess.run(command, cwd=build, env=environment, check=True)
        for mode in ("rolling", "global"):
            profile = temp / mode
            shutil.copytree(repo / "jsonCfg/glim_livox2", profile)
            sensors = json.loads((profile / "config_sensors.json").read_text())
            sensors["sensors"]["global_shutter_lidar"] = mode == "global"
            (profile / "config_sensors.json").write_text(json.dumps(sensors))
            subprocess.run([executable, str(profile), mode], cwd=repo, env=environment, check=True, timeout=30)


if __name__ == "__main__":
    main()
