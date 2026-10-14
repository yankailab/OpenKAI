#!/usr/bin/env python3
"""Exercise distance drivers with synthetic serial and Modbus readings.

Usage: python3 test/run_distance_sensor_tests.py [build-directory]
Build OpenKAI with WITH_SENSOR=ON and compile_commands.json enabled first.
"""

import json
import pathlib
import shlex
import subprocess
import sys
import tempfile

from run_byte_packet_tests import link_command


def main():
    repo = pathlib.Path(__file__).resolve().parents[1]
    build = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else repo / "build"
    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next(item for item in entries if item["file"].endswith("/src/Sensor/Distance/_TOFsense.cpp"))
    with tempfile.TemporaryDirectory(prefix="openkai-distance-") as temp:
        obj = str(pathlib.Path(temp) / "distance-tests.o")
        args = shlex.split(entry["command"])
        args[args.index("-o") + 1] = obj
        args[args.index("-c") + 1] = str(repo / "test/Sensor/DistanceSensors.cpp")
        if "-MF" in args:
            args[args.index("-MF") + 1] = obj + ".d"
        args.extend(["-DOPENKAI_DISTANCE_SENSOR_TEST", "-UNDEBUG"])
        subprocess.run(args, cwd=entry["directory"], check=True)
        args = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        index = next(i for i, arg in enumerate(args) if arg.endswith("/src/main.cpp.o"))
        args[index] = obj
        executable = str(pathlib.Path(temp) / "distance-tests")
        args[args.index("-o") + 1] = executable
        for name in ["modbus_read_input_registers", "modbus_send_raw_request", "modbus_receive_confirmation"]:
            args.append("-Wl,--wrap=" + name)
        subprocess.run(args, cwd=build, check=True)
        subprocess.run([executable], cwd=repo, check=True, timeout=30)


if __name__ == "__main__":
    main()
