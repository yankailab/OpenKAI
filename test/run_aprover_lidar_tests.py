#!/usr/bin/env python3
"""Run the lawnmower control regressions against an AProver_LiDAR CMake build."""
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
    build = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else repo / "build"
    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next(item for item in entries if item["file"].endswith("/samples/AProver_LiDAR/_AProver_LiDAR.cpp"))
    with tempfile.TemporaryDirectory(prefix="openkai-aprover-lidar-tests-") as temp:
        objects = []
        for index, source in enumerate(["test/Autopilot/AProverLiDAR.cpp", "samples/AProver_LiDAR/_AProver_LiDAR.cpp"]):
            obj = str(pathlib.Path(temp) / f"test-{index}.o")
            args = shlex.split(entry["command"])
            args[args.index("-o") + 1] = obj
            args[args.index("-c") + 1] = str(repo / source)
            if "-MF" in args:
                args[args.index("-MF") + 1] = obj + ".d"
            subprocess.run(args + ["-UNDEBUG"], cwd=entry["directory"], check=True)
            objects.append(obj)
        args = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        main_index = next(i for i, arg in enumerate(args) if arg.endswith("/src/main.cpp.o"))
        args[main_index:main_index + 1] = objects
        executable = str(pathlib.Path(temp) / "aprover-lidar-tests")
        args[args.index("-o") + 1] = executable
        subprocess.run(args, cwd=build, check=True)
        subprocess.run([executable, str(pathlib.Path(temp) / "sample.json")], cwd=repo, check=True, timeout=30)


if __name__ == "__main__":
    main()
