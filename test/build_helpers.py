"""Read the current CMake build's executable link command for regression runners."""

import shlex
import subprocess


def link_command(build):
    cache = {}
    for line in (build / "CMakeCache.txt").read_text().splitlines():
        if line.startswith(("#", "//")) or "=" not in line:
            continue
        key, value = line.split("=", 1)
        cache[key.split(":", 1)[0]] = value

    # Select the configured generator, ignoring stale files from older builds.
    generator = cache.get("CMAKE_GENERATOR")
    target = "OpenKAI_executable"
    if generator == "Ninja":
        result = subprocess.run(
            [cache.get("CMAKE_MAKE_PROGRAM", "ninja"), "-C", str(build), "-t", "commands", target],
            capture_output=True, text=True,
        )
        commands = result.stdout.splitlines() if result.returncode == 0 else []
    elif generator == "Unix Makefiles":
        link_file = build / f"CMakeFiles/{target}.dir/link.txt"
        commands = [link_file.read_text()] if link_file.is_file() else []
    else:
        raise RuntimeError(f"Unsupported CMake generator for regression tests: {generator}")

    for command in commands:
        args = shlex.split(command)
        if "-c" in args or not any(arg.endswith("/src/main.cpp.o") for arg in args):
            continue
        if args[:2] == [":", "&&"]:
            args = args[2:]
        if "&&" in args:
            args = args[:args.index("&&")]
        # Test links must not overwrite the application's linker dependency file.
        return [arg for arg in args if not arg.startswith("-Wl,--dependency-file=")]
    raise RuntimeError("OpenKAI executable link command was not found; build with BUILD_EXECUTABLE=ON first")
