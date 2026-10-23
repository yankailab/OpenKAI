#!/usr/bin/env python3
"""Verify native Airy reception, web geometry and optional GLIM mapping.

python3 test/Sensor/LiDAR/airy.py build/OpenKAI --live --seconds 10 --browser
python3 test/Sensor/LiDAR/airy.py build/OpenKAI --live --glim --seconds 15
--capture replays records written as <QHH (host nanoseconds, UDP port, length)
followed by the UDP payload. It preserves sensor timestamps. No sensor commands
or network configuration changes are made. Python standard library only.
"""

import argparse
import importlib.util
import json
import math
from pathlib import Path
import select
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import threading
import time

ROOT = Path(__file__).resolve().parents[3]
sys.dont_write_bytecode = True
spec = importlib.util.spec_from_file_location('sensor_helpers', ROOT / 'test/Sensor/LineScanner/live.py')
helpers = importlib.util.module_from_spec(spec)
spec.loader.exec_module(helpers)


class Commands:
    def __init__(self, port):
        self.ws = helpers.WebSocket(f'ws://127.0.0.1:{port}/', timeout=60)
        self.buffer = ''
        self.serial = 0

    def request(self, module, command, **fields):
        self.serial += 1
        tag = f'airy-test-{self.serial}'
        self.ws.send(json.dumps(dict(module=module, cmd=command, requestId=tag, **fields)) + 'EOJ')
        deadline = time.monotonic() + 60
        while time.monotonic() < deadline:
            self.buffer = self.buffer.lstrip()
            if self.buffer.startswith('EOJ'):
                self.buffer = self.buffer[3:]
                continue
            try:
                reply, end = json.JSONDecoder().raw_decode(self.buffer)
            except ValueError:
                self.buffer += self.ws.receive()
                continue
            self.buffer = self.buffer[end:]
            if reply.get('requestId') == tag and reply.get('module') == module:
                assert reply.get('bSuccess'), reply
                return reply
        raise TimeoutError((module, command))


class Replay:
    def __init__(self, path, ports):
        self.path, self.ports = path, ports
        self.stop = threading.Event()
        self.failure = None
        self.thread = threading.Thread(target=self.run, daemon=True)
        self.thread.start()

    def run(self):
        try:
            with self.path.open('rb') as source, socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
                start, first = time.monotonic(), None
                while not self.stop.is_set():
                    header = source.read(12)
                    if not header:
                        break
                    assert len(header) == 12, 'Truncated capture header'
                    stamp, port, size = struct.unpack('<QHH', header)
                    data = source.read(size)
                    assert len(data) == size, 'Truncated capture payload'
                    if first is None:
                        first = stamp
                    delay = (stamp - first) / 1e9 - (time.monotonic() - start)
                    if delay > 0 and self.stop.wait(delay):
                        break
                    if port in self.ports:
                        sock.sendto(data, ('127.0.0.1', self.ports[port]))
        except Exception as error:
            self.failure = error

    def close(self):
        self.stop.set()
        self.thread.join(timeout=3)
        assert not self.thread.is_alive(), 'Replay did not stop'
        if self.failure:
            raise self.failure


def udp_port():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.bind(('127.0.0.1', 0))
        return sock.getsockname()[1]


def map_stream(port, expected):
    ws = helpers.WebSocket(f'ws://127.0.0.1:{port}/stream/glim', timeout=30)
    try:
        hello = json.loads(ws.receive())
        assert hello['protocol'] == 'openkai.glim' and hello['version'] == 3, hello
        ws.send('start')
        count = 0
        deadline = time.monotonic() + 30
        while count < expected and time.monotonic() < deadline:
            frame = ws.receive()
            if isinstance(frame, bytes):
                assert struct.unpack_from('<4I', frame) == (0x334d4c47, 3, 1, 56)
                total, offset, chunk = struct.unpack_from('<3I', frame, 40)
                assert 0 < chunk <= 65536 and offset + chunk <= total
                assert len(frame) == 56 + chunk * 12
                assert all(math.isfinite(v) for xyz in struct.iter_unpack('<3f', frame[56:]) for v in xyz)
                count += chunk
            else:
                assert json.loads(frame)['type'] in ('reset', 'submap', 'pose')
            ws.send('next')
        assert count == expected, (count, expected)
        return count
    finally:
        ws.close()


def run(args, output):
    launch_name = 'GLIM_Airy.json' if args.glim else 'Airy.json'
    config = json.loads((ROOT / 'jsonCfg' / launch_name).read_text())
    names = ['viewer', 'wsServer'] + (['sensorViewer'] if args.glim else [])
    assigned = set()
    for name in names:
        port = helpers.free_port()
        while port in assigned:
            port = helpers.free_port()
        assigned.add(port)
        config[name].update(host='127.0.0.1', port=port)
    port, command_port = config['viewer']['port'], config['wsServer']['port']
    if args.glim:
        config['GLIM']['exportPath'] = str(output)
    replay_ports = {}
    if args.capture:
        for key in ('msopPort', 'difopPort', 'imuPort'):
            new = udp_port()
            while new in replay_ports.values():
                new = udp_port()
            replay_ports[config['Airy'][key]] = new
            config['Airy'][key] = new
        config['Airy'].update(deviceIP='127.0.0.1', bindIP='127.0.0.1')
    launch = output / 'launch.json'
    launch.write_text(json.dumps(config, indent=2) + '\n')
    result = {'mode': 'capture replay' if args.capture else 'live sensor', 'glim': args.glim}
    commands = points = replay = None
    with (output / 'backend.log').open('w') as log:
        process = subprocess.Popen([str(Path(args.binary).resolve()), str(launch)], cwd=ROOT, stdout=log, stderr=log)
        try:
            page = helpers.wait_http(f'http://127.0.0.1:{port}/', process)
            assert (b'GLIM' if args.glim else b'Airy') in page
            if args.glim:
                assert b'Airy' in helpers.wait_http(f"http://127.0.0.1:{config['sensorViewer']['port']}/", process)
            commands = Commands(command_port)
            initial = commands.request('Airy', 'getConfig')
            result['initialStatus'] = initial['status']
            if args.glim:
                commands.request('GLIM', 'getConfig')
                commands.request('GLIM', 'start')
            else:
                points = helpers.WebSocket(f'ws://127.0.0.1:{port}/stream/points', timeout=30)
                hello = json.loads(points.receive())
                assert hello['version'] == 6 and hello['stream'] == 'points', hello
                points.send('start')
            if args.capture:
                replay = Replay(args.capture, replay_ports)
            start = time.monotonic()
            counts, bounds = [], [[math.inf, -math.inf] for _ in range(3)]
            while time.monotonic() - start < args.seconds:
                if replay and replay.failure:
                    raise replay.failure
                assert process.poll() is None, f'OpenKAI exited during acquisition: {process.returncode}'
                if args.glim:
                    status = commands.request('GLIM', 'getStatus')['status']
                    assert not status['error'], status
                    time.sleep(.5)
                else:
                    # A finite capture can end before --seconds. Wait for the
                    # next frame in bounded intervals without interrupting the
                    # WebSocket parser in the middle of a partially read frame.
                    if not points.buffer and not select.select([points.sock], [], [], 1)[0]:
                        continue
                    frame = points.receive()
                    points.send('next')
                    if not isinstance(frame, bytes):
                        continue
                    cloud = helpers.inspect_cloud(frame)
                    if not cloud:
                        continue
                    counts.append(len(cloud))
                    for axis in range(3):
                        bounds[axis][0] = min(bounds[axis][0], min(p[axis] for p in cloud))
                        bounds[axis][1] = max(bounds[axis][1], max(p[axis] for p in cloud))
            final = commands.request('Airy', 'getConfig')
            (output / 'getConfig.json').write_text(json.dumps(final, indent=2) + '\n')
            result.update(seconds=time.monotonic() - start, sensor=final['status'])
            assert final['status']['pointPackets'] > initial['status']['pointPackets'], final
            assert final['status']['invalidPackets'] == initial['status']['invalidPackets'], final
            if args.glim:
                result['tracking'] = commands.request('GLIM', 'getStatus')['status']
                assert result['tracking']['frames'] >= 5 and result['tracking']['poseValid'], result['tracking']
                result['stopped'] = commands.request('GLIM', 'stop')['status']
                assert result['stopped']['submaps'] > 0, result['stopped']
                result['export'] = commands.request('GLIM', 'savePointCloud', path=str(output / 'map.ply'))
                assert result['export']['points'] > 0 and (output / 'map.ply').stat().st_size > 100
                result['streamedMapPoints'] = map_stream(port, result['stopped']['mapPoints'])
            else:
                assert counts, 'No nonempty point clouds'
                result.update(pointCountRange=[min(counts), max(counts)], xyzBoundsMeters=bounds)
            (output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')
            if args.browser:
                commands.ws.close()
                commands = None
                points.close()
                points = None
                command = [sys.executable, str(ROOT / 'html/viewer/_Airy/tests/browser-smoke.py'),
                           '--live-url', f'http://localhost:{port}/?cmdPort={command_port}',
                           '--screenshot', str(output / 'viewer.png')]
                browser = subprocess.run(command, cwd=ROOT, capture_output=True, text=True, timeout=90)
                (output / 'browser.log').write_text(browser.stdout + browser.stderr)
                assert browser.returncode == 0, browser.stdout + browser.stderr
                result['browser'] = 'passed'
        finally:
            # Cleanup every resource even if the replay failed or a WebSocket
            # close raises; retain the original assertion when one is in flight.
            original_error = sys.exc_info()[1]
            cleanup_errors = []
            for resource in (replay, points, commands.ws if commands else None):
                if resource:
                    try:
                        resource.close()
                    except Exception as error:
                        cleanup_errors.append(error)
            try:
                if process.poll() is None:
                    process.send_signal(signal.SIGINT)
                    try:
                        process.wait(timeout=30)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait()
                        cleanup_errors.append(AssertionError('OpenKAI did not stop after SIGINT'))
                if process.returncode != 0:
                    cleanup_errors.append(AssertionError(f'OpenKAI exit code {process.returncode}'))
            except Exception as error:
                cleanup_errors.append(error)
                if process.poll() is None:
                    process.kill()
                    process.wait()
            if cleanup_errors:
                message = '; '.join(str(error) for error in cleanup_errors)
                if original_error is not None:
                    print(f'Additional cleanup error: {message}', file=sys.stderr)
                else:
                    raise AssertionError(f'Cleanup failed: {message}') from cleanup_errors[0]
    result['shutdown'] = 'clean SIGINT'
    (output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', nargs='?', default='build/OpenKAI')
    source = parser.add_mutually_exclusive_group(required=True)
    source.add_argument('--live', action='store_true')
    source.add_argument('--capture', type=Path)
    parser.add_argument('--glim', action='store_true')
    parser.add_argument('--browser', action='store_true')
    parser.add_argument('--seconds', type=float, default=10)
    parser.add_argument('--output', type=Path)
    args = parser.parse_args()
    if not math.isfinite(args.seconds) or args.seconds <= 0 or args.glim and args.browser:
        parser.error('Use positive --seconds; --browser applies to the raw Airy viewer')
    if args.capture and not args.capture.is_file():
        parser.error('--capture must name an existing packet capture file')
    if args.output:
        args.output.mkdir(parents=True, exist_ok=True)
        result = run(args, args.output.resolve())
    else:
        with tempfile.TemporaryDirectory(prefix='openkai-airy-') as folder:
            try:
                result = run(args, Path(folder))
            except Exception:
                log = Path(folder) / 'backend.log'
                if log.exists():
                    print(log.read_text()[-8000:])
                raise
    print(json.dumps(result, indent=2))
    print('PASS: Airy acquisition, controls, viewer and clean shutdown' + ('; GLIM mapping/export' if args.glim else ''))


if __name__ == '__main__':
    main()
