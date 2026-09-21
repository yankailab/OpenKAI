"""Exercise the Scepter viewer through real WSconsole using jsonCfg/Scepter.json.

Usage: python3 test/scepter/browser.py build/OpenKAI
Uses temporary ports/config/save files; validates live points when a camera opens.
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
    with tempfile.TemporaryDirectory(prefix='openkai-scepter-browser-') as tmp:
        tmp = Path(tmp)
        config = json.loads((root / 'jsonCfg/Scepter.json').read_text())
        for name in ['console', 'view', 'viewD', 'd2rgb']: config[name]['bON'] = False
        config['viewer']['bON'] = True
        port, cmd_port = free_port(), free_port()
        config['viewer']['host'] = config['wsServer']['host'] = '127.0.0.1'
        config['viewer']['port'] = port
        config['wsServer']['port'] = cmd_port
        config['wsServer']['bLog'] = config['wsConsole']['bLog'] = config['scepter']['bLog'] = True
        config['scepter']['fConfig'] = str(tmp / 'saved.json')
        config['scepter']['scScanTime'] = 100
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
            assert b'Scepter viewer' in request(port, '/')[2]
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
                raise AssertionError(evaluate("({status: document.querySelector('#configStatus')?.textContent, log: document.querySelector('#cmdState')?.value})"))
            command('Runtime.enable'); command('Page.enable')
            command('Page.addScriptToEvaluateOnNewDocument', {'source': '''
              window.socketURLs = []; window.replies = [];
              const NativeWebSocket = window.WebSocket;
              window.WebSocket = class extends NativeWebSocket {
                constructor(url, ...args) { super(url, ...args); socketURLs.push(String(url)); }
              };
              window.addEventListener('sceptercommand', e => replies.push(e.detail));
            '''})
            # Verify file launcher and the real served page.
            command('Page.navigate', {'url': (root / 'html/viewer/_Scepter/index.html').as_uri()})
            wait_for("location.protocol === 'file:' && !!window.viewerEndpoint")
            evaluate(f"document.querySelector('#port').value = '{port}'; document.querySelector('#cmdPort').value = '{cmd_port}'; document.querySelector('#start').click()")
            wait_for("location.protocol === 'http:' && document.querySelector('#status')?.textContent === 'Connected'")
            assert evaluate("document.querySelector('#cmdPort').value") == str(cmd_port)
            wait_for("document.querySelectorAll('#configSections .control').length > 50 && !document.querySelector('#loadConfig').disabled")
            assert evaluate("document.querySelectorAll('#imuPanel, #imuGraphs, #orientation').length") == 0
            assert evaluate("document.querySelectorAll('#viewport canvas').length") == 1
            assert evaluate("socketURLs.filter(u => new URL(u).pathname.startsWith('/stream/')).map(u => new URL(u).pathname)") == ['/stream/points', '/stream/lines']
            assert evaluate("document.querySelectorAll('#configSections details').length") >= 7
            assert evaluate("document.querySelector('#cameraModule').value") == 'scepter'
            assert evaluate("document.querySelector('#param-scTimeFilterThreshold').value") == '1'
            assert evaluate("document.querySelector('#param-scAIModuleParams').tagName") == 'TEXTAREA'
            assert not (tmp / 'saved.json').exists()
            device_open = evaluate("replies.findLast(j => j.cmd === 'loadConfig').deviceOpen")
            evaluate("var input = document.querySelector('#param-pclStride'); input.value = '3'; input.dispatchEvent(new Event('change'))")
            wait_for("replies.some(j => j.cmd === 'setConfig')")
            assert evaluate("replies.findLast(j => j.cmd === 'setConfig').bSuccess") == device_open
            assert evaluate("document.querySelector('#param-pclStride').value") == ('3' if device_open else '2')
            assert not (tmp / 'saved.json').exists(), 'Live edit wrote to disk'
            evaluate("document.querySelector('#saveConfig').click()")
            wait_for("replies.some(j => j.cmd === 'saveConfig')")
            saved = json.loads((tmp / 'saved.json').read_text())
            assert len(saved) > 50 and saved['scHDR'] is False and saved['scAIModuleParams'] == []
            assert saved == evaluate("replies.findLast(j => j.cmd === 'saveConfig').config")
            evaluate("document.querySelector('#loadConfig').click()")
            wait_for("replies.filter(j => j.cmd === 'loadConfig').length >= 2")
            assert evaluate("document.querySelector('#param-pclStride').value") == ('3' if device_open else '2')
            if device_open:
                wait_for("parseInt(document.querySelector('#stats').textContent.replaceAll(',', '')) > 0")
            # Exercise the production parser with fragments and framing inside strings.
            evaluate('''(() => {
              const packet = JSON.stringify({cmd:'parserTest', module:'scepter', note:'quoted } EOJ { text'});
              for (let i=0; i<packet.length; i+=7) cmdHandler({data:packet.slice(i,i+7)});
              cmdHandler({data:'E'}); cmdHandler({data:'OJ'});
            })()''')
            assert evaluate("replies.findLast(j => j.cmd === 'parserTest').note") == 'quoted } EOJ { text'
            screenshot = command('Page.captureScreenshot', {'format': 'png'})
            Path('/tmp/openkai-scepter-viewer.png').write_bytes(base64.b64decode(screenshot['data']))
            evaluate("document.querySelector('#stop').click(); document.querySelector('#cmdDisconnect').click()")
            wait_for("document.querySelector('#status').textContent === 'Stopped' && document.querySelector('#saveConfig').disabled")
            assert not exceptions, exceptions
            print('PASS: Scepter browser, real WSconsole, categorized controls, full save, live/save separation, fragmented JSON and disconnect')
            print('Camera: ' + ('live point cloud verified' if device_open else 'no open camera; offline rejection verified'))
        finally:
            if client: client.close()
            if browser:
                browser.terminate(); browser.wait(timeout=10)
            if server.poll() is None:
                server.send_signal(signal.SIGINT)
                try: server.wait(timeout=10)
                except subprocess.TimeoutExpired: server.kill(); server.wait()
            log.seek(0)
            Path('/tmp/openkai-scepter-browser-backend.log').write_text(log.read())
            log.close()

if __name__ == '__main__': main()
