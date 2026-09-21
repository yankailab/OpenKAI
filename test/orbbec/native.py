"""Build and run camera/IMU checks with the completed OpenKAI build; no device needed."""
import json
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

build = Path(sys.argv[1] if len(sys.argv) > 1 else 'build').resolve()
source = Path(__file__).with_name('native.cpp').resolve()
entry = next(e for e in json.loads((build / 'compile_commands.json').read_text()) if e['file'].endswith('/RGBD/_Orbbec.cpp'))
with tempfile.TemporaryDirectory(prefix='openkai-orbbec-test-') as tmp:
    obj, binary = str(Path(tmp) / 'test.o'), str(Path(tmp) / 'test')
    args = entry.get('arguments') or shlex.split(entry['command'])
    args[args.index('-o') + 1] = obj
    args[args.index('-c') + 1] = str(source)
    subprocess.run(args + ['-UNDEBUG'], cwd=build, check=True)
    link = shlex.split((build / 'CMakeFiles/OpenKAI.dir/link.txt').read_text())
    link = [obj if a.endswith('/src/main.cpp.o') else a for a in link if not a.startswith('-Wl,--dependency-file=')]
    link[link.index('-o') + 1] = binary
    subprocess.run(link, cwd=build, check=True)
    subprocess.run([binary, tmp], cwd=tmp, check=True, timeout=30)
