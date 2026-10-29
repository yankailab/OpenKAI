#!/usr/bin/env python3
"""Run rigid-transform and composed point-cloud regressions with a WITH_UNIVERSE build."""
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
                 if item["file"].endswith("/src/Universe/_TransformRigid.cpp"))
    sources = ["test/Universe/TransformRigid.cpp"]
    flags = shlex.split(entry["command"])
    if "-DWITH_VISION" in flags and "-DUSE_OPENCV" in flags:
        sources.append("test/Sensor/RGBDTransform.cpp")
    with tempfile.TemporaryDirectory(prefix="openkai-transform-test-") as temp:
        objects = []
        for index, source in enumerate(sources):
            obj = str(pathlib.Path(temp) / f"transform-tests-{index}.o")
            args = flags.copy()
            args[args.index("-o") + 1] = obj
            args[args.index("-c") + 1] = str(repo / source)
            if "-MF" in args:
                args[args.index("-MF") + 1] = obj + ".d"
            args.extend(["-UNDEBUG"])
            subprocess.run(args, cwd=entry["directory"], check=True)
            objects.append(obj)
        args = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        main_index = next(i for i, arg in enumerate(args) if arg.endswith("/src/main.cpp.o"))
        args[main_index:main_index + 1] = objects
        executable = str(pathlib.Path(temp) / "transform-tests")
        args[args.index("-o") + 1] = executable
        subprocess.run(args, cwd=build, check=True)
        subprocess.run([executable], cwd=repo, check=True, timeout=30)


if __name__ == "__main__":
    main()
