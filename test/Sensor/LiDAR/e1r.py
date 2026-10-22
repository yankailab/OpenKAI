#!/usr/bin/env python3
"""Exercise E1R UDP decoding, console controls and web geometry in OpenKAI.

python3 test/Sensor/LiDAR/e1r.py [build/OpenKAI] [--live] [--output /tmp/e1r-test]
The default uses synthetic UDP packets on loopback and a temporary launch file.
--live retains the E1R network settings from jsonCfg/E1R.json and only reads the
sensor; it never changes settings or sends a device command. Python stdlib only.
"""

import argparse
import importlib.util
import json
import math
from pathlib import Path
import signal
import socket
import struct
import subprocess
import sys
import tempfile
import threading
import time

ROOT = Path(__file__).resolve().parents[3]
spec = importlib.util.spec_from_file_location('openkai_sensor_live', ROOT / 'test/Sensor/LineScanner/live.py')
helpers = importlib.util.module_from_spec(spec)
spec.loader.exec_module(helpers)


class Commands:
    def __init__(self, port):
        self.ws = helpers.WebSocket(f'ws://127.0.0.1:{port}/', timeout=15)
        self.buffer = ''
        self.serial = 0

    def request(self, command, **fields):
        self.serial += 1
        request_id = f'e1r-test-{self.serial}'
        self.ws.send(json.dumps(dict(module='E1R', cmd=command, requestId=request_id, **fields)) + 'EOJ')
        deadline = time.monotonic() + 20
        while time.monotonic() < deadline:
            self.buffer = self.buffer.lstrip()
            if self.buffer.startswith('EOJ'):
                self.buffer = self.buffer[3:]
                continue
            try:
                reply, end = json.JSONDecoder().raw_decode(self.buffer)
            except ValueError:
                data = self.ws.receive()
                assert isinstance(data, str)
                self.buffer += data
                continue
            self.buffer = self.buffer[end:]
            if reply.get('requestId') == request_id and reply.get('module') == 'E1R':
                assert reply.get('cmd') == command, reply
                return reply
        raise TimeoutError(command)


def eventually(action, predicate, timeout=15):
    deadline = time.monotonic() + timeout
    last = None
    while time.monotonic() < deadline:
        last = action()
        if predicate(last):
            return last
        time.sleep(.1)
    raise AssertionError(f'Timed out waiting for expected response: {last}')


def success(reply):
    assert reply.get('bSuccess'), reply
    return reply


def free_udp_port():
    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
        sock.bind(('127.0.0.1', 0))
        return sock.getsockname()[1]


def timestamp(packet, offset, micros):
    seconds, part = divmod(micros, 1_000_000)
    packet[offset:offset + 6] = seconds.to_bytes(6, 'big')
    struct.pack_into('>I', packet, offset + 6, part)


def msop(sequence, micros):
    packet = bytearray(1200)
    packet[:4] = bytes.fromhex('55aa5aa5')
    struct.pack_into('>HHBB', packet, 4, sequence, 1, 4, 0)
    timestamp(packet, 10, micros)
    packet[30:32] = bytes([0x62, 105])
    for point in range(96):
        # Mix live firmware's unclassified 0 and manual's normal 1 attributes.
        # 93 valid points, one noisy, one zero distance, one beyond 120 m.
        radius = 0 if point == 2 else 30000 if point == 3 else 200
        attribute = 2 if point == 1 else point % 2
        struct.pack_into('>HHhhhBB', packet, 32 + point * 12,
                         point, radius, 32767, 0, 0, 128, attribute)
    struct.pack_into('>HHI', packet, 1188, 1200, sequence, 0xE5C)
    return packet


def difop(msop_port, difop_port, micros):
    packet = bytearray(256)
    packet[:8] = bytes.fromhex('a5ff005a11115555')
    packet[16:19] = bytes([1, 2, 3])
    packet[20:26] = bytes.fromhex('0123456789ab')
    packet[44:48] = socket.inet_aton('127.0.0.1')
    packet[48:52] = socket.inet_aton('255.0.0.0')
    packet[52:58] = bytes.fromhex('020000000001')
    packet[58:62] = socket.inet_aton('127.0.0.1')
    struct.pack_into('>HH', packet, 62, 6699, msop_port)
    packet[66:70] = socket.inet_aton('127.0.0.1')
    struct.pack_into('>HH', packet, 70, 7788, difop_port)
    packet[99:103] = bytes([10, 4, 0, 1])
    timestamp(packet, 103, micros)
    packet[113] = 0
    return packet


class Replay:
    def __init__(self, msop_port, difop_port):
        self.ports = (msop_port, difop_port)
        self.stop_event = threading.Event()
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.sock.bind(('127.0.0.1', 0))
        self.failure = None
        self.worker = threading.Thread(target=self.run, daemon=True)
        self.worker.start()

    def run(self):
        try:
            while not self.stop_event.is_set():
                msop_port, difop_port = self.ports
                micros = time.time_ns() // 1000
                self.sock.sendto(difop(msop_port, difop_port, micros), ('127.0.0.1', difop_port))
                for sequence in range(3):
                    self.sock.sendto(msop(sequence, micros + sequence * 1000), ('127.0.0.1', msop_port))
                    if self.stop_event.wait(.005):
                        return
                self.stop_event.wait(.085)
        except Exception as error:
            self.failure = error

    def close(self):
        self.stop_event.set()
        self.worker.join(timeout=2)
        self.sock.close()
        assert not self.worker.is_alive(), 'UDP replay did not stop'
        if self.failure:
            raise self.failure


def receive_cloud(points, predicate=lambda cloud: bool(cloud), timeout=15):
    deadline = time.monotonic() + timeout
    cloud = []
    while time.monotonic() < deadline:
        frame = points.receive()
        if not isinstance(frame, bytes):
            continue
        cloud = helpers.inspect_cloud(frame)
        points.send('next')
        if predicate(cloud):
            return cloud
    raise AssertionError(f'No matching point cloud; last count={len(cloud)}, sample={cloud[:3]}')


def run(args, output):
    config = json.loads((ROOT / 'jsonCfg/E1R.json').read_text())
    port, command_port = helpers.free_port(), helpers.free_port()
    while command_port == port:
        command_port = helpers.free_port()
    config['APP']['bStdErr'] = True
    config['viewer'].update(host='127.0.0.1', port=port)
    config['wsServer'].update(host='127.0.0.1', port=command_port)
    if not args.live:
        msop_port, difop_port = free_udp_port(), free_udp_port()
        while difop_port == msop_port:
            difop_port = free_udp_port()
        config['E1R'].update(deviceIP='127.0.0.1', bindIP='127.0.0.1',
                             msopPort=msop_port, difopPort=difop_port,
                             bUseSensorTime=False, bDropNoise=True, vRangeD=[.1, 120],
                             vPos=[0, 0, 0], vOrt=[0, 0, 0, 1])
    launch = output / 'launch.json'
    launch.write_text(json.dumps(config, indent=2))
    result = {'mode': 'live (read-only)' if args.live else 'synthetic UDP'}
    commands = points = replay = None
    with (output / 'backend.log').open('w') as log:
        process = subprocess.Popen([str(Path(args.binary).resolve()), str(launch)], cwd=ROOT,
                                   stdout=log, stderr=log)
        try:
            page = helpers.wait_http(f'http://127.0.0.1:{port}/', process)
            assert b'E1R viewer' in page, 'E1R viewer was not served'
            commands = Commands(command_port)
            initial = success(commands.request('getConfig'))
            if not args.live:
                replay = Replay(msop_port, difop_port)
            reply = eventually(lambda: success(commands.request('getConfig')),
                               lambda r: r.get('status', {}).get('pointPackets', 0) >= 6 and
                               r.get('status', {}).get('difopPackets', 0) > 0, timeout=args.timeout)
            assert reply['deviceOpen'], reply
            (output / 'getConfig.json').write_text(json.dumps(reply, indent=2))
            result['initialStatus'] = reply['status']
            schema = reply['schema']
            schema = {item['key']: item for item in schema} if isinstance(schema, list) else schema
            assert set(initial['config']).issubset(schema), 'Every config field needs a viewer control'
            read_only = [key for key, field in schema.items() if field.get('readOnly')]
            assert read_only, 'Device telemetry must be exposed read-only'
            result['schemaFields'] = len(schema)
            points = helpers.WebSocket(f'ws://127.0.0.1:{port}/stream/points', timeout=args.timeout)
            hello = json.loads(points.receive())
            assert hello['version'] == 6 and hello['stream'] == 'points', hello
            points.send('start')
            expected_x = 32767 / 32768
            cloud = receive_cloud(points, (lambda cloud: bool(cloud)) if args.live else
                                  (lambda cloud: len(cloud) == 279), timeout=args.timeout)
            if args.live:
                assert any(abs(v) > .1 for point in cloud for v in point), cloud[:3]
                counts = [len(cloud)]
                start = time.monotonic()
                before = success(commands.request('getConfig'))['status']
                bounds = [[min(p[axis] for p in cloud), max(p[axis] for p in cloud)] for axis in range(3)]
                while len(counts) < 4 or time.monotonic() - start < args.seconds:
                    cloud = receive_cloud(points, timeout=args.timeout)
                    counts.append(len(cloud))
                    for axis in range(3):
                        bounds[axis][0] = min(bounds[axis][0], min(p[axis] for p in cloud))
                        bounds[axis][1] = max(bounds[axis][1], max(p[axis] for p in cloud))
                result['cloudCounts'] = counts
                elapsed = time.monotonic() - start
                after = success(commands.request('getConfig'))['status']
                counters = ['pointPackets', 'difopPackets', 'frames', 'points', 'partialFrames',
                            'droppedPackets', 'invalidPackets', 'outOfOrderPackets', 'frameOverflows']
                delta = {key: after[key] - before[key] for key in counters}
                result['stream'] = {'seconds': elapsed, 'counterDelta': delta,
                                   'sensorFramesPerSecond': delta['frames'] / elapsed,
                                   'pointCountRange': [min(counts), max(counts)], 'xyzBoundsMeters': bounds}
                assert delta['frames'] > 0 and delta['points'] > 0, result['stream']
                assert delta['invalidPackets'] == 0 and delta['frameOverflows'] == 0, result['stream']
            else:
                assert all(math.isclose(p[0], expected_x, abs_tol=1e-6) and p[1] == p[2] == 0
                           for p in cloud), cloud[:3]
                result['decodedCloud'] = {'points': len(cloud), 'firstPoint': cloud[0]}
                original = reply['config']
                for patch in [{'msopPort': 0}, {'bDropNoise': 'true'}, {'vRangeD': [120, 1]},
                              {'vColorDefault': [1, 2, 0]}, {'vOrt': [0, 0, 0, 0]},
                              {'unexpectedSetting': 1}, {'bReflectivity': False, 'msopPort': 0}]:
                    rejected = commands.request('setConfig', config=patch)
                    assert not rejected.get('bSuccess'), rejected
                    unchanged = success(commands.request('getConfig'))['config']
                    assert unchanged == original, ('Invalid patch was partially applied', patch, unchanged)
                hardware_key = read_only[0]
                rejected = commands.request('setConfig', config={hardware_key: 'read-only-test'})
                assert not rejected.get('bSuccess'), rejected
                with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as occupied:
                    occupied.bind(('127.0.0.1', 0))
                    rejected = commands.request('setConfig', config={
                        'bReflectivity': False, 'difopPort': occupied.getsockname()[1]})
                    assert not rejected.get('bSuccess'), rejected
                    assert success(commands.request('getConfig'))['config'] == original, 'Failed bind changed config'
                prior = success(commands.request('getConfig'))['status']['pointPackets']
                eventually(lambda: success(commands.request('getConfig')),
                           lambda r: r['status']['pointPackets'] > prior + 3)
                before = success(commands.request('getConfig'))['status']['invalidPackets']
                replay.sock.sendto(b'invalid-e1r', ('127.0.0.1', msop_port))
                bad_type = msop(7, time.time_ns() // 1000)
                bad_type[30] = 0
                replay.sock.sendto(bad_type, ('127.0.0.1', msop_port))
                eventually(lambda: success(commands.request('getConfig')),
                           lambda r: r['status']['invalidPackets'] >= before + 2)
                patched = success(commands.request('setConfig', config={
                    'bReflectivity': False, 'vColorDefault': [1, .5, 0], 'vPos': [1, 2, 3]}))
                assert patched['config']['vPos'] == [1, 2, 3], patched
                transformed = receive_cloud(points, lambda cloud: len(cloud) == 279 and
                                            all(math.isclose(p[0], expected_x + 1, abs_tol=1e-6)
                                                and p[1] == 2 and p[2] == 3 for p in cloud))
                result['transformedPoint'] = transformed[0]
                success(commands.request('saveConfig'))
                saved = json.loads(launch.read_text())
                assert saved['E1R']['bReflectivity'] is False
                assert saved['E1R']['vColorDefault'] == [1, .5, 0] and saved['E1R']['vPos'] == [1, 2, 3]
                assert saved['viewer'] == config['viewer'] and saved['wsServer'] == config['wsServer']
                assert saved['E1R']['PCLframeOut'] == config['E1R']['PCLframeOut']
                new_msop, new_difop = free_udp_port(), free_udp_port()
                while len({new_msop, new_difop, msop_port, difop_port}) != 4:
                    new_msop, new_difop = free_udp_port(), free_udp_port()
                before = success(commands.request('getConfig'))['status']['pointPackets']
                success(commands.request('setConfig', config={'msopPort': new_msop, 'difopPort': new_difop}))
                replay.ports = (new_msop, new_difop)
                eventually(lambda: success(commands.request('getConfig')),
                           lambda r: r['status']['pointPackets'] > before + 6)
                # A successful rebind must release both previous receiver ports.
                for old_port in (msop_port, difop_port):
                    with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as probe:
                        probe.bind(('127.0.0.1', old_port))
                receive_cloud(points, lambda cloud: len(cloud) == 279)
                result['checks'] = ['MSOP coordinates, distance/noise filtering and frame boundaries',
                                    'DIFOP telemetry', 'invalid packet rejection', 'atomic invalid patches and bind failure',
                                    'read-only telemetry', 'pose transform', 'temporary launch save',
                                    'UDP socket rebind and old-port release']
            result['finalStatus'] = success(commands.request('getConfig'))['status']
            # Keep the sensor measurements even if the optional browser check fails.
            (output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')
            if args.browser:
                # wsMode=2 serves the first command client. Hand the connection
                # over to Chrome after the protocol measurements are complete.
                commands.ws.close()
                commands = None
                points.close()
                points = None
                browser_command = [sys.executable, str(ROOT / 'html/viewer/_E1R/tests/browser-smoke.py'),
                                   '--live-url', f'http://localhost:{port}/?cmdPort={command_port}',
                                   '--screenshot', str(output / 'viewer.png')]
                browser = subprocess.run(browser_command, cwd=ROOT, text=True, capture_output=True, timeout=90)
                (output / 'browser.log').write_text(browser.stdout + browser.stderr)
                assert browser.returncode == 0, browser.stdout + browser.stderr
                result['browser'] = 'PASS: live controls, geometry, reconnect and responsive layout'
        finally:
            if replay:
                replay.close()
            if points:
                points.close()
            if commands:
                commands.ws.close()
            if process.poll() is None:
                process.send_signal(signal.SIGINT)
                try:
                    process.wait(timeout=20)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
                    raise AssertionError('OpenKAI did not stop within 20 seconds after SIGINT')
            assert process.returncode == 0, f'OpenKAI exited with {process.returncode}'
    result['shutdown'] = 'clean SIGINT'
    return result


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', nargs='?', default='build/OpenKAI')
    parser.add_argument('--live', action='store_true', help='Read the real sensor instead of replaying UDP')
    parser.add_argument('--timeout', type=float, default=20)
    parser.add_argument('--seconds', type=float, default=10, help='Duration of the live stream measurement')
    parser.add_argument('--browser', action='store_true', help='Also verify the served viewer with Chrome/Chromium')
    parser.add_argument('--output', type=Path, help='Keep launch, log, getConfig reply and results here')
    args = parser.parse_args()
    if args.output:
        args.output.mkdir(parents=True, exist_ok=True)
        result = run(args, args.output.resolve())
        (args.output / 'results.json').write_text(json.dumps(result, indent=2) + '\n')
    else:
        with tempfile.TemporaryDirectory(prefix='openkai-e1r-test-') as folder:
            try:
                result = run(args, Path(folder))
            except Exception:
                log = Path(folder) / 'backend.log'
                if log.exists():
                    print(log.read_text()[-8000:])
                raise
    print(json.dumps(result, indent=2))
    print('PASS: E1R UDP, console, web geometry and shutdown')


if __name__ == '__main__':
    main()
