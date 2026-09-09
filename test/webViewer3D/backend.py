"""Smoke-test the real framework module with the supplied bunny/octree config."""
import json
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import time
from pathlib import Path
from integration import WebSocket, request


def main():
    executable = sys.argv[1]
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
        config['viewer']['host'] = '127.0.0.1'
        config['viewer']['port'] = port
        path = Path(tmp) / 'viewer.json'
        path.write_text(json.dumps(config))
        with (Path(tmp) / 'backend.log').open('w+') as log:
            process = subprocess.Popen([executable, str(path)], cwd=root, stdout=log, stderr=log)
            client = None
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
                client = WebSocket(port)
                _, payload = client.receive()
                hello = json.loads(payload)
                assert {o['name'] for o in hello['objects']} == {'pcFile', 'octGrid'}
                client.send('start')
                for _ in range(100):
                    opcode, frame = client.receive()
                    assert opcode == 2
                    count = struct.unpack_from('<I', frame, 12)[0]
                    assert count == 2
                    at = 32
                    points = cells = 0
                    cell_colors = set()
                    for _ in range(count):
                        _, nP, nL = struct.unpack_from('<III', frame, at)
                        points += nP
                        nC = struct.unpack_from('<I', frame, at + 12)[0]
                        flags = struct.unpack_from('<I', frame, at + 48)[0]
                        cells += nC
                        at += 64 + nP * 16 + nL * 32
                        if flags & 1:
                            cell_colors.update(tuple(frame[at + 40 + i * 19 + 16:at + 40 + (i + 1) * 19])
                                               for i in range(nC))
                            at = (at + 40 + nC * 19 + 3) & ~3
                    assert at == len(frame)
                    if points and cells: break
                    client.send('next')
                    time.sleep(.05)
                assert points > 0 and cells > 0, (points, cells)
                # This PLY has no RGB; the sample's point default is white.
                # A legacy grid override must not silently tint its cell stream.
                assert cell_colors == {(255, 255, 255)}, f'Unexpected grid RGB: {cell_colors}'
                print(f'PASS: real _WebViewer3D streams {points} bunny points and {cells} octree cells')
                process.send_signal(signal.SIGINT)
                process.wait(timeout=5)
                assert process.returncode == 0
            finally:
                if client: client.close()
                if process.poll() is None:
                    process.kill()
                    process.wait()


if __name__ == '__main__': main()
