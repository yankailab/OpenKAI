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
    with tempfile.TemporaryDirectory(prefix="openkai-transform-test-") as temp:
        obj = str(pathlib.Path(temp) / "transform-tests.o")
        args = shlex.split(entry["command"])
        args[args.index("-o") + 1] = obj
        args[args.index("-c") + 1] = str(repo / "test/Universe/TransformRigid.cpp")
        if "-MF" in args:
            args[args.index("-MF") + 1] = obj + ".d"
        args.extend(["-UNDEBUG"])
        subprocess.run(args, cwd=entry["directory"], check=True)
        args = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        args[next(i for i, arg in enumerate(args) if arg.endswith("/src/main.cpp.o"))] = obj
        executable = str(pathlib.Path(temp) / "transform-tests")
        args[args.index("-o") + 1] = executable
        subprocess.run(args, cwd=build, check=True)
        subprocess.run([executable], cwd=repo, check=True, timeout=30)


if __name__ == "__main__":
    main()
