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
from command_fixture import CommandServer


def main():
    fixture, root = sys.argv[1:]
    chrome = shutil.which('google-chrome') or shutil.which('chromium')
    if not chrome: raise RuntimeError('Chromium is required for the browser smoke test')
    server = subprocess.Popen([fixture, root], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
    browser = None
    client = None
    commands = CommandServer()
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
            evaluate(f"document.querySelector('#port').value = '{port}'; document.querySelector('#cmdPort').value = '{commands.port}'; document.querySelector('#start').click();")
            wait_for("location.protocol === 'http:' && document.querySelector('#stats')?.textContent.includes('10,000 points')")
            assert evaluate("document.querySelector('#status').textContent") == 'Connected'
            assert evaluate("document.querySelectorAll('#objects input').length") == 1
            wait_for("window.wsSocket?.readyState === WebSocket.OPEN")
            assert evaluate("document.querySelector('#cmdPort').value") == str(commands.port)
            assert evaluate("window.wsSocket.url") == f'ws://127.0.0.1:{commands.port}/'
            evaluate("window.testReplies = []; const originalHandler = window.handleCmd; window.handleCmd = j => { testReplies.push(j); originalHandler(j); };")
            assert evaluate("wsSendCmd({cmd: 'test', module: 'tester', v: 7})")
            wait_for("testReplies.some(j => j.cmd === 'ackTest' && j.v === 7)")
            assert commands.received == [{'cmd': 'test', 'module': 'tester', 'v': 7}]
            assert evaluate("testReplies.find(j => j.cmd === 'ackTest').text.length") > 1024
            wait_for("testReplies.some(j => j.cmd === 'hb') && wsCmdBuffer === ''")
            assert 'ackTest' in evaluate("document.querySelector('#cmdState').value")
            # Malformed input and log limits never interrupt streaming or throw globally.
            evaluate("cmdHandler({data: '{bad json}'}); cmdHandler({data: '{\"cmd\":\"hb\"}'}); wsCmdLog('x'.repeat(20000));")
            assert evaluate("document.querySelector('#cmdState').value.length") == 16384
            assert not evaluate("wsSendCmd({cmd: 'test', module: 'tester', v: 'EOJ'})")
            evaluate("cmdHandler({data: '{\"cmd\":'}); document.querySelector('#cmdDisconnect').click();")
            assert evaluate("wsCmdBuffer === '' && wsSocket === null")
            assert not evaluate("wsSendCmd({cmd: 'test', module: 'tester', v: 8})")
            assert evaluate("document.querySelector('#status').textContent") == 'Connected'
            # A refused command handshake must leave the binary stream connected.
            evaluate(f"document.querySelector('#cmdPort').value = '{port}'; document.querySelector('#cmdConnect').click();")
            wait_for("wsSocket === null")
            assert evaluate("document.querySelector('#status').textContent") == 'Connected'
            evaluate(f"document.querySelector('#cmdPort').value = '{commands.port}'; document.querySelector('#cmdConnect').click();")
            wait_for("wsSocket?.readyState === WebSocket.OPEN")
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
            grid_result = evaluate(f"""(async () => {{
              const {{ decodeFrame }} = await import('/js/protocol.js');
              const {{ cellBox }} = await import('/js/octreeCells.js');
              const {{ GridBoxes }} = await import('/js/gridBoxes.js');
              const THREE = await import('/vendor/three.module.min.js');
              const data = await new Promise(resolve => {{
                const ws = new WebSocket('ws://127.0.0.1:{port}/stream'); ws.binaryType = 'arraybuffer';
                ws.onmessage = e => {{ if (typeof e.data === 'string') ws.send('start'); else {{ ws.close(); resolve(e.data); }} }};
              }});
              const object = decodeFrame(data).objects[0], grid = object.grid;
              if (object.nC !== 41 || grid.cells.length !== 41 * 19 || grid.cells.buffer !== data) throw Error('Incorrect cell payload');
              for (let depth = 0; depth <= 40; ++depth) {{
                const box = cellBox(grid, depth), scale = 2 ** -depth;
                if (box.id.length !== 16 || box.id[0] % 64 !== depth || box.color.join() !== '255,51,0' ||
                    box.size.some(x => x !== 2 * scale) || box.center.some(x => x !== 1 - scale)) throw Error('Incorrect cell box');
              }}
              // Reject malformed counts, metadata, path bits and padding atomically.
              const start = 32 + 64 + object.nP * 16 + object.nL * 32;
              const legacy = data.slice(0, start), lv = new DataView(legacy);
              lv.setUint32(4, 1, true); lv.setUint32(16, legacy.byteLength, true);
              lv.setUint32(44, 0, true); lv.setUint32(80, 0, true);
              if (decodeFrame(legacy).objects[0].grid !== null) throw Error('Legacy frame failed');
              // A grid with 41 unaligned records followed by ordinary geometry.
              const mixed = new Uint8Array(data.byteLength + legacy.byteLength - 32);
              mixed.set(new Uint8Array(data)); mixed.set(new Uint8Array(legacy, 32), data.byteLength);
              const mv = new DataView(mixed.buffer);
              mv.setUint32(12, 2, true); mv.setUint32(16, mixed.byteLength, true);
              mv.setUint32(data.byteLength, 8, true);
              const second = decodeFrame(mixed.buffer).objects[1];
              if (second.points.buffer !== mixed.buffer || second.points[0] !== 1 || second.nC !== 0) throw Error('Mixed object alignment');
              const malformed = [];
              for (const [at, value] of [[44, 0xffffffff], [80, 2], [start + 24, 41]]) {{
                const copy = data.slice(0); new DataView(copy).setUint32(at, value, true); malformed.push(copy);
              }}
              for (const [at, value] of [[start + 40, 41], [start + 55, 128], [start + 41, 1], [data.byteLength - 1, 1]]) {{
                const copy = data.slice(0); new Uint8Array(copy)[at] = value; malformed.push(copy);
              }}
              const copy = data.slice(0); new DataView(copy).setFloat32(start + 12, -1, true); malformed.push(copy);
              for (const b of malformed) {{ let bad = false; try {{ decodeFrame(b); }} catch {{ bad = true; }} if (!bad) throw Error('Invalid grid accepted'); }}
              const boxes = new GridBoxes();
              boxes.update(grid, object.bounds, 1);
              if (boxes.geometry.instanceCount !== 41 || boxes.getCell(1).box.min.toArray().join() !== '0,0,0' || boxes.getCell(41)) throw Error('Box identity/bounds');
              const renderer = new THREE.WebGLRenderer({{ preserveDrawingBuffer: true }});
              renderer.setSize(128, 128);
              const scene = new THREE.Scene(), camera = new THREE.PerspectiveCamera(60, 1, .1, 100);
              camera.position.set(0, 0, 5); scene.add(boxes);
              renderer.render(scene, camera);
              if (renderer.info.render.lines !== 41 * 12) throw Error('Boxes not rendered as instances');
              const gl = renderer.getContext(), pixels = new Uint8Array(128 * 128 * 4);
              gl.readPixels(0, 0, 128, 128, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
              let colored = 0;
              for (let i = 0; i < pixels.length; i += 4) if (pixels[i] > 200 && pixels[i + 1] > 30 && pixels[i + 2] < 10) ++colored;
              if (colored < 50) throw Error('No visible colored boxes');
              const many = {{ ...grid, cells: new Uint8Array(300 * 19) }};
              for (let i = 0; i < 300; ++i) many.cells.set(grid.cells.subarray(0, 19), i * 19);
              boxes.update(many, object.bounds, .5); renderer.render(scene, camera);
              if (renderer.info.render.lines !== 300 * 12) throw Error('Instance capacity did not grow');
              boxes.update({{ ...grid, cells: new Uint8Array(0) }}, object.bounds, 1);
              renderer.render(scene, camera);
              if (boxes.getCell(0) || renderer.info.render.lines !== 0) throw Error('Empty grid left stale boxes');
              // The same RGB must produce the same screen color for cells and
              // points. Intermediate channel values expose skipped conversion.
              const pointGeometry = new THREE.BufferGeometry();
              pointGeometry.setAttribute('position', new THREE.Float32BufferAttribute([0, 0, 0], 3));
              const pointColor = new THREE.BufferAttribute(new Uint8Array(4), 4, true);
              pointGeometry.setAttribute('color', pointColor);
              const point = new THREE.Points(pointGeometry, new THREE.PointsMaterial({{ vertexColors: true, size: 8, sizeAttenuation: false }}));
              const screenColor = () => {{
                renderer.render(scene, camera);
                gl.readPixels(0, 0, 128, 128, gl.RGBA, gl.UNSIGNED_BYTE, pixels);
                const counts = new Map();
                for (let i = 0; i < pixels.length; i += 4) {{
                  if (!pixels[i] && !pixels[i + 1] && !pixels[i + 2]) continue;
                  const rgb = pixels.subarray(i, i + 3).join();
                  counts.set(rgb, (counts.get(rgb) || 0) + 1);
                }}
                return [...counts].sort((a, b) => b[1] - a[1])[0]?.[0];
              }};
              for (const rgb of [[64, 128, 192], [192, 64, 128], [128, 192, 64]]) {{
                scene.remove(boxes); scene.add(point);
                pointColor.array.set([...rgb, 255]); pointColor.needsUpdate = true;
                const expected = screenColor();
                const cells = new Uint8Array(19); cells.set(rgb, 16);
                boxes.update({{ ...grid, cells }}, object.bounds, 1);
                scene.remove(point); scene.add(boxes);
                const actual = screenColor();
                if (!expected || actual !== expected) throw Error(`Cell RGB ${{rgb}} rendered as ${{actual}}, point rendered as ${{expected}}`);
              }}
              pointGeometry.dispose(); point.material.dispose();
              boxes.geometry.dispose(); boxes.material.dispose(); renderer.dispose();
              return 'PASS: grid decoding through depth 40, validation, instanced WebGL boxes, point/cell RGB agreement, resizing and clearing';
            }})()""")
            print(grid_result)
            evaluate("document.querySelector('#fit').click(); document.querySelector('#objects input').click(); document.querySelector('#objects input').click();")
            screenshot = command('Page.captureScreenshot', {'format': 'png'})['data']
            Path('/tmp/openkai-webviewer.png').write_bytes(base64.b64decode(screenshot))
            evaluate("document.querySelector('#stop').click()")
            assert evaluate("document.querySelector('#status').textContent") == 'Stopped'
            assert evaluate("wsSocket === null")
            evaluate("window.TestWebSocket = window.WebSocket; window.WebSocket = class extends window.TestWebSocket { constructor(...args) { super(...args); if (this.url.endsWith('/stream')) window.testSocket = this; } };")
            evaluate("document.querySelector('#start').click()")
            wait_for("document.querySelector('#status').textContent === 'Connected'")
            wait_for("wsSocket?.readyState === WebSocket.OPEN")
            evaluate("window.savedCmdSocket = wsSocket")
            evaluate("window.testSocket.close()")
            wait_for("document.querySelector('#status').textContent.includes('retrying')")
            wait_for("document.querySelector('#status').textContent === 'Connected'")
            assert evaluate("wsSocket === savedCmdSocket && wsSocket.readyState === WebSocket.OPEN")
            assert not exceptions, exceptions
            print('PASS: local-file launcher, WebGL2 rendering, camera/visibility controls, Stop/Start; ' + result)
            print('Screenshot: /tmp/openkai-webviewer.png')
            print('PASS: independent command port, JSON + EOJ sending, split replies, bounded console, command failure isolation')
        finally:
            if client: client.close()
            if browser:
                browser.terminate()
                browser.wait(timeout=5)
            server.communicate('\n', timeout=5)
            commands.close()


if __name__ == '__main__': main()
