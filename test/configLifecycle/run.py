#!/usr/bin/env python3
"""Run configuration lifecycle checks using an existing CMake executable build."""

import argparse
import json
from pathlib import Path
import shlex
import subprocess
import tempfile


def compiler_flags(entry):
    command = entry.get("arguments") or shlex.split(entry["command"])
    flags = []
    index = 1
    while index < len(command):
        argument = command[index]
        if argument in ("-o", "-MF", "-MT", "-MQ"):
            index += 2
            continue
        if argument not in ("-c", "-MMD", "-MD", "-MP", entry["file"]):
            flags.append(argument)
        index += 1
    return command[0], flags


def executable_link_command(build):
    if (build / "build.ninja").is_file():
        commands = subprocess.check_output(
            ["ninja", "-t", "commands", "OpenKAI"], cwd=build, text=True
        )
    else:
        commands = (build / "CMakeFiles/OpenKAI.dir/link.txt").read_text()
    for line in commands.splitlines():
        for part in line.split("&&"):
            command = shlex.split(part)
            if "-o" in command and "-c" not in command:
                if any(value.endswith("/src/main.cpp.o") for value in command):
                    return command
    raise RuntimeError("Cannot find the OpenKAI executable link command")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("build", type=Path, help="Configured CMake build directory")
    parser.add_argument("--skip-build", action="store_true", help="Use already built objects")
    args = parser.parse_args()
    build = args.build.resolve()
    root = Path(__file__).resolve().parents[2]
    if not args.skip_build:
        subprocess.run(["cmake", "--build", str(build)], check=True)

    entries = json.loads((build / "compile_commands.json").read_text())
    entry = next(item for item in entries if item["file"].endswith("/src/Base/BASE.cpp"))
    compiler, flags = compiler_flags(entry)
    with tempfile.TemporaryDirectory(prefix="openkai-config-lifecycle-") as directory:
        temporary = Path(directory)
        test_object = temporary / "configLifecycle.o"
        subprocess.run(
            [compiler, *flags, "-UNDEBUG", "-I" + str(root), "-c",
             str(root / "test/configLifecycle/configLifecycle.cpp"), "-o", str(test_object)],
            cwd=entry["directory"], check=True
        )
        command = executable_link_command(build)
        command = [value for value in command if not value.endswith("/src/main.cpp.o")]
        executable = temporary / "configLifecycle"
        command[command.index("-o") + 1] = str(executable)
        command[1:1] = [str(test_object)]
        subprocess.run(command, cwd=build, check=True)
        subprocess.run([str(executable), str(root)], cwd=temporary, check=True)


if __name__ == "__main__":
    main()
