#!/usr/bin/env python3
"""Run packet storage, duplex transport, and parser tests against an existing CMake build.

Usage: python3 test/run_byte_packet_tests.py [build-directory] [--mavlink-only]
Build OpenKAI with WITH_IO=ON and WITH_PROTOCOL=ON before running this script.
WITH_ARDUPILOT=ON additionally exercises the MAVLink stream consumers.
"""

import argparse
import json
import pathlib
import shlex
import subprocess
import tempfile


def link_command(build):
    link_file = build / "CMakeFiles/OpenKAI.dir/link.txt"
    if link_file.exists():
        return shlex.split(link_file.read_text())

    # Ninja keeps the command in its build graph instead of link.txt.
    commands = subprocess.run(
        ["ninja", "-C", str(build), "-t", "commands", "OpenKAI"],
        check=True, capture_output=True, text=True,
    ).stdout.splitlines()
    for command in commands:
        args = shlex.split(command)
        if "-c" in args or not any(arg.endswith("/src/main.cpp.o") for arg in args):
            continue
        if args[:2] == [":", "&&"]:
            args = args[2:]
        if "&&" in args:
            args = args[:args.index("&&")]
        return args
    raise RuntimeError("OpenKAI link command was not found; build the application first")


def main():
    repo = pathlib.Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build_directory", nargs="?", type=pathlib.Path, default=repo / "build")
    parser.add_argument("--mavlink-only", action="store_true", help="run MAVLink stream, codec, and consumer regressions only")
    options = parser.parse_args()
    build = options.build_directory.resolve()
    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next(item for item in entries if item["file"].endswith("/src/Protocol/_ProtocolBase.cpp"))
    link_args = link_command(build)
    sources = [
        "test/Protocol/BytePacketProtocols.cpp",
        "test/DataObject/BytePacketStream.cpp",
        "test/DataObject/MavlinkStream.cpp",
        "test/Autopilot/MavlinkStreams.cpp",
        "test/IO/BytePacketTransports.cpp",
        "test/IO/BytePacketWebSocket.cpp",
    ]
    with tempfile.TemporaryDirectory(prefix="openkai-byte-packets-") as temp:
        test_objects = []
        for index, source in enumerate(sources):
            test_object = str(pathlib.Path(temp) / f"packet-test-{index}.o")
            compile_args = shlex.split(entry["command"])
            compile_args[compile_args.index("-o") + 1] = test_object
            compile_args[compile_args.index("-c") + 1] = str(repo / source)
            if "-MF" in compile_args:
                compile_args[compile_args.index("-MF") + 1] = test_object + ".d"
            compile_args.extend(["-DOPENKAI_BYTE_PACKET_PROTOCOL_TEST", "-UNDEBUG"])
            subprocess.run(compile_args, cwd=entry["directory"], check=True)
            test_objects.append(test_object)
        executable = str(pathlib.Path(temp) / "byte-packet-tests")
        link_args = [arg for arg in link_args if not arg.startswith("-Wl,--dependency-file=")]
        main_object = next(arg for arg in link_args if arg.endswith("/src/main.cpp.o"))
        index = link_args.index(main_object)
        link_args[index:index + 1] = test_objects
        link_args[link_args.index("-o") + 1] = executable
        subprocess.run(link_args, cwd=build, check=True)
        test_args = [executable]
        if options.mavlink_only:
            test_args.append("--mavlink-only")
        subprocess.run(test_args, cwd=repo, check=True)


if __name__ == "__main__":
    main()
