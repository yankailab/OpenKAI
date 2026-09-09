"""Check octree wireframes through the real geometry output consumed by viewers.

Run: python3 test/webViewer3D/octree.py build/OpenKAI-0.0.1
Requires a build with USE_WEBVIEWER3D=ON; uses only the Python standard library.
"""
from collections import Counter
from itertools import combinations, product
import json
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import time
from pathlib import Path

from integration import WebSocket


def box_edges(lo, hi):
    vertices = list(product(*zip(lo, hi)))
    return Counter((a, b) for a, b in combinations(vertices, 2)
                   if sum(x != y for x, y in zip(a, b)) == 1)


def check(executable, name, points, depth, boxes, size=(8, 8, 8),
          origin=(0, 0, 0), max_lines=1000):
    root = Path(__file__).resolve().parents[2]
    expected = sum((box_edges(*box) for box in boxes), Counter())
    with tempfile.TemporaryDirectory(prefix='openkai-octree-') as tmp:
        folder = Path(tmp)
        ply = folder / 'points.ply'
        ply.write_text('ply\nformat ascii 1.0\nelement vertex ' + str(len(points)) +
                       '\nproperty float x\nproperty float y\nproperty float z\nend_header\n' +
                       ''.join(' '.join(map(str, p)) + '\n' for p in points))
        with socket.socket() as listener:
            listener.bind(('127.0.0.1', 0))
            port = listener.getsockname()[1]
        config = {
            'APP': {'class': 'ModuleMgr', 'appName': 'OctreeTest', 'bStdErr': True},
            'points': {'class': '_PCfile', 'thread': {'FPS': 30},
                       'nP': 16, 'vfName': [str(ply)]},
            'octGrid': {'class': '_OctreeGrid', 'thread': {'FPS': 30},
                        'nP': 16, 'vPorigin': origin, 'vRootCellSize': size,
                        'nMaxLevel': depth, 'nMaxLines': max_lines,
                        'vGeometryBase': ['points']},
            'viewer': {'class': '_WebViewer3D', 'thread': {'FPS': 30},
                       'host': '127.0.0.1', 'port': port,
                       'webRoot': str(root / 'html/webViewer3D'),
                       'nPbuf': 16, 'nLbuf': 1024,
                       'vGeometryBase': ['points', 'octGrid']},
        }
        path = folder / 'config.json'
        path.write_text(json.dumps(config))
        with (folder / 'backend.log').open('w+') as log:
            process = subprocess.Popen([executable, str(path)], cwd=folder,
                                       stdout=log, stderr=log)
            client = None
            try:
                deadline = time.monotonic() + 10
                while client is None:
                    try:
                        client = WebSocket(port)
                    except OSError:
                        if process.poll() is not None or time.monotonic() > deadline:
                            log.seek(0)
                            raise AssertionError(log.read())
                        time.sleep(.05)
                opcode, hello = client.receive()
                assert opcode == 1
                names = {o['id']: o['name'] for o in json.loads(hello)['objects']}
                client.send('start')
                ready_at = None
                while time.monotonic() < deadline:
                    opcode, frame = client.receive()
                    assert opcode == 2
                    count = struct.unpack_from('<I', frame, 12)[0]
                    at, n_points, actual = 32, 0, Counter()
                    for _ in range(count):
                        object_id, nP, nL = struct.unpack_from('<III', frame, at)
                        n_points += nP
                        if names[object_id] == 'octGrid':
                            start = at + 64 + nP * 16
                            for i in range(nL):
                                coords = struct.unpack_from('<6f', frame, start + i * 24)
                                actual[tuple(sorted((coords[:3], coords[3:])))] += 1
                        at += 64 + nP * 16 + nL * 32
                    assert at == len(frame)
                    # Let the grid consume the file before asserting empty output, too.
                    if n_points == len(points) and ready_at is None:
                        ready_at = time.monotonic()
                    if ready_at is not None and time.monotonic() - ready_at > .3:
                        break
                    client.send('next')
                    time.sleep(.05)
                assert ready_at is not None, f'{name}: point source did not load'
                assert actual == expected, (
                    f'{name}: expected {sum(expected.values())} edges, '
                    f'got {sum(actual.values())}; '
                    f'missing {sum((expected - actual).values())}, '
                    f'unexpected {sum((actual - expected).values())}')
                print(f'PASS: {name} ({sum(actual.values())} lines)')
                process.send_signal(signal.SIGINT)
                process.wait(timeout=5)
                assert process.returncode == 0
            finally:
                if client:
                    client.close()
                if process.poll() is None:
                    process.kill()
                    process.wait()


def main():
    executable = str(Path(sys.argv[1]).resolve())
    root_box = ((-4, -4, -4), (4, 4, 4))
    positive_boxes = [((0, 0, 0), (4, 4, 4)),
                      ((2, 2, 2), (4, 4, 4)),
                      ((3, 3, 3), (4, 4, 4)),
                      ((3, 3, 3), (3.5, 3.5, 3.5))]
    point = (3.25, 3.25, 3.25)
    check(executable, 'all five occupied levels', [point], 4,
          [root_box] + positive_boxes)
    check(executable, 'root-only grid', [point], 0, [root_box])
    check(executable, 'shared ancestors and opposite octants',
          [point, (3.375, 3.375, 3.375), (-3.25, -3.25, -3.25)], 2,
          [root_box] + positive_boxes[:2] +
          [((-4, -4, -4), (0, 0, 0)), ((-4, -4, -4), (-2, -2, -2))])
    check(executable, 'translated rectangular cells', [(13.25, -18.375, 30.8125)], 2,
          [((6, -22, 29), (14, -18, 31)),
           ((10, -20, 30), (14, -18, 31)),
           ((12, -19, 30.5), (14, -18, 31))],
          size=(8, 4, 2), origin=(10, -20, 30))
    check(executable, 'line cap keeps complete boxes', [point], 4,
          [root_box, positive_boxes[0]], max_lines=25)
    check(executable, 'out-of-bounds point leaves grid empty', [(5, 5, 5)], 4, [])


if __name__ == '__main__':
    main()
