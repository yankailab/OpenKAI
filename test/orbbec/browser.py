"""Exercise the Orbbec page and real WSconsole using jsonCfg/Orbbec.json.

Usage: python3 test/orbbec/browser.py build/OpenKAI
Uses temporary ports/config/save files. Camera checks run when a device opens.
A synthetic IMU packet also checks all charts and orientation without hardware.
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
    chrome = shutil.which('google-chrome') or shutil.which('chromium')
    if not chrome: raise RuntimeError('Chromium is required')
    with tempfile.TemporaryDirectory(prefix='openkai-orbbec-browser-') as tmp:
        tmp = Path(tmp)
        config = json.loads((root / 'jsonCfg/Orbbec.json').read_text())
        for name in ['console', 'view', 'viewD', 'd2rgb']: config[name]['bON'] = False
        config['viewer']['bON'] = True
        port, cmd_port = free_port(), free_port()
        config['viewer']['host'] = config['wsServer']['host'] = '127.0.0.1'
        config['viewer']['port'] = port
        config['wsServer']['port'] = cmd_port
        config['wsServer']['bLog'] = config['wsConsole']['bLog'] = config['Orbbec']['bLog'] = True
        config['Orbbec']['fConfig'] = str(tmp / 'saved.json')
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
            assert b'Orbbec viewer' in request(port, '/')[2]
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
                    time.sleep(.05)
                raise AssertionError(evaluate("({status: document.querySelector('#configStatus')?.textContent, imu: document.querySelector('#imuStatus')?.textContent, log: document.querySelector('#cmdState')?.value})"))
            command('Runtime.enable'); command('Page.enable')
            command('Page.addScriptToEvaluateOnNewDocument', {'source': '''
              window.socketURLs = []; window.replies = []; window.samples = [];
              const NativeWebSocket = window.WebSocket;
              window.WebSocket = class extends NativeWebSocket {
                constructor(url, ...args) { super(url, ...args); socketURLs.push(String(url)); }
              };
              window.addEventListener('orbbeccommand', e => replies.push(e.detail));
              window.addEventListener('imudata', e => samples.push(e.detail));
            '''})
            # Verify file launcher and the real served page.
            command('Page.navigate', {'url': (root / 'html/viewer/_Orbbec/index.html').as_uri()})
            wait_for("location.protocol === 'file:' && !!window.viewerEndpoint")
            evaluate(f"document.querySelector('#port').value = '{port}'; document.querySelector('#cmdPort').value = '{cmd_port}'; document.querySelector('#start').click()")
            wait_for("location.protocol === 'http:' && document.querySelector('#status')?.textContent === 'Connected'")
            assert evaluate("document.querySelector('#cmdPort').value") == str(cmd_port)
            wait_for("document.querySelectorAll('#configSections .control').length > 120 && !document.querySelector('#loadConfig').disabled")
            assert evaluate("document.querySelectorAll('#imuGraphs canvas').length") == 6
            assert evaluate("document.querySelectorAll('#orientation canvas, #viewport canvas').length") == 2
            assert evaluate("socketURLs.filter(u => new URL(u).pathname.startsWith('/stream/')).map(u => new URL(u).pathname)") == ['/stream/points', '/stream/lines']
            assert evaluate("document.querySelectorAll('#configSections details').length") >= 7
            assert evaluate("document.querySelector('#param-OB_PROP_COLOR_EXPOSURE_INT').value") == ''
            assert not (tmp / 'saved.json').exists()
            device_open = evaluate("replies.findLast(j => j.cmd === 'loadConfig').deviceOpen")
            evaluate("var input = document.querySelector('#param-tOutMs'); input.value = '1001'; input.dispatchEvent(new Event('change'))")
            wait_for("replies.some(j => j.cmd === 'setConfig')")
            if device_open:
                assert evaluate("replies.findLast(j => j.cmd === 'setConfig').bSuccess")
                assert not (tmp / 'saved.json').exists(), 'Live edit wrote to disk'
                evaluate("document.querySelector('#saveConfig').click()")
                wait_for("replies.some(j => j.cmd === 'saveConfig')")
                assert json.loads((tmp / 'saved.json').read_text()) == {'tOutMs': 1001}
                evaluate("document.querySelector('#loadConfig').click()")
                wait_for("replies.filter(j => j.cmd === 'loadConfig').length >= 2")
                assert evaluate("document.querySelector('#param-tOutMs').value") == '1001'
                # Restart capture and verify an unsupported stream profile rolls back.
                evaluate("window.restartReplies = replies.length; var input = document.querySelector('#param-bPCLrgb'); input.value = 'false'; input.dispatchEvent(new Event('change'))")
                wait_for("replies.length > restartReplies && !document.querySelector('#cameraFields').disabled")
                assert evaluate("replies.at(-1).bSuccess && replies.at(-1).config.bPCLrgb === false")
                evaluate("window.restartReplies = replies.length; var input = document.querySelector('#param-bPCLrgb'); input.value = 'true'; input.dispatchEvent(new Event('change'))")
                wait_for("replies.length > restartReplies && !document.querySelector('#cameraFields').disabled")
                assert evaluate("replies.at(-1).bSuccess && replies.at(-1).config.bPCLrgb === true")
                evaluate("window.restartReplies = replies.length; var input = document.querySelector('#param-devFPS'); input.value = '999'; input.dispatchEvent(new Event('change'))")
                wait_for("replies.length > restartReplies && !document.querySelector('#cameraFields').disabled")
                assert evaluate("!replies.at(-1).bSuccess && replies.at(-1).config.devFPS === 30 && replies.at(-1).deviceOpen")
                assert json.loads((tmp / 'saved.json').read_text()) == {'tOutMs': 1001}
            else:
                assert not evaluate("replies.findLast(j => j.cmd === 'setConfig').bSuccess")
                assert evaluate("document.querySelector('#param-tOutMs').value") == '1000'
                evaluate("document.querySelector('#saveConfig').click()")
                wait_for("replies.some(j => j.cmd === 'saveConfig')")
                assert json.loads((tmp / 'saved.json').read_text()) == {}
            evaluate("document.querySelector('#imuStart').click()")
            wait_for("replies.some(j => j.cmd === 'startStream') && !document.querySelector('#imuStop').disabled")
            if device_open:
                wait_for("samples.some(j => j.orientationValid)", 20)
                assert evaluate("samples.every(j => j.gyro.length === 3 && j.acc.length === 3 && j.quaternion.length === 4)")
                wait_for("parseInt(document.querySelector('#stats').textContent.replaceAll(',', '')) > 0")
            # Feed fragmented JSON through the production parser, including quoted braces.
            evaluate('''(() => {
              const sample = {cmd:'imuData', module:'obIMU', tGyro:987654321, tAcc:987654321,
                gyro:[1,2,3], acc:[4,5,6], quaternion:[0.70710678,0,0,0.70710678], rpy:[0,0,Math.PI/2],
                fusion:true, orientationValid:true};
              const text = JSON.stringify(sample);
              for (let i=0; i<text.length; i+=7) cmdHandler({data:text.slice(i,i+7)});
            })()''')
            assert evaluate("[...document.querySelectorAll('.graph figcaption span:last-child')].map(e => e.textContent)") == ['1.000','2.000','3.000','4.000','5.000','6.000']
            assert evaluate("document.querySelector('#angles').textContent.includes('90.0°')")
            evaluate("document.querySelector('#imuStop').click()")
            wait_for("replies.some(j => j.cmd === 'stopStream') && document.querySelector('#imuStatus').textContent === 'Stopped'")
            count = evaluate('samples.length'); time.sleep(.15)
            assert evaluate('samples.length') == count
            screenshot = command('Page.captureScreenshot', {'format': 'png'})
            Path('/tmp/openkai-orbbec-viewer.png').write_bytes(base64.b64decode(screenshot['data']))
            evaluate("document.querySelector('#stop').click(); document.querySelector('#cmdDisconnect').click()")
            wait_for("document.querySelector('#status').textContent === 'Stopped' && document.querySelector('#imuStart').disabled && document.querySelector('#saveConfig').disabled")
            assert not exceptions, exceptions
            print('PASS: Orbbec browser, real WSconsole, control schema, live/save separation, fragmented IMU JSON, six graphs, orientation and stop/disconnect')
            print('Camera: ' + ('live point cloud and fused IMU verified' if device_open else 'no open camera; offline rejection verified'))
        finally:
            if client: client.close()
            if browser:
                browser.terminate(); browser.wait(timeout=10)
            if server.poll() is None:
                server.send_signal(signal.SIGINT)
                try: server.wait(timeout=10)
                except subprocess.TimeoutExpired: server.kill(); server.wait()
            log.seek(0)
            Path('/tmp/openkai-orbbec-browser-backend.log').write_text(log.read())
            log.close()

if __name__ == '__main__': main()
