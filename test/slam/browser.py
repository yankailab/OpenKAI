"""Real browser + WSconsole integration for GLIM_orbbec.json.

python3 test/slam/browser.py build/OpenKAI [--hardware]
Without --hardware the sensor is disabled; lifecycle and renderer checks need no USB.
Hardware mode requires poses and streamed map points from the attached Orbbec.
Uses temporary ports/configuration; saves a screenshot and backend log under /tmp.
"""
import base64
import json
import shutil
import signal
import socket
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'webViewer3D'))
from integration import WebSocket, request


def free_port():
    with socket.socket() as s:
        s.bind(('127.0.0.1', 0))
        return s.getsockname()[1]


def main():
    root = Path(__file__).resolve().parents[2]
    executable = str(Path(sys.argv[1]).resolve())
    hardware = '--hardware' in sys.argv
    chrome = shutil.which('google-chrome') or shutil.which('chromium')
    if not chrome: raise RuntimeError('Chromium is required')
    with tempfile.TemporaryDirectory(prefix='openkai-glim-browser-', ignore_cleanup_errors=True) as folder:
        tmp = Path(folder)
        config = json.loads((root / 'jsonCfg/GLIM_orbbec.json').read_text())
        for name in ['console', 'view', 'viewD', 'd2rgb']: config[name]['bON'] = False
        config['Orbbec']['bON'] = hardware
        config['Orbbec']['bLog'] = True
        config['GLIM']['bAutoStart'] = False
        config['GLIM']['bLog'] = True
        port, cmd_port = free_port(), free_port()
        config['viewer']['host'] = config['wsServer']['host'] = '127.0.0.1'
        config['viewer']['port'], config['wsServer']['port'] = port, cmd_port
        path = tmp / 'config.json'; path.write_text(json.dumps(config))
        log = (tmp / 'backend.log').open('w+')
        server = subprocess.Popen([executable, str(path)], cwd=root, stdout=log, stderr=log)
        browser = client = None
        exceptions = []
        try:
            deadline = time.monotonic() + 30
            while True:
                try:
                    assert request(port, '/')[0] == 200
                    break
                except OSError:
                    if server.poll() is not None or time.monotonic() > deadline:
                        log.seek(0); raise AssertionError(log.read())
                    time.sleep(.1)
            assert b'GLIM SLAM' in request(port, '/')[2]
            browser = subprocess.Popen([chrome, '--headless=new', '--no-sandbox', '--disable-dev-shm-usage',
                '--use-gl=angle', '--use-angle=swiftshader', '--enable-unsafe-swiftshader',
                '--no-first-run', '--no-default-browser-check', '--disable-background-networking',
                '--remote-debugging-port=0', f'--user-data-dir={tmp / "chrome"}', '--window-size=1440,1000', 'about:blank'],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            active = tmp / 'chrome/DevToolsActivePort'
            deadline = time.monotonic() + 15
            while not active.exists() and time.monotonic() < deadline: time.sleep(.05)
            debug_port = int(active.read_text().splitlines()[0])
            with urllib.request.urlopen(f'http://127.0.0.1:{debug_port}/json/list') as response:
                target = next(t for t in json.load(response) if t['type'] == 'page')
            client = WebSocket(debug_port, '/' + target['webSocketDebuggerUrl'].split('/', 3)[3])
            client.socket.settimeout(20)
            next_id = 0
            def command(method, params=None):
                nonlocal next_id
                next_id += 1
                client.send(json.dumps({'id': next_id, 'method': method, 'params': params or {}}))
                while True:
                    _, payload = client.receive(); message = json.loads(payload)
                    if message.get('method') == 'Runtime.exceptionThrown': exceptions.append(message)
                    if message.get('id') == next_id:
                        assert 'error' not in message, message
                        return message.get('result', {})
            def evaluate(expression):
                result = command('Runtime.evaluate', {'expression': expression, 'awaitPromise': True, 'returnByValue': True})
                assert 'exceptionDetails' not in result, result
                return result['result'].get('value')
            def wait_for(expression, seconds=20):
                end = time.monotonic() + seconds
                while time.monotonic() < end:
                    if evaluate(expression): return
                    if server.poll() is not None: break
                    time.sleep(.1)
                raise AssertionError(evaluate("({status: document.querySelector('#slamStatus')?.textContent, command: document.querySelector('#commandStatus')?.textContent, log: document.querySelector('#cmdState')?.value, latest: replies.slice(-1)})"))
            command('Runtime.enable'); command('Page.enable')
            command('Page.addScriptToEvaluateOnNewDocument', {'source': '''
              window.socketURLs = []; window.replies = [];
              const NativeWebSocket = window.WebSocket;
              window.WebSocket = class extends NativeWebSocket {
                constructor(url, ...args) { super(url, ...args); socketURLs.push(String(url)); }
              };
              window.addEventListener('slamcommand', e => { replies.push(e.detail); if (replies.length > 200) replies.shift(); });
            '''})
            command('Page.navigate', {'url': (root / 'html/viewer/_GLIM/index.html').as_uri()})
            wait_for("location.protocol === 'file:' && !!window.viewerEndpoint")
            evaluate(f"document.querySelector('#port').value = '{port}'; document.querySelector('#cmdPort').value = '{cmd_port}'; document.querySelector('#start').click()")
            wait_for("location.protocol === 'http:' && document.querySelector('#status')?.textContent === 'Connected'")
            wait_for("document.querySelector('#slamStatus').textContent === 'Stopped' && !document.querySelector('#slamStart').disabled")
            assert evaluate("document.querySelectorAll('#imuPanel, #imuGraphs, #cameraFields').length") == 0
            assert evaluate("document.querySelectorAll('#viewport canvas').length") == 1
            assert evaluate("socketURLs.filter(u => new URL(u).pathname.startsWith('/stream/')).map(u => new URL(u).pathname)") == ['/stream/points', '/stream/lines']
            # Check the real renderer's transform convention independently of sensor motion.
            assert evaluate('''(async () => {
              const {Viewer3D} = await import('./js/viewer3D.js');
              const div = document.createElement('div'); div.style.cssText = 'width:400px;height:300px'; document.body.append(div);
              const v = new Viewer3D(div);
              v.configureSensor(90, 60, 2, 'x');
              v.setSensorPose({poseValid:true, poseFresh:true, position:[1,2,3], orientation:[0,0,Math.SQRT1_2,Math.SQRT1_2]});
              v.render();
              const points = v.frustum.geometry.getAttribute('position');
              const valid = v.sensor.visible && v.sensor.position.toArray().join() === '1,2,3' && Math.abs(points.getX(1)-2)<1e-5;
              v.setSensorPose(null); v.render();
              const hidden = !v.sensor.visible;
              v.dispose(); div.remove(); return valid && hidden;
            })()''')
            evaluate("document.querySelector('#slamStart').click()")
            wait_for("replies.some(j => j.cmd === 'start' && j.bSuccess) && !document.querySelector('#slamStop').disabled")
            if hardware:
                wait_for("replies.some(j => j.status?.poseFresh && j.status.mapPoints > 0)", 60)
                wait_for("parseInt(document.querySelector('#stats').textContent.replaceAll(',', '')) > 0")
                before = evaluate("replies.findLast(j => j.status)?.status")
                time.sleep(3)
                after = evaluate("replies.findLast(j => j.status)?.status")
                elapsed = (after['poseTimestampUs'] - before['poseTimestampUs']) * 1e-6
                imu_rate = (after['imuSamples'] - before['imuSamples']) / elapsed
                frame_rate = (after['frames'] - before['frames']) / elapsed
                assert 150 < imu_rate < 250, (imu_rate, after)
                assert after['maxIMUgapUs'] < 15000, after
                assert frame_rate > 10, (frame_rate, after)
                assert evaluate("Number.parseFloat(document.querySelector('#frameRate').textContent) > 0")
                assert evaluate("Number.parseFloat(document.querySelector('#processingMs').textContent) > 0")
                print('Live status:', after, flush=True)
                print(f'Capture: {frame_rate:.1f} SLAM frames/s, {imu_rate:.1f} IMU pairs/s', flush=True)
            else:
                wait_for("document.querySelector('#slamStatus').textContent === 'Initializing'")
            screenshot = command('Page.captureScreenshot', {'format': 'png'})
            Path('/tmp/openkai-glim-viewer.png').write_bytes(base64.b64decode(screenshot['data']))
            evaluate("document.querySelector('#slamStop').click()")
            wait_for("replies.some(j => j.cmd === 'stop' && j.bSuccess) && document.querySelector('#slamStatus').textContent === 'Stopped'", 90)
            if hardware:
                assert evaluate("replies.findLast(j => j.cmd === 'stop').status.mapPoints > 0")
                assert evaluate("replies.findLast(j => j.cmd === 'stop').status.submaps > 0")
            evaluate("document.querySelector('#slamReset').click()")
            wait_for("replies.some(j => j.cmd === 'reset' && j.bSuccess && j.status.mapPoints === 0 && !j.status.poseValid)")
            wait_for("parseInt(document.querySelector('#stats').textContent.replaceAll(',', '')) === 0")
            assert evaluate("document.querySelector('#posX').textContent") == '—'
            # Parser must tolerate arbitrary transport fragments and literal EOJ in strings.
            evaluate('''(() => {
              const packet = JSON.stringify({cmd:'parserTest', module:'GLIM', note:'quoted } EOJ { text'});
              for (let i=0; i<packet.length; i+=7) cmdHandler({data:packet.slice(i,i+7)});
              cmdHandler({data:'E'}); cmdHandler({data:'OJ'});
            })()''')
            assert evaluate("replies.findLast(j => j.cmd === 'parserTest').note") == 'quoted } EOJ { text'
            evaluate("document.querySelector('#cmdDisconnect').click()")
            wait_for("document.querySelector('#slamStatus').textContent === 'Disconnected' && document.querySelector('#slamStart').disabled")
            evaluate("document.querySelector('#cmdConnect').click()")
            wait_for("document.querySelector('#slamStatus').textContent === 'Stopped' && !document.querySelector('#slamStart').disabled")
            evaluate("document.querySelector('#stop').click(); document.querySelector('#cmdDisconnect').click()")
            wait_for("document.querySelector('#status').textContent === 'Stopped'")
            assert not exceptions, exceptions
            print('PASS: GLIM browser, WSconsole lifecycle/status, fragmented replies, map reset, sensor/FoV renderer, reconnect')
            print('Camera: ' + ('live pose, map stream and completed submaps verified' if hardware else 'disabled for hardware-independent checks'))
        finally:
            if client: client.close()
            if browser:
                browser.terminate(); browser.wait(timeout=10)
            if server.poll() is None:
                server.send_signal(signal.SIGINT)
                try: server.wait(timeout=10)
                except subprocess.TimeoutExpired: server.kill(); server.wait()
            log.seek(0)
            Path('/tmp/openkai-glim-browser-backend.log').write_text(log.read())
            log.close()

if __name__ == '__main__': main()
