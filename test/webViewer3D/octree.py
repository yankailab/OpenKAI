"""Check compact octree snapshots and reconstruct their boxes independently.

Run: python3 test/webViewer3D/octree.py build/OpenKAI-0.0.1
Requires a build with USE_WEBVIEWER3D=ON; uses only the Python standard library.
"""
from collections import Counter
from itertools import combinations, product
import argparse
import json
import signal
import socket
import subprocess
import tempfile
import time
from pathlib import Path

from integration import WebSocket, read_frame


def box_edges(lo, hi):
    vertices = list(product(*zip(lo, hi)))
    return Counter((a, b) for a, b in combinations(vertices, 2)
                   if sum(x != y for x, y in zip(a, b)) == 1)


def check(executable, name, points, depth, boxes, size=(8, 8, 8),
          origin=(0, 0, 0), max_lines=1000, max_cells=None, viewer_cells=100000, color=None, alpha=None):
    root = Path(__file__).resolve().parents[2]
    expected = sum((box_edges(*box) for box in boxes), Counter())
    with tempfile.TemporaryDirectory(prefix='openkai-octree-') as tmp:
        folder = Path(tmp)
        ply = folder / 'points.ply'
        ply.write_text('ply\nformat ascii 1.0\nelement vertex ' + str(len(points)) +
                       '\nproperty float x\nproperty float y\nproperty float z\n' +
                       ('property uchar alpha\n' if alpha is not None else '') + 'end_header\n' +
                       ''.join(' '.join(map(str, p)) + (f' {alpha}' if alpha is not None else '') + '\n' for p in points))
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
                       'vGeometryBase': ['octGrid', 'points'], 'nCbuf': viewer_cells},
        }
        if max_cells is not None: config['octGrid']['nMaxCells'] = max_cells
        if color is not None: config['octGrid']['vColCellOcc'] = color
        path = folder / 'config.json'
        path.write_text(json.dumps(config))
        with (folder / 'backend.log').open('w+') as log:
            process = subprocess.Popen([executable, str(path)], cwd=folder,
                                       stdout=log, stderr=log)
            client = point_client = None
            try:
                deadline = time.monotonic() + 10
                while client is None:
                    try:
                        client = WebSocket(port, '/stream/cells')
                    except OSError:
                        if process.poll() is not None or time.monotonic() > deadline:
                            log.seek(0)
                            raise AssertionError(log.read())
                        time.sleep(.05)
                opcode, hello = client.receive()
                assert opcode == 1
                names = {o['id']: o['name'] for o in json.loads(hello)['objects']}
                point_client = WebSocket(port, '/stream/points')
                point_client.receive()
                point_client.send('start')
                client.send('start')
                ready_at = None
                while time.monotonic() < deadline:
                    opcode, frame = client.receive()
                    assert opcode == 2
                    point_objects = read_frame(point_client.receive()[1], 1)
                    n_points = sum(o['count'] for o in point_objects)
                    for obj in point_objects:
                        assert all(a == (255 if alpha is None else alpha) for a in obj['colors'][3::4])
                    actual = Counter()
                    objects = read_frame(frame, 3)
                    assert len(objects) == 1 and names[objects[0]['id']] == 'octGrid'
                    obj = objects[0]
                    header = obj['header']
                    assert header[:3] == tuple(origin) and header[3:6] == tuple(size)
                    assert header[6] == depth and header[7] == 0
                    ids = set()
                    for i in range(obj['count']):
                        record = obj['cells'][i * 20:(i + 1) * 20]
                        cell_id = int.from_bytes(record[:16], 'little')
                        level = cell_id & 63
                        assert level <= depth and cell_id >> 126 == 0
                        assert cell_id & ((1 << (126 - 3 * level)) - 64) == 0
                        assert cell_id not in ids
                        ids.add(cell_id)
                        c, extent = list(origin), list(size)
                        for j in range(level):
                            child = (cell_id >> (123 - 3 * j)) & 7
                            for axis in range(3):
                                c[axis] += extent[axis] * (.25 if child & (4 >> axis) else -.25)
                                extent[axis] *= .5
                        actual += box_edges(tuple(c[a] - extent[a] / 2 for a in range(3)),
                                            tuple(c[a] + extent[a] / 2 for a in range(3)))
                        expected_rgba = bytes(int(x * 255 + .5) for x in color) if color else bytes([255] * 3)
                        if len(expected_rgba) == 3:
                            expected_rgba += bytes([255 if color or alpha is None else alpha])
                        if not color and check.cell_alpha is not None:
                            expected_rgba = expected_rgba[:3] + bytes([int(check.cell_alpha * 255 + .5)])
                        assert record[16:] == expected_rgba, (record[16:], expected_rgba)
                    # Let the grid consume the file before asserting empty output, too.
                    if n_points == len(points) and ready_at is None:
                        ready_at = time.monotonic()
                    if ready_at is not None and time.monotonic() - ready_at > .3:
                        break
                    client.send('next')
                    point_client.send('next')
                    time.sleep(.05)
                assert ready_at is not None, f'{name}: point source did not load'
                assert actual == expected, (
                    f'{name}: expected {sum(expected.values())} edges, '
                    f'got {sum(actual.values())}; '
                    f'missing {sum((expected - actual).values())}, '
                    f'unexpected {sum((actual - expected).values())}')
                print(f'PASS: {name} ({sum(actual.values()) // 12} cells, 20 bytes each)')
                process.send_signal(signal.SIGINT)
                process.wait(timeout=5)
                assert process.returncode == 0
            finally:
                if client: client.close()
                if point_client: point_client.close()
                if process.poll() is None:
                    process.kill()
                    process.wait()


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('executable')
    parser.add_argument('--cell-alpha', type=float, help='Expected alpha when testing a temporary grid alpha override')
    args = parser.parse_args()
    executable = str(Path(args.executable).resolve())
    check.cell_alpha = args.cell_alpha
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
    check(executable, 'explicit cell cap and RGB override', [point], 4,
          [root_box, positive_boxes[0]], max_cells=2, color=(1, .2, 0))
    check(executable, 'explicit RGBA override', [point], 0, [root_box], color=(1, .2, 0, .25))
    check(executable, 'point and cell alpha from PLY', [point], 0, [root_box], alpha=64)
    check(executable, 'fully transparent point and cell', [point], 0, [root_box], alpha=0)
    check(executable, 'viewer cell cap independent of line cap', [point], 4,
          [root_box], viewer_cells=1)
    check(executable, 'zero cell cap retains empty header', [point], 4, [], max_cells=0)
    for octant in range(8):
        lo = tuple(0 if octant & (4 >> a) else -4 for a in range(3))
        hi = tuple(4 if octant & (4 >> a) else 0 for a in range(3))
        p = tuple(2 if octant & (4 >> a) else -2 for a in range(3))
        check(executable, f'octant {octant}', [p], 1, [root_box, (lo, hi)])
    check(executable, 'out-of-bounds point leaves grid empty', [(5, 5, 5)], 4, [])


if __name__ == '__main__':
    main()
