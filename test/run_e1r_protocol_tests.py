#!/usr/bin/env python3
"""Compile and run E1R packet parser tests; no OpenKAI build or sensor needed.

Usage: python3 test/run_e1r_protocol_tests.py [--sanitize]
Only a C++11 compiler and the standard library are required. Set CXX to choose
the compiler. --sanitize enables AddressSanitizer and UndefinedBehaviorSanitizer.
"""

import argparse
import os
import pathlib
import shlex
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--sanitize", action="store_true")
    options = parser.parse_args()
    repo = pathlib.Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix="openkai-e1r-protocol-") as temp:
        executable = str(pathlib.Path(temp) / "e1r-protocol-test")
        command = shlex.split(os.environ.get("CXX", "c++"))
        command += ["-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic", "-UNDEBUG"]
        if options.sanitize:
            command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        command += [str(repo / "test/Sensor/E1RProtocol.cpp"), "-o", executable]
        subprocess.run(command, cwd=repo, check=True)
        subprocess.run([executable], cwd=repo, check=True, timeout=30)


if __name__ == "__main__":
    main()
