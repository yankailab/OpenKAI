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
                    points = lines = 0
                    for _ in range(count):
                        _, nP, nL = struct.unpack_from('<III', frame, at)
                        points += nP
                        lines += nL
                        at += 64 + nP * 16 + nL * 32
                    assert at == len(frame)
                    if points and lines: break
                    client.send('next')
                    time.sleep(.05)
                assert points > 0 and lines > 0, (points, lines)
                print(f'PASS: real _WebViewer3D streams {points} bunny points and {lines} octree lines')
                process.send_signal(signal.SIGINT)
                process.wait(timeout=5)
                assert process.returncode == 0
            finally:
                if client: client.close()
                if process.poll() is None:
                    process.kill()
                    process.wait()


if __name__ == '__main__': main()
