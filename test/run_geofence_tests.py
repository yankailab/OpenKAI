#!/usr/bin/env python3
"""Run synchronous fence prediction and bounce regressions with a WITH_NAVIGATION build."""
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
    entry = next(item for item in json.loads((build / "compile_commands.json").read_text())
                 if item["file"].endswith("/src/Navigation/_GeoFence.cpp"))
    with tempfile.TemporaryDirectory(prefix="openkai-geofence-test-") as temp:
        objects = []
        for index, source in enumerate(["test/Navigation/GeoFence.cpp", "src/Navigation/_GeoFence.cpp"]):
            obj = str(pathlib.Path(temp) / f"geofence-{index}.o")
            args = shlex.split(entry["command"])
            args[args.index("-o") + 1] = obj
            args[args.index("-c") + 1] = str(repo / source)
            if "-MF" in args:
                args[args.index("-MF") + 1] = obj + ".d"
            args.extend(["-UNDEBUG"])
            subprocess.run(args, cwd=entry["directory"], check=True)
            objects.append(obj)
        args = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        args[next(i for i, arg in enumerate(args) if arg.endswith("/src/main.cpp.o"))] = objects[0]
        fence_index = next((i for i, arg in enumerate(args)
                            if arg.endswith("/src/Navigation/_GeoFence.cpp.o")), None)
        if fence_index is None:  # shared-library builds: interpose the freshly compiled implementation
            args.insert(args.index(objects[0]) + 1, objects[1])
        else:
            args[fence_index] = objects[1]
        executable = str(pathlib.Path(temp) / "geofence-tests")
        args[args.index("-o") + 1] = executable
        subprocess.run(args, cwd=build, check=True)
        subprocess.run([executable], cwd=repo, check=True, timeout=30)


if __name__ == "__main__":
    main()
