"""Optional real Chromium smoke test using CDP and the Python standard library.

Usage: python3 browser.py /path/to/viewer_fixture /path/to/html/webViewer3D
Requires google-chrome or chromium; writes a screenshot to /tmp.
"""
import base64
import json
import shutil
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path
from integration import WebSocket


def main():
    fixture, root = sys.argv[1:]
    chrome = shutil.which('google-chrome') or shutil.which('chromium')
    if not chrome: raise RuntimeError('Chromium is required for the browser smoke test')
    server = subprocess.Popen([fixture, root], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    browser = None
    client = None
    with tempfile.TemporaryDirectory(prefix='openkai-chrome-') as profile:
        try:
            port = int(server.stdout.readline())
            browser = subprocess.Popen([chrome, '--headless=new', '--no-sandbox', '--disable-dev-shm-usage',
                '--use-gl=angle', '--use-angle=swiftshader', '--enable-unsafe-swiftshader',
                '--no-first-run', '--no-default-browser-check', '--disable-background-networking',
                '--remote-debugging-port=0', f'--user-data-dir={profile}', '--window-size=1280,800', 'about:blank'],
                stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
            active = Path(profile) / 'DevToolsActivePort'
            deadline = time.monotonic() + 10
            while not active.exists() and time.monotonic() < deadline: time.sleep(.05)
            debug_port = int(active.read_text().splitlines()[0])
            with urllib.request.urlopen(f'http://127.0.0.1:{debug_port}/json/list') as response:
                target = next(t for t in json.load(response) if t['type'] == 'page')
            client = WebSocket(debug_port, '/' + target['webSocketDebuggerUrl'].split('/', 3)[3])
            client.socket.settimeout(15)
            next_id = 0
            exceptions = []
            def command(method, params=None):
                nonlocal next_id
                next_id += 1
                client.send(json.dumps({'id': next_id, 'method': method, 'params': params or {}}))
                while True:
                    _, payload = client.receive()
                    message = json.loads(payload)
                    if message.get('method') == 'Runtime.exceptionThrown': exceptions.append(message)
                    if message.get('id') == next_id:
                        assert 'error' not in message, message
                        return message.get('result', {})
            def evaluate(expression):
                result = command('Runtime.evaluate', {'expression': expression, 'awaitPromise': True, 'returnByValue': True})
                assert 'exceptionDetails' not in result, result
                return result['result'].get('value')
            def wait_for(expression):
                end = time.monotonic() + 15
                while time.monotonic() < end:
                    if evaluate(expression): return
                    time.sleep(.05)
                raise AssertionError(evaluate("({url: location.href, status: document.querySelector('#status')?.textContent, html: document.body.innerText})"))
            command('Runtime.enable')
            command('Page.enable')
            # Opening the local launcher and clicking Start must navigate to the backend.
            command('Page.navigate', {'url': (Path(root).resolve() / 'index.html').as_uri()})
            wait_for("location.protocol === 'file:' && !!window.viewerEndpoint")
            evaluate(f"document.querySelector('#port').value = '{port}'; document.querySelector('#start').click();")
            wait_for("location.protocol === 'http:' && document.querySelector('#stats')?.textContent.includes('10,000 points')")
            assert evaluate("document.querySelector('#status').textContent") == 'Connected'
            assert evaluate("document.querySelectorAll('#objects input').length") == 1
            # Protocol validation and zero-copy decoding in the actual browser JS engine.
            result = evaluate(f"""(async () => {{
              const {{ decodeFrame }} = await import('http://127.0.0.1:{port}/js/protocol.js');
              const data = await new Promise(resolve => {{
                const ws = new WebSocket('ws://127.0.0.1:{port}/stream'); ws.binaryType = 'arraybuffer';
                ws.onmessage = e => {{ if (typeof e.data === 'string') ws.send('start'); else {{ ws.close(); resolve(e.data); }} }};
              }});
              const f = decodeFrame(data);
              if (f.objects[0].points.buffer !== data || f.objects[0].lines.length !== 6) throw Error('Incorrect attributes');
              const malformed = [new ArrayBuffer(0), data.slice(0, -1)];
              for (const [offset, value] of [[0, 0], [4, 99], [12, 1025], [36, 0xffffffff]]) {{
                const copy = data.slice(0); new DataView(copy).setUint32(offset, value, true); malformed.push(copy);
              }}
              for (const b of malformed) {{ let rejected = false; try {{ decodeFrame(b); }} catch {{ rejected = true; }} if (!rejected) throw Error('Malformed frame accepted'); }}
              return 'PASS: zero-copy attributes and malformed frames';
            }})()""")
            evaluate("document.querySelector('#fit').click(); document.querySelector('#objects input').click(); document.querySelector('#objects input').click();")
            screenshot = command('Page.captureScreenshot', {'format': 'png'})['data']
            Path('/tmp/openkai-webviewer.png').write_bytes(base64.b64decode(screenshot))
            evaluate("document.querySelector('#stop').click()")
            assert evaluate("document.querySelector('#status').textContent") == 'Stopped'
            evaluate("window.TestWebSocket = window.WebSocket; window.WebSocket = class extends window.TestWebSocket { constructor(...args) { super(...args); window.testSocket = this; } };")
            evaluate("document.querySelector('#start').click()")
            wait_for("document.querySelector('#status').textContent === 'Connected'")
            evaluate("window.testSocket.close()")
            wait_for("document.querySelector('#status').textContent.includes('retrying')")
            wait_for("document.querySelector('#status').textContent === 'Connected'")
            assert not exceptions, exceptions
            print('PASS: local-file launcher, WebGL2 rendering, camera/visibility controls, Stop/Start; ' + result)
            print('Screenshot: /tmp/openkai-webviewer.png')
        finally:
            if client: client.close()
            if browser:
                browser.terminate()
                browser.wait(timeout=5)
            server.communicate('\n', timeout=5)


if __name__ == '__main__': main()
