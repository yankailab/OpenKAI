"""Smoke-test the real framework module with the supplied bunny/octree config."""
import argparse
import json
import signal
import socket
import subprocess
import tempfile
import time
from pathlib import Path
from integration import WebSocket, request, read_frame


def check(executable, cells_only, cell_alpha):
    root = Path(__file__).resolve().parents[2]
    with tempfile.TemporaryDirectory(prefix='openkai-backend-') as tmp:
        config = json.loads((root / 'jsonCfg/WebViewer3D.json').read_text())
        with socket.socket() as s:
            s.bind(('127.0.0.1', 0))
            port = s.getsockname()[1]
        config['pcFile']['bON'] = True
        for name in ['cam', 'scPC', 'wsCmdServer', 'wsCmdConsole']:
            config[name]['bON'] = False
        config['octGrid']['vGeometryBase'] = ['pcFile']
        config['viewer']['vGeometry'] = [
            {'_GeometryBase': 'pcFile', 'nP': 400000, 'nL': 0},
            {'_GeometryBase': 'octGrid', 'nP': 0, 'nL': 0, 'nC': 8333}]
        if cells_only:
            config['viewer']['nPbuf'] = config['viewer']['nLbuf'] = 0
            config['viewer']['vGeometry'] = config['viewer']['vGeometry'][1:]
        config['viewer']['host'] = '127.0.0.1'
        config['viewer']['port'] = port
        path = Path(tmp) / 'viewer.json'
        path.write_text(json.dumps(config))
        with (Path(tmp) / 'backend.log').open('w+') as log:
            process = subprocess.Popen([executable, str(path)], cwd=root, stdout=log, stderr=log)
            clients = []
            try:
                deadline = time.monotonic() + 15
                while True:
                    try:
                        assert request(port, '/')[0] == 200
                        break
                    except OSError:
                        if process.poll() is not None or time.monotonic() > deadline:
                            log.seek(0)
                            raise AssertionError(log.read())
                        time.sleep(.1)
                for kind in ['points', 'lines', 'cells']:
                    client = WebSocket(port, '/stream/' + kind); clients.append(client)
                    hello = json.loads(client.receive()[1])
                    assert hello['version'] == 4 and hello['stream'] == kind
                    assert {o['name'] for o in hello['objects']} == ({'octGrid'} if cells_only else {'pcFile', 'octGrid'})
                    client.send('start')
                for _ in range(100):
                    frames = [read_frame(c.receive()[1], kind) for kind, c in enumerate(clients, 1)]
                    points = sum(o['count'] for o in frames[0])
                    assert frames[1] == []
                    assert len(frames[2]) == 1
                    cells = frames[2][0]['count']
                    if cells_only: assert frames[0] == []
                    if cells and (cells_only or points): break
                    for c in clients: c.send('next')
                    time.sleep(.05)
                assert cells > 0 and (cells_only or points > 0), (points, cells)
                payload = frames[2][0]['cells']
                colors = {tuple(payload[i * 20 + 16:(i + 1) * 20]) for i in range(cells)}
                assert colors == {(255, 255, 255, round(cell_alpha * 255))}, colors
                print(f'PASS: real _WebViewer3D: {points} points, {cells} cells, independent empty lines' +
                      ('; point/line buffers disabled' if cells_only else ''))
                process.send_signal(signal.SIGINT)
                process.wait(timeout=5)
                assert process.returncode == 0
            finally:
                for client in clients: client.close()
                if process.poll() is None:
                    process.kill()
                    process.wait()


if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('executable')
    parser.add_argument('--cell-alpha', type=float, default=1, help='Expected alpha when testing a temporary grid alpha override')
    args = parser.parse_args()
    for cells_only in (False, True): check(args.executable, cells_only, args.cell_alpha)
