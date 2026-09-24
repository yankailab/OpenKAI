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
        config['GLIM']['fConfig'] = str(tmp / 'glim.controls.json')
        config['GLIM']['exportPath'] = str(tmp / 'exports')
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
              window.socketURLs = []; window.replies = []; window.streamEvents = [];
              const NativeWebSocket = window.WebSocket;
              window.WebSocket = class extends NativeWebSocket {
                constructor(url, ...args) {
                  super(url, ...args); socketURLs.push(String(url));
                  if (new URL(url).pathname === '/stream/glim') this.addEventListener('message', e => {
                    if (typeof e.data === 'string') streamEvents.push(JSON.parse(e.data));
                    else { const v = new DataView(e.data); streamEvents.push({type:'chunk', id:v.getBigUint64(24,true).toString(), offset:v.getUint32(44,true), count:v.getUint32(48,true)}); }
                    if (streamEvents.length > 1000) streamEvents.shift();
                  });
                }
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
            assert evaluate("socketURLs.filter(u => new URL(u).pathname.startsWith('/stream/')).map(u => new URL(u).pathname)") == ['/stream/glim']
            wait_for("document.querySelector('#param-nMinPoints') && !document.querySelector('#parameterFields').disabled")
            original_minimum = evaluate("Number(document.querySelector('#param-nMinPoints').value)")
            evaluate(f"document.querySelector('#param-nMinPoints').value = {original_minimum + 1}; document.querySelector('#param-nMinPoints').dispatchEvent(new Event('input', {{bubbles:true}}))")
            time.sleep(.7)
            assert evaluate("Number(document.querySelector('#param-nMinPoints').value)") == original_minimum + 1
            evaluate("document.querySelector('#saveParameters').click()")
            wait_for("replies.some(j => j.cmd === 'saveConfig' && j.bSuccess)")
            assert (tmp / 'glim.controls.json').exists()
            assert evaluate("replies.findLast(j => j.cmd === 'saveConfig').config.nMinPoints") == original_minimum + 1
            # Start must apply pending edits before it starts the estimator.
            evaluate(f"document.querySelector('#param-nMinPoints').value = {original_minimum}; document.querySelector('#param-nMinPoints').dispatchEvent(new Event('input', {{bubbles:true}}))")
            # Check real rendering, chunk validation, accumulation and graph corrections.

            assert evaluate('''(async () => {
              const {Viewer3D} = await import('./js/viewer3D.js');
              const {decodeChunk, decodeEvent} = await import('./js/protocol.js');
              const div = document.createElement('div'); div.style.cssText = 'width:400px;height:300px'; document.body.append(div);
              const v = new Viewer3D(div);
              const identity = [1,0,0,0,0,1,0,0,0,0,1,0,0,0,0,1];
              const check = (condition, reason) => { if (!condition) throw new Error(reason); };
              v.update(decodeEvent(JSON.stringify({type:'reset', session:'7', revision:'1'})));
              for (const id of ['10','11']) {
                v.update(decodeEvent(JSON.stringify({type:'submap', session:'7', revision:'2', id, timestampUs:'100', pointCount:2, pose:identity})));
                for (let offset=0; offset<2; ++offset) {
                  const bytes = new ArrayBuffer(68), d = new DataView(bytes);
                  [0x314d4c47,1,1,56].forEach((x,i) => d.setUint32(i*4,x,true));
                  d.setBigUint64(16,7n,true); d.setBigUint64(24,BigInt(id),true); d.setBigUint64(32,100n,true);
                  d.setUint32(40,2,true); d.setUint32(44,offset,true); d.setUint32(48,1,true);
                  [offset,offset,offset].forEach((x,i) => d.setFloat32(56+i*4,x,true));
                  v.update(decodeChunk(bytes));
                }
              }
              check(v.objects.size === 2 && v.pointCount === 4, 'Submaps did not accumulate');
              const geometry = v.objects.get('10').points.geometry;
              const moved = identity.slice(); moved[12] = 20;
              v.update({type:'pose', session:'7', id:'10', pose:moved}); v.render();
              check(v.objects.get('10').points.geometry === geometry && v.bounds.max.x === 21, 'Correction reuploaded geometry or missed bounds');
              check(v.objects.get('10').points.matrixWorld.elements[12] === 20, 'Correction missed renderer transform');
              const invalid = new ArrayBuffer(56);
              let rejected = false; try { decodeChunk(invalid); } catch { rejected = true; }
              check(rejected, 'Accepted unrelated point/line protocol');
              v.configureSensor(90, 60, 2, 'x');
              const status = {session:'7', poseTimestampUs:1000, poseValid:true, poseFresh:true, position:[1,2,3], orientation:[0,0,Math.SQRT1_2,Math.SQRT1_2]};
              v.setSensorPose(status); v.render();
              const points = v.frustum.geometry.getAttribute('position');
              check(v.sensor.visible && v.sensor.position.toArray().join() === '1,2,3' && Math.abs(points.getX(1)-2)<1e-5, 'Sensor / FoV transform');
              v.setSensorPose({...status, poseTimestampUs:2000, position:[2,2,3]});
              v.setSensorPose({...status, poseTimestampUs:2000, position:[2,2,3]});
              check(v.trajectoryCount === 1, 'Trajectory duplicates an old pose');
              v.markSensorStale();
              v.setSensorPose({...status, poseTimestampUs:2500, position:[20,2,3]});
              check(v.trajectoryCount === 1, 'Trajectory joined across missing telemetry');
              for(let i=0;i<v.trajectoryCapacity+10;++i) v.setSensorPose({...status, poseTimestampUs:3000+i, position:[i,2,3]});
              check(v.trajectoryCount === v.trajectoryCapacity, 'Trajectory is unbounded');
              v.setSensorPose({session:'8', poseValid:false}); v.render();
              check(v.trajectoryCount === 0 && !v.sensor.visible, 'Session reset retained trajectory');
              v.update({type:'reset',session:'8',revision:'1'});
              check(v.pointCount === 0 && v.objects.size === 0, 'Map reset retained geometry');
              v.dispose(); div.remove(); return true;
            })()''')
            evaluate("document.querySelector('#slamStart').click()")
            wait_for("replies.some(j => j.cmd === 'start' && j.bSuccess) && !document.querySelector('#slamStop').disabled")
            assert evaluate("replies.findLast(j => j.cmd === 'setConfig').config.nMinPoints") == original_minimum
            assert evaluate("document.querySelector('#parameterFields').disabled && document.querySelector('#saveParameters').disabled")
            evaluate("wsSendCmd({cmd:'setConfig',module:'GLIM',requestId:'running-config-test',config:{nMinPoints:123}})")
            wait_for("replies.some(j => j.requestId === 'running-config-test' && j.bSuccess === false)")
            if hardware:
                wait_for("replies.some(j => j.status?.poseFresh)", 60)
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
                wait_for("parseInt(document.querySelector('#stats').textContent.replaceAll(',', '')) > 0")
                wait_for("streamEvents.some(e => e.type === 'submap') && streamEvents.some(e => e.type === 'chunk')")
                # Completed geometry is idle after replay, then replays once on reconnect.
                time.sleep(1)
                streamed = evaluate("streamEvents.filter(e => e.type === 'chunk').length")
                retained_points = evaluate("parseInt(document.querySelector('#stats').textContent.replaceAll(',', ''))")
                time.sleep(1)
                assert evaluate("streamEvents.filter(e => e.type === 'chunk').length") == streamed
                evaluate("document.querySelector('#stop').click(); document.querySelector('#start').click()")
                wait_for(f"document.querySelector('#status').textContent === 'Connected' && parseInt(document.querySelector('#stats').textContent.replaceAll(',', '')) === {retained_points}")
                evaluate("document.querySelector('#savePointCloud').click()")
                wait_for("replies.some(j => j.cmd === 'savePointCloud' && j.bSuccess)", 60)
                saved = evaluate("replies.findLast(j => j.cmd === 'savePointCloud')")
                cloud = Path(saved['path']); assert cloud.exists() and cloud.is_relative_to(tmp / 'exports'), saved
                assert cloud.read_bytes().startswith(b'ply\n') and saved['points'] > 0, saved
            wait_for("!document.querySelector('#parameterFields').disabled")
            evaluate("document.querySelector('#loadParameters').click()")
            wait_for("replies.some(j => j.cmd === 'loadConfig' && j.bSuccess)")
            assert evaluate("Number(document.querySelector('#param-nMinPoints').value)") == original_minimum + 1
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
            print('PASS: GLIM browser, submap accumulation/corrections, bounded trajectory, parameter save/load/edit locks, WSconsole lifecycle, renderer and reconnect')
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
