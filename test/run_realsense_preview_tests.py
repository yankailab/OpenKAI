#!/usr/bin/env python3
"""Test the RealSense preview switch using SDK software motion frames, without a camera.

Usage: python3 test/run_realsense_preview_tests.py [build-directory]
Build OpenKAI with USE_REALSENSE=ON and compile_commands.json enabled first.
The SDK context needs udev-monitor access even though no physical camera opens.
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
    entry = next(item for item in entries if item["file"].endswith("/src/Vision/RGBD/_RealSense.cpp"))
    with tempfile.TemporaryDirectory(prefix="openkai-realsense-preview-test-") as temp:
        obj = str(pathlib.Path(temp) / "tests.o")
        args = list(entry["arguments"]) if "arguments" in entry else shlex.split(entry["command"])
        args[args.index("-o") + 1] = obj
        args[args.index("-c") + 1] = str(repo / "test/Sensor/RealSensePreview.cpp")
        if "-MF" in args:
            args[args.index("-MF") + 1] = obj + ".d"
        args += ["-DOPENKAI_REALSENSE_PREVIEW_TEST", "-UNDEBUG"]
        subprocess.run(args, cwd=entry["directory"], check=True)
        args = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        args[next(i for i, arg in enumerate(args) if arg.endswith("/src/main.cpp.o"))] = obj
        executable = str(pathlib.Path(temp) / "tests")
        args[args.index("-o") + 1] = executable
        subprocess.run(args, cwd=build, check=True)
        subprocess.run([executable], cwd=repo, check=True, timeout=30)


if __name__ == "__main__":
    main()
