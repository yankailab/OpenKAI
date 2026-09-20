"""Optional real Chromium smoke test using CDP and the Python standard library.

Usage: python3 geometry_browser.py /path/to/viewer_fixture /path/to/html/viewer/_GeometryBase
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
            # Record every stream request, including connections made by main.js.
            command('Page.addScriptToEvaluateOnNewDocument', {'source': '''
              window.socketURLs = [];
              const NativeWebSocket = window.WebSocket;
              window.WebSocket = class extends NativeWebSocket {
                constructor(url, ...args) { super(url, ...args); socketURLs.push(String(url)); }
              };
            '''})
            command('Page.navigate', {'url': (Path(root).resolve() / 'index.html').as_uri()})
            wait_for("location.protocol === 'file:' && !!window.viewerEndpoint")
            evaluate(f"document.querySelector('#port').value = '{port}'; document.querySelector('#start').click()")
            wait_for("location.protocol === 'http:' && document.querySelector('#stats')?.textContent.includes('10,000 points')")
            assert evaluate("document.querySelector('#status').textContent") == 'Connected'
            assert evaluate("document.querySelector('#stats').textContent.includes('1 lines')")
            assert evaluate("document.querySelectorAll('aside, #cmdPort, #objects').length") == 0
            assert evaluate("socketURLs.map(url => new URL(url).pathname)") == ['/stream/points', '/stream/lines']
            result = evaluate('''(async () => {
              const { Viewer3D } = await import('/js/viewer3D.js');
              const { decodeFrame } = await import('/js/protocol.js');
              async function snapshot(type) {
                return new Promise(resolve => {
                  const ws = new WebSocket(`ws://${location.host}/stream/${type}`);
                  ws.binaryType = 'arraybuffer';
                  ws.onmessage = e => {
                    if (typeof e.data === 'string') {
                      window.geometryHello = JSON.parse(e.data); ws.send('start');
                    } else { ws.close(); resolve(e.data); }
                  };
                });
              }
              const pointBytes = await snapshot('points'), lineBytes = await snapshot('lines');
              const points = decodeFrame(pointBytes, 'points'), lines = decodeFrame(lineBytes, 'lines');
              if (points.objects[0].positions.buffer !== pointBytes || points.objects[0].colors.buffer !== pointBytes)
                throw Error('Attributes are not zero-copy views');
              for (const data of [pointBytes.slice(0, -1), await snapshot('cells')]) {
                let rejected = false;
                try { decodeFrame(data); } catch { rejected = true; }
                if (!rejected) throw Error('Unsupported frame accepted');
              }
              for (const [offset, value] of [[0, 0], [4, 4], [8, 3], [16, 1025], [36, 0xffffffff]]) {
                const copy = pointBytes.slice(0); new DataView(copy).setUint32(offset, value, true);
                let rejected = false;
                try { decodeFrame(copy); } catch { rejected = true; }
                if (!rejected) throw Error('Malformed frame accepted');
              }
              const container = document.createElement('div');
              container.style.cssText = 'position:fixed;left:0;top:100px;width:640px;height:400px';
              document.body.append(container);
              const viewer = window.testViewer = new Viewer3D(container);
              viewer.configure(geometryHello);
              viewer.update(points); viewer.update(lines); viewer.render();
              if (viewer.renderer.info.render.points !== 10000 || viewer.objects.size !== 1 ||
                  viewer.objects.get(7).lines.geometry.drawRange.count !== 2 || !viewer.controls.enabled)
                throw Error('Points, lines or camera controls missing');
              // Removing one stream must preserve the other source's geometry.
              viewer.clearStream('points'); viewer.render();
              if (viewer.renderer.info.render.points !== 0 || viewer.objects.get(7).lines.geometry.drawRange.count !== 2)
                throw Error('Stream clearing affected lines');
              viewer.update(points);
              viewer.update({type:'lines', objects:[]});
              if (viewer.objects.get(7).points.geometry.drawRange.count !== 10000 ||
                  viewer.objects.get(7).lines.geometry.drawRange.count !== 0) throw Error('Stale lines');
              viewer.update(lines); viewer.render();
              return 'PASS: version-5 decoder, cell rejection, WebGL points/lines and independent clearing';
            })()''')
            before = evaluate('testViewer.camera.position.toArray()')
            command('Input.dispatchMouseEvent', {'type': 'mousePressed', 'x': 250, 'y': 250, 'button': 'left', 'clickCount': 1})
            command('Input.dispatchMouseEvent', {'type': 'mouseMoved', 'x': 320, 'y': 270, 'button': 'left', 'buttons': 1})
            command('Input.dispatchMouseEvent', {'type': 'mouseReleased', 'x': 320, 'y': 270, 'button': 'left', 'clickCount': 1})
            evaluate('testViewer.render()')
            assert evaluate('testViewer.camera.position.toArray()') != before
            evaluate('testViewer.dispose(); testViewer.container.remove()')
            # Only the application sockets were points/lines; extra sockets above are decoder tests.
            evaluate("document.querySelector('#stop').click()")
            wait_for("document.querySelector('#status').textContent === 'Stopped'")
            wait_for("document.querySelector('#stats').textContent.startsWith('0 points · 0 lines')")
            evaluate("document.querySelector('#start').click()")
            wait_for("document.querySelector('#stats').textContent.includes('10,000 points')")
            screenshot = command('Page.captureScreenshot', {'format': 'png'})
            Path('/tmp/openkai-geometry-viewer.png').write_bytes(base64.b64decode(screenshot['data']))
            assert not exceptions, exceptions
            print(result)
            print('PASS: local launcher, only two application sockets, mouse navigation, Stop/Start')
            print('Screenshot: /tmp/openkai-geometry-viewer.png')
        finally:
            if client: client.close()
            if browser:
                browser.terminate()
                browser.wait(timeout=5)
            server.communicate('\n', timeout=5)


if __name__ == '__main__': main()
