#!/usr/bin/env python3
"""Run synchronous fence prediction and bounce regressions with a WITH_NAVIGATION build."""
import json
import pathlib
import shlex
import subprocess
import sys
import tempfile
from build_helpers import link_command


def main():
    repo = pathlib.Path(__file__).resolve().parents[1]
    build = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else repo / "build"
    entry = next(item for item in json.loads((build / "compile_commands.json").read_text())
                 if item["file"].endswith("/src/Navigation/_GeoFence.cpp"))
    with tempfile.TemporaryDirectory(prefix="openkai-geofence-test-") as temp:
        obj = str(pathlib.Path(temp) / "geofence.o")
        args = shlex.split(entry["command"])
        args[args.index("-o") + 1] = obj
        args[args.index("-c") + 1] = str(repo / "test/Navigation/GeoFence.cpp")
        if "-MF" in args:
            args[args.index("-MF") + 1] = obj + ".d"
        subprocess.run(args + ["-UNDEBUG"], cwd=entry["directory"], check=True)
        args = link_command(build)
        args[next(i for i, arg in enumerate(args) if arg.endswith("/src/main.cpp.o"))] = obj
        executable = str(pathlib.Path(temp) / "geofence-tests")
        args[args.index("-o") + 1] = executable
        subprocess.run(args, cwd=build, check=True)
        subprocess.run([executable], cwd=repo, check=True, timeout=30)


if __name__ == "__main__":
    main()
