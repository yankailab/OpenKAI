#!/usr/bin/env python3
"""Run packet storage, duplex transport, and parser tests against an existing CMake build.

Usage: python3 test/run_byte_packet_tests.py [build-directory]
Build OpenKAI with WITH_IO=ON and WITH_PROTOCOL=ON before running this script.
"""

import json
import pathlib
import shlex
import subprocess
import sys
import tempfile


def main():
    repo = pathlib.Path(__file__).resolve().parents[1]
    build = pathlib.Path(sys.argv[1]).resolve() if len(sys.argv) > 1 else repo / "build"
    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next(item for item in entries if item["file"].endswith("/src/Protocol/_ProtocolBase.cpp"))
    link_args = shlex.split((build / "CMakeFiles/OpenKAI.dir/link.txt").read_text())
    sources = [
        "test/Protocol/BytePacketProtocols.cpp",
        "test/DataObject/BytePacketStream.cpp",
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
        subprocess.run([executable], cwd=repo, check=True)


if __name__ == "__main__":
    main()
