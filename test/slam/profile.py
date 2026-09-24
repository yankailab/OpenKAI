"""Measure live GLIM stage costs through WSconsole, with map streaming active.

python3 test/slam/profile.py build/OpenKAI --seconds 30 --output /tmp/glim-profile.json
Requires the camera from GLIM_orbbec.json. Uses temporary ports/configuration.
"""
import argparse
import json
import signal
import socket
import statistics
import struct
import subprocess
import sys
import tempfile
import threading
import time
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'webViewer3D'))
from integration import WebSocket


def free_port():
    with socket.socket() as sock:
        sock.bind(('127.0.0.1', 0))
        return sock.getsockname()[1]


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('executable', type=Path)
    parser.add_argument('--seconds', type=float, default=30)
    parser.add_argument('--warmup', type=float, default=10)
    parser.add_argument('--output', type=Path, default=Path('/tmp/glim-profile.json'))
    args = parser.parse_args()
    if args.seconds <= 0 or args.warmup < 0:
        parser.error('seconds must be positive and warmup must be nonnegative')
    root = Path(__file__).resolve().parents[2]
    executable = args.executable.resolve()
    config = json.loads((root / 'jsonCfg/GLIM_orbbec.json').read_text())
    for name in ['console', 'view', 'viewD', 'd2rgb']:
        config[name]['bON'] = False
    for name in ['viewer', 'wsServer']:
        config[name]['host'], config[name]['port'] = '127.0.0.1', free_port()
    config['GLIM']['bAutoStart'] = False
    log_path = args.output.with_suffix('.log')
    stop = threading.Event()
    samples, stream_errors = [], []
    stream_count = stream_bytes = 0
    submaps = {}
    client = stream = worker = None
    with tempfile.TemporaryDirectory(prefix='openkai-glim-profile-') as folder, log_path.open('w') as log:
        path = Path(folder) / 'config.json'
        path.write_text(json.dumps(config))
        backend = subprocess.Popen([str(executable), str(path)], cwd=root, stdout=log, stderr=log)
        try:
            deadline = time.monotonic() + 30
            while time.monotonic() < deadline:
                try:
                    client = WebSocket(config['wsServer']['port'], '/')
                    break
                except OSError:
                    if backend.poll() is not None:
                        raise RuntimeError(f'Backend exited; see {log_path}')
                    time.sleep(.1)
            if client is None:
                raise RuntimeError(f'Backend unavailable; see {log_path}')
            client.socket.settimeout(60)
            request_id = 0
            def command(cmd):
                nonlocal request_id
                request_id += 1
                client.send(json.dumps({'module': 'GLIM', 'cmd': cmd, 'requestId': request_id}) + 'EOJ')
                data = ''
                deadline = time.monotonic() + 60
                while True:
                    remaining = deadline - time.monotonic()
                    if remaining <= 0:
                        raise TimeoutError(f'No reply to {cmd}')
                    client.socket.settimeout(remaining)
                    opcode, payload = client.receive()
                    if opcode == 8:
                        raise EOFError('Command connection closed')
                    if opcode != 1:
                        continue
                    data += payload.decode()
                    try:
                        message = json.loads(data)
                    except json.JSONDecodeError:
                        continue
                    data = ''
                    if message.get('requestId') == request_id:
                        if not message.get('bSuccess'):
                            raise RuntimeError(message)
                        if 'status' not in message:
                            raise RuntimeError(f'Missing status: {message}')
                        return message['status']

            stream = WebSocket(config['viewer']['port'], '/stream/glim')
            opcode, hello = stream.receive()
            assert opcode == 1 and json.loads(hello)['protocol'] == 'openkai.glim'
            stream.send('start')
            def drain():
                nonlocal stream_count, stream_bytes
                try:
                    while not stop.is_set():
                        opcode, payload = stream.receive()
                        if opcode == 8:
                            if not stop.is_set():
                                stream_errors.append('Geometry stream closed before profiling finished')
                            break
                        if opcode == 1:
                            message = json.loads(payload)
                            if message['type'] == 'reset':
                                submaps.clear()
                            elif message['type'] == 'submap':
                                submaps[message['id']] = [message['pointCount'], 0]
                        elif opcode == 2:
                            magic, version, kind, header, session, submap, stamp, total, offset, count, reserved = struct.unpack_from('<IIIIQQQIIII', payload)
                            assert (magic, version, kind, header, reserved) == (0x314d4c47, 1, 1, 56, 0)
                            assert len(payload) == 56 + count * 12
                            entry = submaps[str(submap)]
                            assert entry == [total, offset]
                            entry[1] += count
                            stream_count += 1
                            stream_bytes += len(payload)
                        if opcode in (1, 2):
                            stream.send('next')
                except (OSError, EOFError, ValueError, KeyError, AssertionError, struct.error) as error:
                    if not stop.is_set():
                        stream_errors.append(str(error))
            stream.socket.settimeout(60)
            worker = threading.Thread(target=drain)
            worker.start()
            command('start')
            start = time.monotonic()
            while time.monotonic() - start < args.warmup + args.seconds:
                status = command('getStatus')
                status['wallSeconds'] = time.monotonic() - start
                status['streamFrames'] = stream_count
                samples.append(status)
                print(json.dumps({key: status[key] for key in ('wallSeconds', 'state', 'frames', 'processingMs', 'mapPoints')}), flush=True)
                time.sleep(.5)
            retained = command('stop')
            deadline = time.monotonic() + 10
            while time.monotonic() < deadline and not stream_errors:
                if len(submaps) == retained['submaps'] and all(total == received for total, received in submaps.values()):
                    break
                time.sleep(.05)
            valid = [s for s in samples if s['wallSeconds'] >= args.warmup and s['poseValid']]
            if len(valid) < 2 or valid[-1]['frames'] <= valid[0]['frames']:
                raise RuntimeError(f'No sustained tracking; see {log_path}')
            first, last = valid[0], valid[-1]
            count = last['frames'] - first['frames']
            summary = {
                'framesPerSecond': count / (last['wallSeconds'] - first['wallSeconds']),
                'sampledMedianProcessingMs': statistics.median(s['processingMs'] for s in valid),
                'workMsPerFrame': {key: (value - first['workMs'][key]) / count for key, value in last['workMs'].items()},
                'inputPoints': last['inputPoints'], 'registrationPoints': last['registrationPoints'],
                'mapPoints': last['mapPoints'], 'maxIMUgapUs': last['maxIMUgapUs'],
                'streamFrames': stream_count, 'streamErrors': stream_errors,
                'streamBytes': stream_bytes, 'streamSubmaps': len(submaps),
            }
            args.output.write_text(json.dumps({'summary': summary, 'samples': samples, 'stopped': retained}, indent=2) + '\n')
            print(json.dumps(summary, indent=2), flush=True)
            if stream_errors or not submaps or len(submaps) != retained['submaps'] or any(total != received for total, received in submaps.values()):
                raise RuntimeError(stream_errors or 'Completed submaps were not fully streamed')
        finally:
            stop.set()
            if stream:
                try:
                    stream.socket.shutdown(socket.SHUT_RDWR)
                except OSError:
                    pass
                stream.close()
            if worker:
                worker.join(timeout=5)
            if client:
                client.close()
            if backend.poll() is None:
                backend.send_signal(signal.SIGINT)
                try:
                    backend.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    backend.kill()
                    backend.wait()


if __name__ == '__main__':
    main()
