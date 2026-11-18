#!/usr/bin/env python3
"""Test native Modbus configuration, reconnection and Modbus/EBYTE lifecycle.

Usage: python3 test/run_modbus_lifecycle_tests.py [build-directory]
Requires WITH_IO=ON, USE_LIBMODBUS=ON, and BUILD_EXECUTABLE=ON. No hardware is
used: connection tests use ephemeral loopback TCP ports and pseudo-terminals.
Every case runs separately with a timeout. Build flags and dependencies come
from CMake outputs.
"""
import argparse
import json
import os
from pathlib import Path
import shlex
import subprocess
import tempfile

from build_helpers import link_command


def main():
    repo = Path(__file__).resolve().parents[1]
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('build_directory', nargs='?', type=Path, default=repo / 'build')
    parser.add_argument('--headers', type=Path, help='matching OpenKAI header root (normally source/src)')
    parser.add_argument('--library', type=Path, help='matching shared library (normally build/libOpenKAI.so)')
    parser.add_argument('--case', action='append', choices=['stop-modbus', 'stop-ebyte', 'destroy-modbus',
                        'destroy-ebyte', 'manager-bus-first', 'manager-ebyte-first',
                        'configuration', 'reconfigure', 'concurrent-close', 'reconnect'])
    options = parser.parse_args()
    build = options.build_directory.resolve()
    cache = (build / 'CMakeCache.txt').read_text().splitlines()
    source = Path(next(line.split('=', 1)[1] for line in cache if line.startswith('CMAKE_HOME_DIRECTORY:')))
    headers = (options.headers or source / 'src').resolve()
    library = (options.library or build / 'libOpenKAI.so').resolve()
    link = link_command(build)
    compile_database = build / 'compile_commands.json'
    if compile_database.is_file():
        entry = next(item for item in json.loads(compile_database.read_text())
                     if item['file'].endswith('/src/Protocol/_Modbus.cpp'))
        compile_args = list(entry.get('arguments') or shlex.split(entry['command']))
        compile_cwd = Path(entry['directory'])
    else:
        if not (build / 'CMakeFiles/OpenKAI.dir/flags.make').is_file():
            parser.error('Configure CMake with CMAKE_EXPORT_COMPILE_COMMANDS=ON first')
        values = {}
        for line in (build / 'CMakeFiles/OpenKAI.dir/flags.make').read_text().splitlines():
            if ' = ' in line:
                key, value = line.split(' = ', 1)
                values[key] = shlex.split(value)
        compiler = next(line.split('=', 1)[1] for line in cache if line.startswith('CMAKE_CXX_COMPILER:'))
        compile_args = [compiler, *values['CXX_DEFINES'], *values['CXX_INCLUDES'], *values['CXX_FLAGS'],
                        '-c', 'source.cpp', '-o', 'test.o']
        compile_cwd = build
    environment = dict(os.environ, CCACHE_DISABLE='1')
    runtime_environment = environment.copy()
    runtime_environment['LD_LIBRARY_PATH'] = ':'.join(
        [str(library.parent)] + [path for path in environment.get('LD_LIBRARY_PATH', '').split(':') if path])
    # A renamed library can still advertise the standard OpenKAI SONAME. Load
    # the exact selected file in test children, without affecting the compiler.
    runtime_environment['LD_PRELOAD'] = ' '.join(
        [str(library)] + ([environment['LD_PRELOAD']] if environment.get('LD_PRELOAD') else []))
    cases = options.case or ['stop-modbus', 'stop-ebyte', 'destroy-modbus', 'destroy-ebyte',
                            'manager-bus-first', 'manager-ebyte-first',
                            'configuration', 'reconfigure', 'concurrent-close', 'reconnect']
    with tempfile.TemporaryDirectory(prefix='openkai-modbus-lifecycle-') as temp:
        work = Path(temp)
        obj, executable = work / 'test.o', work / 'modbus-lifecycle'
        compile_args[compile_args.index('-c') + 1] = str(Path(__file__).resolve().parent / 'IO/ModbusLifecycle.cpp')
        compile_args[compile_args.index('-o') + 1] = str(obj)
        if '-MF' in compile_args:
            compile_args[compile_args.index('-MF') + 1] = str(work / 'test.d')
        else:
            compile_args.extend(['-MF', str(work / 'test.d')])
        compile_args[1:1] = ['-I' + str(headers)]
        compile_args.append('-UNDEBUG')
        subprocess.run(compile_args, cwd=compile_cwd, env=environment, check=True)
        main_index = next(i for i, arg in enumerate(link) if arg.endswith('/src/main.cpp.o'))
        link[main_index] = str(obj)
        link[link.index('-o') + 1] = str(executable)
        library_index = next(i for i, arg in enumerate(link) if Path(arg).name.startswith('libOpenKAI.so'))
        link[library_index] = str(library)
        subprocess.run(link, cwd=build, env=environment, check=True)
        failed = []
        for case in cases:
            try:
                result = subprocess.run([str(executable), case, str(work)], cwd=source,
                                        env=runtime_environment, capture_output=True, text=True, timeout=10)
                if result.returncode:
                    print(f'FAIL: {case} exited {result.returncode}\n{result.stdout}{result.stderr}')
                    failed.append(case)
                else:
                    print(result.stdout.strip())
            except subprocess.TimeoutExpired as error:
                print(f'FAIL: {case} did not stop within 10 seconds\n{error.stdout or b""}\n{error.stderr or b""}')
                failed.append(case)
        if failed:
            raise SystemExit('Failed lifecycle cases: ' + ', '.join(failed))


if __name__ == '__main__':
    main()
