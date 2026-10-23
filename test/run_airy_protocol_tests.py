#!/usr/bin/env python3
"""Run native Airy protocol regressions without an OpenKAI build or SDK.

Usage: python3 test/run_airy_protocol_tests.py [--sanitize] [--capture-dir path]
Optional captures contain 7788.bin and packets.bin (<QHH timestamp,port,length
records followed by UDP payloads). No network or sensor connection is used.
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
    parser.add_argument("--capture-dir", type=pathlib.Path)
    options = parser.parse_args()
    repo = pathlib.Path(__file__).resolve().parents[1]
    with tempfile.TemporaryDirectory(prefix="openkai-airy-protocol-") as temp:
        executable = str(pathlib.Path(temp) / "airy-protocol-test")
        command = shlex.split(os.environ.get("CXX", "c++"))
        command += ["-std=c++11", "-Wall", "-Wextra", "-Werror", "-pedantic", "-UNDEBUG"]
        if options.sanitize:
            command += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
        command += [str(repo / "test/Sensor/AiryProtocol.cpp"), "-o", executable]
        subprocess.run(command, cwd=repo, check=True)
        arguments = [executable]
        if options.capture_dir:
            arguments.append(str(options.capture_dir.resolve()))
        subprocess.run(arguments, cwd=repo, check=True, timeout=60)


if __name__ == "__main__":
    main()
