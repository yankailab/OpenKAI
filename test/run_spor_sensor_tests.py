#!/usr/bin/env python3
"""Run SporSensor configuration and profile conversion regressions without hardware.

Usage: python3 test/run_spor_sensor_tests.py [build-directory] [--fixtures-dir path]
Build OpenKAI with WITH_SENSOR=ON, USE_SPOR_SENSOR_SDK=ON, and
CMAKE_EXPORT_COMPILE_COMMANDS=ON first. Captured profiles are read separately from
the SDK source checkout's tests/data directory; no connection to the sensor or
loopback sockets is made.
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
    parser.add_argument(
        "--fixtures-dir",
        type=pathlib.Path,
        default=repo.parent / "SporSensorSDK" / "tests" / "data",
        help="captured profile directory (default: ../SporSensorSDK/tests/data)",
    )
    options = parser.parse_args()
    build = options.build_directory.resolve()
    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next(
        (item for item in entries if item["file"].endswith("/src/Sensor/LineScanner/_SporSensor.cpp")),
        None,
    )
    if entry is None:
        parser.error("Build OpenKAI with WITH_SENSOR=ON and USE_SPOR_SENSOR_SDK=ON first")
    fixtures = options.fixtures_dir.resolve()
    for name in ("profile-brightness-v2.6.0.2.bin", "profile-invalid-v2.6.0.2.bin"):
        if not (fixtures / name).is_file():
            parser.error(f"SDK regression fixture is missing: {fixtures / name}")

    # Compiling the test should not require access to a global ccache directory.
    environment = os.environ.copy()
    environment["CCACHE_DISABLE"] = "1"
    with tempfile.TemporaryDirectory(prefix="openkai-spor-sensor-tests-") as temp:
        obj = str(pathlib.Path(temp) / "tests.o")
        command = list(entry["arguments"]) if "arguments" in entry else shlex.split(entry["command"])
        command[command.index("-o") + 1] = obj
        command[command.index("-c") + 1] = str(repo / "test/Sensor/LineScanner/SporSensor.cpp")
        if "-MF" in command:
            command[command.index("-MF") + 1] = obj + ".d"
        subprocess.run(command + ["-UNDEBUG"], cwd=entry["directory"], env=environment, check=True)
        command = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        command[next(i for i, arg in enumerate(command) if arg.endswith("/src/main.cpp.o"))] = obj
        executable = str(pathlib.Path(temp) / "spor-sensor-tests")
        command[command.index("-o") + 1] = executable
        subprocess.run(command, cwd=build, env=environment, check=True)
        subprocess.run([executable, str(fixtures)], cwd=repo, env=environment, check=True, timeout=30)


if __name__ == "__main__":
    main()
