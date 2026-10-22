#!/usr/bin/env python3
"""Exercise a connected Mid360 through the actual OpenKAI web protocols.

python3 test/Sensor/LiDAR/live.py [build/OpenKAI] [--glim] [--seconds 20]
Uses a temporary launch file, tests controls and save without changing the
checked-in configuration, restores edited settings, and shuts down with SIGINT.
Requires the Ethernet sensor configured in jsonCfg/Livox2.json. The --glim mode
also initializes LiDAR-inertial odometry and checks that poses are produced.
"""
import argparse
import importlib.util
import json
import math
from pathlib import Path
import signal
import subprocess
import tempfile
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

    def request(self, command, module='Lvx1', **fields):
        self.serial += 1
        request_id = f'livox-live-{self.serial}'
        self.ws.send(json.dumps(dict(module=module, cmd=command, requestId=request_id, **fields)) + 'EOJ')
        deadline = time.monotonic() + 30
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
            if reply.get('requestId') == request_id and reply.get('module') == module:
                return reply
        raise TimeoutError(command)


def eventually(action, predicate, timeout=20):
    deadline = time.monotonic() + timeout
    last = None
    while time.monotonic() < deadline:
        last = action()
        if predicate(last):
            return last
        time.sleep(.15)
    raise AssertionError(f'Timed out waiting for valid response: {last}')


def success(reply):
    assert reply.get('bSuccess'), reply
    return reply


def run(args, temp):
    config = json.loads((ROOT / 'jsonCfg' / ('GLIM_livox2.json' if args.glim else 'Livox2.json')).read_text())
    port, cmd_port = helpers.free_port(), helpers.free_port()
    config['APP']['bStdErr'] = True
    config['wsServer'].update(host='127.0.0.1', port=cmd_port)
    config['viewer'].update(host='127.0.0.1', port=port)
    if args.glim:
        config['sensorViewer'].update(host='127.0.0.1', port=helpers.free_port())
    launch = temp / 'launch.json'
    launch.write_text(json.dumps(config, indent=2))
    result = {}
    commands, points = None, None
    original = None
    with (temp / 'backend.log').open('w') as log:
        process = subprocess.Popen([str(Path(args.binary).resolve()), str(launch)], cwd=ROOT, stdout=log, stderr=log)
        try:
            page = helpers.wait_http(f'http://127.0.0.1:{port}/', process)
            assert b'<html' in page.lower()
            commands = Commands(cmd_port)
            reply = eventually(lambda: commands.request('getConfig'), lambda r: r.get('deviceOpen') and r.get('bSuccess'))
            original = reply['config']
            eventually(lambda: commands.request('getConfig'), lambda r: r.get('status', {}).get('configApplied'))
            result['device'] = reply
            imu = eventually(lambda: commands.request('getIMU'), lambda r: r.get('available'))
            assert imu['enabled'] and imu['deviceOpen'], imu
            assert all(math.isfinite(x) for x in imu['gyro'] + imu['acc']), imu
            acceleration = math.sqrt(sum(x*x for x in imu['acc']))
            assert 7 < acceleration < 12, f'Expected stationary gravity in m/s², got {acceleration}: {imu}'
            assert int(imu['tAcc']) > 0 and int(imu['tGyro']) > 0, imu
            result['imu'] = imu
            geometry_port = config['sensorViewer']['port'] if args.glim else port
            points = helpers.WebSocket(f'ws://127.0.0.1:{geometry_port}/stream/points', timeout=15)
            hello = json.loads(points.receive())
            assert hello['version'] == 6 and hello['stream'] == 'points', hello
            points.send('start')
            clouds = []
            start = time.monotonic()
            while len(clouds) < 10:
                frame = points.receive()
                if not isinstance(frame, bytes):
                    continue
                cloud = helpers.inspect_cloud(frame)
                if cloud:
                    clouds.append(len(cloud))
                    assert any(abs(v) > .1 for p in cloud for v in p)
                points.send('next')
            result['cloud'] = dict(frames=len(clouds), counts=clouds, seconds=time.monotonic()-start)
            points.close()
            points = None
            for invalid in [{'frameDurationMs': 0}, {'lvxIMUdataEn': 7}, {'unexpectedSetting': 1}]:
                rejected = commands.request('setConfig', config=invalid)
                assert not rejected.get('bSuccess'), rejected
            changed = 50 if original['frameDurationMs'] != 50 else 100
            patched = success(commands.request('setConfig', config={'frameDurationMs': changed}))
            assert patched['config']['frameDurationMs'] == changed, patched
            saved = success(commands.request('saveConfig'))
            on_disk = json.loads(launch.read_text())
            assert on_disk['Lvx1']['frameDurationMs'] == changed, on_disk['Lvx1']
            assert on_disk['wsServer']['port'] == cmd_port
            success(commands.request('setConfig', config={'frameDurationMs': original['frameDurationMs']}))
            success(commands.request('setConfig', config={'lvxIMUdataEn': 0}))
            eventually(lambda: commands.request('getIMU'), lambda r: not r.get('enabled') and not r.get('available'))
            success(commands.request('setConfig', config={'lvxIMUdataEn': original['lvxIMUdataEn']}))
            eventually(lambda: commands.request('getIMU'), lambda r: r.get('available'))
            result['controls'] = ['invalid patches rejected', 'frame duration', 'save temporary launch', 'IMU off/on']
            for data_type in [2, 3, 1]:
                success(commands.request('setConfig', config={'lvxPCLdataType': data_type}))
                applied = eventually(lambda: commands.request('getConfig'),
                                     lambda r: r.get('status', {}).get('configApplied') and
                                     r.get('status', {}).get('observed', {}).get('lvxPCLdataType') == data_type)
                count_before = applied['status']['frames']
                eventually(lambda: commands.request('getConfig'),
                           lambda r: r.get('status', {}).get('frames', 0) >= count_before + 3)
                points = helpers.WebSocket(f'ws://127.0.0.1:{geometry_port}/stream/points', timeout=15)
                assert json.loads(points.receive())['version'] == 6
                points.send('start')
                cloud = helpers.inspect_cloud(points.receive())
                assert len(cloud) > 100, (data_type, len(cloud))
                points.close()
                points = None
            result['controls'].append('Cartesian high/low and spherical point formats')
            if args.glim:
                success(commands.request('start', module='GLIM'))
                tracking_start = time.monotonic()
                state = eventually(lambda: commands.request('getStatus', module='GLIM'),
                                   lambda r: r.get('status', {}).get('poseValid') and r['status'].get('frames', 0) >= 15,
                                   timeout=max(40, args.seconds))
                while time.monotonic() - tracking_start < args.seconds:
                    assert not state['status']['error'], state
                    time.sleep(.5)
                    state = success(commands.request('getStatus', module='GLIM'))
                assert not state['status']['error'] and state['status']['imuSamples'] > 0, state
                assert all(math.isfinite(v) for v in state['status']['position']), state
                result['slam'] = state['status']
                result['trackingSeconds'] = time.monotonic() - tracking_start
                success(commands.request('stop', module='GLIM'))
                exported = success(commands.request('savePointCloud', module='GLIM', path=str(temp / 'stationary-map.ply')))
                assert exported['points'] > 100, exported
                assert (temp / 'stationary-map.ply').stat().st_size > 1000
                result['export'] = {'points': exported['points']}
            else:
                time.sleep(max(0, args.seconds - (time.monotonic()-start)))
                result['finalDevice'] = success(commands.request('getConfig'))
            return result
        finally:
            if points:
                points.close()
            if commands:
                try:
                    if original:
                        commands.request('setConfig', config={k: original[k] for k in ['frameDurationMs', 'lvxIMUdataEn', 'lvxPCLdataType']})
                    if args.glim:
                        commands.request('stop', module='GLIM')
                except (OSError, EOFError, TimeoutError) as error:
                    print(f'Cleanup command failed: {error}')
                finally:
                    commands.ws.close()
            if process.poll() is None:
                process.send_signal(signal.SIGINT)
                try:
                    process.wait(timeout=30)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()
                    raise AssertionError('OpenKAI did not shut down after SIGINT')
            assert process.returncode == 0, f'OpenKAI exited with {process.returncode}'


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary', nargs='?', default='build/OpenKAI')
    parser.add_argument('--glim', action='store_true')
    parser.add_argument('--seconds', type=int, default=20)
    parser.add_argument('--output', type=Path, help='Optional directory for launch file, backend log and results')
    args = parser.parse_args()
    if args.output:
        args.output.mkdir(parents=True, exist_ok=True)
        result = run(args, args.output.resolve())
        (args.output / 'results.json').write_text(json.dumps(result, indent=2)+'\n')
    else:
        with tempfile.TemporaryDirectory(prefix='openkai-livox-live-') as folder:
            try:
                result = run(args, Path(folder))
            except Exception:
                print((Path(folder)/'backend.log').read_text()[-8000:])
                raise
    print(json.dumps(result, indent=2))


if __name__ == '__main__':
    main()
