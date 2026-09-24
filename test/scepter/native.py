"""Build and run Scepter frame regressions against an existing build; no device needed."""
import json
import re
import shlex
import subprocess
import sys
import tempfile
from pathlib import Path

build = Path(sys.argv[1] if len(sys.argv) > 1 else 'build').resolve()
# Test code calls spdlog directly, which can retain it even if the application's
# linker drops it under --as-needed. Check the actual executable as well.
dynamic = subprocess.check_output(['readelf', '-d', str(build / 'OpenKAI')], text=True)
needed = re.findall(r'\(NEEDED\).*?\[([^]]+)\]', dynamic)
if any(name.startswith('libglim.so') for name in needed):
    logger = next((i for i, name in enumerate(needed) if name.startswith('libspdlog.so')), None)
    sdk = next((i for i, name in enumerate(needed) if name.startswith('libScepter_api.so')), None)
    assert logger is not None and sdk is not None and logger < sdk, (
        'GLIM system spdlog must be retained before Scepter in executable DT_NEEDED', needed)
source = Path(__file__).with_name('native.cpp').resolve()
entry = next(e for e in json.loads((build / 'compile_commands.json').read_text()) if e['file'].endswith('/RGBD/_Scepter.cpp'))
with tempfile.TemporaryDirectory(prefix='openkai-scepter-test-') as tmp:
    obj, binary = str(Path(tmp) / 'test.o'), str(Path(tmp) / 'test')
    args = entry.get('arguments') or shlex.split(entry['command'])
    args[args.index('-o') + 1] = obj
    args[args.index('-c') + 1] = str(source)
    subprocess.run(args + ['-UNDEBUG'], cwd=build, check=True)
    link = shlex.split((build / 'CMakeFiles/OpenKAI.dir/link.txt').read_text())
    link = [obj if a.endswith('/src/main.cpp.o') else a for a in link if not a.startswith('-Wl,--dependency-file=')]
    link[link.index('-o') + 1] = binary
    subprocess.run(link, cwd=build, check=True)
    subprocess.run([binary], cwd=tmp, check=True, timeout=30)
