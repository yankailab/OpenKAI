#!/usr/bin/env python3
"""Test _FTservo using the actual FTServo_Linux library and a pseudo-terminal.

Usage: python3 test/run_ftservo_tests.py [build-directory]
Requires USE_FTSERVO=ON, BUILD_EXECUTABLE=ON, CMAKE_EXPORT_COMPILE_COMMANDS=ON.
No servo hardware is used.
"""
import json
from pathlib import Path
import shlex
import subprocess
import sys
import tempfile

from run_byte_packet_tests import link_command


def main():
    repo = Path(__file__).resolve().parents[1]
    build = Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else repo / "build-ftservo"
    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next(item for item in entries if item["file"].endswith("/src/Actuator/_FTservo.cpp"))
    with tempfile.TemporaryDirectory(prefix="openkai-ftservo-tests-") as temp:
        obj = str(Path(temp) / "ftservo-tests.o")
        args = list(entry.get("arguments") or shlex.split(entry["command"]))
        args[args.index("-o") + 1] = obj
        args[args.index("-c") + 1] = str(repo / "test/Actuator/FTservo.cpp")
        if "-MF" in args:
            args[args.index("-MF") + 1] = obj + ".d"
        args.append("-UNDEBUG")
        subprocess.run(args, cwd=entry["directory"], check=True)
        args = [arg for arg in link_command(build) if not arg.startswith("-Wl,--dependency-file=")]
        index = next(i for i, arg in enumerate(args) if arg.endswith("/src/main.cpp.o"))
        args[index] = obj
        executable = str(Path(temp) / "ftservo-tests")
        args[args.index("-o") + 1] = executable
        subprocess.run(args, cwd=build, check=True)
        subprocess.run([executable, str(repo / "jsonCfg/FTservo.json")], cwd=repo, check=True, timeout=20)


if __name__ == "__main__":
    main()
