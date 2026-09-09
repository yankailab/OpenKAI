"""Build/run native grid + ImGUI API checks using an existing ImGUI-enabled build.

Usage: python3 test/webViewer3D/native_cells.py build
Requires CMAKE_EXPORT_COMPILE_COMMANDS=ON and a completed build (Ninja/Makefiles).
"""
import json
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

build = Path(sys.argv[1]).resolve()
source = Path(__file__).with_suffix('.cpp').resolve()
entry = next(e for e in json.loads((build / 'compile_commands.json').read_text())
             if e['file'].endswith('/Grid/_OctreeGrid.cpp'))
with tempfile.TemporaryDirectory(prefix='openkai-native-cells-') as tmp:
    obj, binary = str(Path(tmp) / 'native.o'), str(Path(tmp) / 'native')
    compile_args = entry.get('arguments') or shlex.split(entry['command'])
    compile_args[compile_args.index('-o') + 1] = obj
    compile_args[compile_args.index('-c') + 1] = str(source)
    compile_args.append('-UNDEBUG')
    subprocess.run(compile_args, cwd=entry['directory'], check=True)
    link_file = build / 'CMakeFiles/OpenKAI.dir/link.txt'
    if link_file.exists():
        link = shlex.split(link_file.read_text())
    else:
        commands = subprocess.check_output(['ninja', '-t', 'commands'], cwd=build, text=True)
        line = next(line for line in reversed(commands.splitlines()) if 'src/main.cpp.o' in line and ' -o ' in line and ' -c ' not in line)
        link = shlex.split(line)
        if link[:2] == [':', '&&']: link = link[2:]
        if '&&' in link: link = link[:link.index('&&')]
    link = [obj if arg.endswith('/src/main.cpp.o') else arg for arg in link
            if not arg.startswith('-Wl,--dependency-file=')]
    link[link.index('-o') + 1] = binary
    subprocess.run(link, cwd=build, check=True)
    subprocess.run([binary], check=True)
