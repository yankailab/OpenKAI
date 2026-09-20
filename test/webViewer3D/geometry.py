"""Build and check the real simple viewer with synthetic point frames and lines.

Usage: python3 test/webViewer3D/geometry.py build
Uses the completed OpenKAI build's compile/link settings; no camera is needed.
"""
import json
import shlex
import socket
import struct
import subprocess
import sys
import tempfile
import time
from pathlib import Path
from integration import WebSocket, read_frame, request

build = Path(sys.argv[1]).resolve()
source = Path(__file__).with_name('geometry_fixture.cpp').resolve()
root = source.parents[2] / 'html/viewer/_GeometryBase'
entry = next(e for e in json.loads((build / 'compile_commands.json').read_text())
             if e['file'].endswith('/Grid/_SelectableOctGrid.cpp'))
with tempfile.TemporaryDirectory(prefix='openkai-geometry-') as tmp:
    obj, binary = str(Path(tmp) / 'geometry.o'), str(Path(tmp) / 'geometry')
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
    for framed, expiry in [(False, 0), (True, 0), (False, 1), (True, 1)]:
        with socket.socket() as reserve:
            reserve.bind(('127.0.0.1', 0))
            port = reserve.getsockname()[1]
        server = subprocess.Popen([binary, str(root), str(port), str(int(framed)), str(expiry)],
                                  stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
        clients = []
        try:
            assert server.stdout.readline().strip() == 'READY'
            status, _, body = request(port, '/')
            assert status == 200 and b'<aside' not in body and b'wsCmd' not in body
            assert request(port, '/js/gridCellPicker.js')[0] == 404
            try:
                cell = WebSocket(port, '/stream/cells')
            except AssertionError as error:
                assert b'404' in error.args[0]
            else:
                cell.close()
                raise AssertionError('Cell stream accepted')
            frames = []
            for kind, name in [(1, 'points'), (2, 'lines')]:
                client = WebSocket(port, '/stream/' + name)
                clients.append(client)
                hello = json.loads(client.receive()[1])
                assert hello['version'] == 5 and hello['stream'] == name
                assert [o['name'] for o in hello['objects']] == ['points', 'lines', 'empty']
                client.send('start')
                opcode, data = client.receive()
                assert opcode == 2
                frames.append(read_frame(data, kind))
                # Flow control waits for this stream's next credit.
                client.socket.settimeout(.1)
                try:
                    client.receive()
                except TimeoutError:
                    pass
                else:
                    raise AssertionError('Frame arrived without credit')
                client.socket.settimeout(3)
            points, lines = frames
            expected = 0 if expiry else 2 if framed else 4
            assert [(o['id'], o['count']) for o in points] == [(0, expected), (2, 0)]
            assert [(o['id'], o['count']) for o in lines] == [(1, 0 if expiry else 1)]
            if not expiry:
                xs = [p[0] for p in struct.iter_unpack('<3f', points[0]['positions'])]
                assert xs == ([0, 1] if framed else [2, 1, 0, -1]), xs
                assert bytes(lines[0]['colors']) == bytes([255, 0, 255] * 2)
            # Clearing sources sends empty snapshots on both streams.
            server.stdin.write('clear\n'); server.stdin.flush()
            assert server.stdout.readline().strip() == 'OK'
            for kind, client in enumerate(clients, 1):
                for _ in range(20):
                    client.send('next')
                    objects = read_frame(client.receive()[1], kind)
                    if all(o['count'] == 0 for o in objects): break
                    time.sleep(.04)
                else:
                    raise AssertionError('Stale geometry after clear')
            print(f'PASS: real _WebGeometryBase bFrame={framed}, expiry={expiry}, independent streams and clear')
        finally:
            for client in clients: client.close()
            server.communicate('stop\n', timeout=5)
            assert server.returncode == 0
