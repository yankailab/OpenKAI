#!/usr/bin/env python3
"""Exercise the standalone editor in real Chrome using only Python's standard library.

Run from any directory: python3 html/console/editor/tests/browser-smoke.py
No HTTP server, installed test libraries, or network assets are needed.
"""

import argparse
import base64
import hashlib
import json
import os
from pathlib import Path
import shutil
import socket
import struct
import subprocess
import tempfile
import time
import urllib.parse
import urllib.request


class DevTools:
    """Minimal synchronous Chrome DevTools Protocol client over WebSocket."""

    def __init__(self, url):
        address = urllib.parse.urlsplit(url)
        self.socket = socket.create_connection((address.hostname, address.port), timeout=15)
        self.buffer = b""
        self.sequence = 0
        self.events = []
        key = base64.b64encode(os.urandom(16)).decode()
        request = (
            f"GET {address.path} HTTP/1.1\r\nHost: {address.netloc}\r\n"
            f"Upgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: {key}\r\n"
            "Sec-WebSocket-Version: 13\r\n\r\n"
        )
        self.socket.sendall(request.encode())
        while b"\r\n\r\n" not in self.buffer:
            self.buffer += self.socket.recv(4096)
        header, self.buffer = self.buffer.split(b"\r\n\r\n", 1)
        expected = base64.b64encode(hashlib.sha1(
            (key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode()
        ).digest())
        if not header.startswith(b"HTTP/1.1 101 ") or expected.lower() not in header.lower():
            raise RuntimeError(f"WebSocket upgrade failed: {header!r}")

    def _read(self, count):
        while len(self.buffer) < count:
            chunk = self.socket.recv(max(4096, count - len(self.buffer)))
            if not chunk:
                raise EOFError("Chrome closed the DevTools connection")
            self.buffer += chunk
        result, self.buffer = self.buffer[:count], self.buffer[count:]
        return result

    def _send(self, payload, opcode=1):
        mask = os.urandom(4)
        length = len(payload)
        header = bytes([0x80 | opcode])
        if length < 126:
            header += bytes([0x80 | length])
        elif length <= 65535:
            header += bytes([0x80 | 126]) + struct.pack("!H", length)
        else:
            header += bytes([0x80 | 127]) + struct.pack("!Q", length)
        encoded = bytes(value ^ mask[index % 4] for index, value in enumerate(payload))
        self.socket.sendall(header + mask + encoded)

    def _receive(self):
        message = bytearray()
        while True:
            first, second = self._read(2)
            length = second & 127
            if length == 126:
                length = struct.unpack("!H", self._read(2))[0]
            elif length == 127:
                length = struct.unpack("!Q", self._read(8))[0]
            mask = self._read(4) if second & 128 else None
            payload = self._read(length)
            if mask:
                payload = bytes(value ^ mask[index % 4] for index, value in enumerate(payload))
            opcode = first & 15
            if opcode == 8:
                raise EOFError("Chrome closed the DevTools WebSocket")
            if opcode == 9:
                self._send(payload, opcode=10)
                continue
            if opcode == 10:
                continue
            message.extend(payload)
            if first & 128:
                return json.loads(message)

    def call(self, method, **params):
        self.sequence += 1
        self._send(json.dumps({"id": self.sequence, "method": method, "params": params}).encode())
        while True:
            response = self._receive()
            if response.get("id") == self.sequence:
                if "error" in response:
                    raise RuntimeError(f"{method}: {response['error']}")
                return response.get("result", {})
            self.events.append(response)

    def evaluate(self, expression):
        result = self.call("Runtime.evaluate", expression=expression, awaitPromise=True,
                           returnByValue=True, userGesture=True)
        if "exceptionDetails" in result:
            details = result["exceptionDetails"]
            raise AssertionError(details.get("exception", {}).get("description", details))
        return result.get("result", {}).get("value")

    def close(self):
        self.socket.close()


class Browser:
    def __enter__(self):
        executable = os.environ.get("CHROME") or next(
            (shutil.which(name) for name in ("google-chrome", "chromium", "chromium-browser")
             if shutil.which(name)), None)
        if not executable:
            raise SystemExit("Chrome or Chromium is required; set CHROME to its executable path.")
        self.directory = tempfile.TemporaryDirectory(prefix="openkai-editor-browser-")
        profile = Path(self.directory.name)
        self.log = open(profile / "chrome.log", "w+")
        self.process = subprocess.Popen([
            executable, "--headless=new", "--no-sandbox", "--disable-dev-shm-usage",
            "--disable-background-networking", "--disable-component-update", "--disable-sync",
            "--no-first-run", "--no-default-browser-check", "--no-proxy-server",
            "--host-resolver-rules=MAP * ~NOTFOUND, EXCLUDE localhost",
            "--disable-features=MediaRouter", "--remote-debugging-port=0",
            f"--user-data-dir={profile}", "--window-size=1440,1000", "about:blank"
        ], stdout=self.log, stderr=self.log)
        try:
            deadline = time.monotonic() + 20
            port_file = profile / "DevToolsActivePort"
            while not port_file.exists():
                if self.process.poll() is not None or time.monotonic() > deadline:
                    self.log.seek(0)
                    raise RuntimeError("Chrome did not start: " + self.log.read()[-4000:])
                time.sleep(0.05)
            port = int(port_file.read_text().splitlines()[0])
            with urllib.request.urlopen(f"http://127.0.0.1:{port}/json/list", timeout=5) as response:
                targets = json.load(response)
            page = next(target for target in targets if target["type"] == "page")
            self.devtools = DevTools(page["webSocketDebuggerUrl"])
            self.devtools.call("Runtime.enable")
            self.devtools.call("Log.enable")
            self.devtools.call("Page.enable")
            self.devtools.call("Network.enable")
            self.devtools.call("Network.emulateNetworkConditions", offline=True, latency=0,
                               downloadThroughput=-1, uploadThroughput=-1)
            self.devtools.call("Emulation.setDeviceMetricsOverride", width=1440, height=1000,
                               deviceScaleFactor=1, mobile=False)
            return self.devtools
        except BaseException:
            self.__exit__(None, None, None)
            raise

    def __exit__(self, *_):
        if hasattr(self, "devtools"):
            self.devtools.close()
        self.process.terminate()
        try:
            self.process.wait(timeout=5)
        except subprocess.TimeoutExpired:
            self.process.kill()
            self.process.wait()
        self.log.close()
        self.directory.cleanup()


def wait_for(browser, expression, timeout=15):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        value = browser.evaluate(expression)
        if value:
            return value
        time.sleep(0.05)
    raise AssertionError(f"Timed out waiting for {expression}")


def run(browser, editor):
    browser.call("Page.navigate", url=editor.as_uri())
    wait_for(browser, "document.readyState === 'complete'")
    summary = browser.evaluate("({title:document.title, protocol:location.protocol})")
    assert summary["protocol"] == "file:", "The standalone page must work directly from disk"
    wait_for(browser, "typeof OpenKAIModel === 'function' && !!window.OPENKAI_SCHEMA")
    schema = json.loads((editor.parent / "OpenKAI.json").read_text())
    summary["catalog"] = browser.evaluate("""(() => {
        const source = """ + json.dumps(schema) + """;
        if (JSON.stringify(source) !== JSON.stringify(OPENKAI_SCHEMA))
            throw Error('The offline schema bundle is stale');
        let created = 0;
        for (const definition of OPENKAI_SCHEMA.classes) {
            if (!definition.creatable) continue;
            const model = new OpenKAIModel(OPENKAI_SCHEMA);
            model.addNode(definition.name);
            model.dependencies();
            model.validate();
            JSON.parse(model.export());
            created++;
        }
        return {classes: source.classes.length, created};
    })()""")
    fixtures = [
        {"path": str(path), "text": path.read_text()}
        for path in sorted((editor.parents[3] / "jsonCfg").rglob("*.json"))
    ]
    summary["fixtureRoundTrips"] = browser.evaluate("""(() => {
        const fixtures = """ + json.dumps(fixtures) + """;
        for (const fixture of fixtures) {
            const model = new OpenKAIModel();
            model.import(fixture.text);
            const actual = JSON.stringify(JSON.parse(model.export()));
            const expected = JSON.stringify(JSON.parse(OpenKAIModel.stripComments(fixture.text)));
            if (actual !== expected) throw Error('Round-trip mismatch: ' + fixture.path);
        }
        return fixtures.length;
    })()""")
    browser.evaluate((editor.parent / "tests/model.test.js").read_text())
    summary["modelTests"] = browser.evaluate("runOpenKAIModelTests()")
    assert all(test["passed"] for test in summary["modelTests"]), summary["modelTests"]
    summary["uiChecks"] = run_ui(browser, editor)
    return summary


def mouse_point(browser, selector):
    return browser.evaluate("""(() => {
        const element = document.querySelector(""" + json.dumps(selector) + """);
        if (!element) throw Error('Missing control: ' + """ + json.dumps(selector) + """);
        const rect = element.getBoundingClientRect();
        return {x: rect.left + rect.width / 2, y: rect.top + rect.height / 2};
    })()""")


def click(browser, selector):
    point = mouse_point(browser, selector)
    browser.call("Input.dispatchMouseEvent", type="mouseMoved", **point)
    browser.call("Input.dispatchMouseEvent", type="mousePressed", button="left", clickCount=1, **point)
    browser.call("Input.dispatchMouseEvent", type="mouseReleased", button="left", clickCount=1, **point)


def drag(browser, source, target=None, offset=None):
    start = mouse_point(browser, source)
    end = mouse_point(browser, target) if target else {"x": start["x"] + offset[0], "y": start["y"] + offset[1]}
    browser.call("Input.dispatchMouseEvent", type="mouseMoved", **start)
    browser.call("Input.dispatchMouseEvent", type="mousePressed", button="left", clickCount=1, **start)
    for step in range(1, 6):
        point = {axis: start[axis] + (end[axis] - start[axis]) * step / 5 for axis in ("x", "y")}
        browser.call("Input.dispatchMouseEvent", type="mouseMoved", button="left", buttons=1, **point)
    browser.call("Input.dispatchMouseEvent", type="mouseReleased", button="left", clickCount=1, **end)


def run_ui(browser, editor):
    wait_for(browser, "document.body.dataset.ready === 'true' && !!window.OpenKAIEditor")
    checks = []
    browser.evaluate(r"""(() => {
        window.smokeAssert = (value, message) => { if (!value) throw Error(message); };
        window.smokeChange = (selector, value, kind = 'change') => {
            const control = document.querySelector(selector);
            smokeAssert(control, 'Missing control: ' + selector);
            control.value = value;
            control.dispatchEvent(new Event(kind, {bubbles:true}));
        };
        window.smokeCheckArrow = (providerSelector, dependentSelector) => {
            const edge = document.querySelector('#edge-paths .edge:not(.pending)');
            smokeAssert(edge, 'Connected instances must have an arrow');
            const nearPort = (point, selector) => {
                const rect = document.querySelector(selector).getBoundingClientRect();
                const screen = point.matrixTransform(edge.getScreenCTM());
                return Math.hypot(screen.x - rect.left - rect.width / 2,
                    screen.y - rect.top - rect.height / 2) < 6;
            };
            smokeAssert(nearPort(edge.getPointAtLength(0), providerSelector), 'Arrow must start at the provider');
            smokeAssert(nearPort(edge.getPointAtLength(edge.getTotalLength()), dependentSelector), 'Arrow must end at the instance retaining the pointer');
            smokeAssert(getComputedStyle(edge).markerEnd.includes('#arrow'), 'Arrowhead must be at the dependent end');
        };
        document.getElementById('new-config').click();
        smokeChange('#class-search', '_Camera', 'input');
        const tile = document.querySelector('[data-class="_Camera"]');
        smokeAssert(tile && tile.draggable, 'Camera should be draggable from the class library');
        const dataTransfer = new DataTransfer();
        tile.dispatchEvent(new DragEvent('dragstart', {bubbles:true, dataTransfer}));
        const canvas = document.getElementById('canvas');
        const rect = canvas.getBoundingClientRect();
        canvas.dispatchEvent(new DragEvent('dragover', {bubbles:true, cancelable:true, dataTransfer}));
        canvas.dispatchEvent(new DragEvent('drop', {bubbles:true, cancelable:true, dataTransfer,
            clientX:rect.left + 350, clientY:rect.top + 100}));
        smokeAssert(OpenKAIEditor.model.getNode('/camera').className === '_Camera', 'Dropped class must create a camera');
        smokeAssert(JSON.parse(OpenKAIEditor.exportConfig()).camera.bON === true, 'New instances must default to boolean bON true');
        smokeChange('#class-search', '_Crop', 'input');
        document.querySelector('[aria-label="Add _Crop"]').click();
        smokeAssert(OpenKAIEditor.model.getNode('/crop').className === '_Crop', 'Add button must create a crop');
        smokeChange('#class-search', '', 'input');
        document.getElementById('auto-layout').click();
    })()""")
    checks.append("class search, class drag/drop and add button")
    click(browser, '[data-id="/camera"] .node-header')
    browser.evaluate(r"""(() => {
        smokeChange('[data-path=\'["deviceID"]\']', '2');
        smokeAssert(OpenKAIEditor.model.getValue('/camera',['deviceID']) === 2, 'Numeric parameter must update');
        smokeChange('[data-path=\'["vSizeRGB"]\']', '[800,600]');
        smokeAssert(JSON.stringify(OpenKAIEditor.model.getValue('/camera',['vSizeRGB'])) === '[800,600]', 'Array parameter must update');
        smokeChange('[data-path=\'["vSizeRGB"]\']', '{bad');
        smokeAssert(document.querySelector('[data-path=\'["vSizeRGB"]\']').getAttribute('aria-invalid') === 'true', 'Invalid JSON must be identified');
        smokeAssert(JSON.stringify(OpenKAIEditor.model.getValue('/camera',['vSizeRGB'])) === '[800,600]', 'Invalid parameter edit must preserve the prior value');
        smokeChange('[data-path=\'["vSizeRGB"]\']', '[800,600]');
    })()""")
    checks.append("typed parameter editing and invalid-value preservation")
    source = '[data-id="/crop"] .dependency-port[data-path=\'["_VisionBase"]\']'
    target = '[data-id="/camera"] .provider-port'
    click(browser, source)
    click(browser, target)
    browser.evaluate("smokeAssert(OpenKAIEditor.model.getValue('/crop',['_VisionBase']) === 'camera', 'Click ports must connect dependency')")
    browser.evaluate("smokeCheckArrow(" + json.dumps(target) + "," + json.dumps(source) + ")")
    click(browser, '[data-id="/crop"] .node-header')
    browser.evaluate("document.querySelector('[aria-label=" + json.dumps('Disconnect camera') + "]').click()")
    browser.evaluate("smokeAssert(OpenKAIEditor.model.getValue('/crop',['_VisionBase']) === undefined, 'Scalar dependency must disconnect')")
    drag(browser, source, target)
    browser.evaluate("smokeAssert(OpenKAIEditor.model.getValue('/crop',['_VisionBase']) === 'camera', 'Dragging ports must connect dependency')")
    browser.evaluate("smokeCheckArrow(" + json.dumps(target) + "," + json.dumps(source) + ")")
    checks.append("pointer click and drag connections, provider-to-dependent arrows, scalar disconnect")
    click(browser, '[data-id="/camera"] .node-header')
    browser.evaluate(r"""(() => {
        smokeChange('#instance-name', 'frontCamera');
        smokeAssert(OpenKAIEditor.model.getValue('/crop',['_VisionBase']) === 'frontCamera', 'Rename must update references');
        document.getElementById('undo').click();
        smokeAssert(OpenKAIEditor.model.getValue('/crop',['_VisionBase']) === 'camera', 'Undo must restore references');
        document.getElementById('redo').click();
        smokeAssert(OpenKAIEditor.model.getValue('/crop',['_VisionBase']) === 'frontCamera', 'Redo must restore rename');
        document.getElementById('auto-layout').click();
    })()""")
    position = browser.evaluate("document.querySelector('[data-id=\"/frontCamera\"]').style.cssText")
    drag(browser, '[data-id="/frontCamera"] .node-header', offset=(22, 15))
    assert browser.evaluate("document.querySelector('[data-id=\"/frontCamera\"]').style.cssText") != position
    checks.append("rename reference updates, undo/redo, node dragging")
    browser.evaluate(r"""(() => {
        smokeChange('#class-search', '_Console', 'input');
        document.querySelector('[aria-label="Add _Console"]').click();
        smokeChange('[aria-label="Provider for vBASE"]', '/frontCamera');
        document.querySelector('[aria-label="Add connection for vBASE"]').click();
        smokeChange('[aria-label="Provider for vBASE"]', '/crop');
        document.querySelector('[aria-label="Add connection for vBASE"]').click();
        smokeAssert(JSON.stringify(OpenKAIEditor.model.getValue('/console',['vBASE'])) === '["frontCamera","crop"]', 'List dependency must append providers');
        document.querySelector('[aria-label="Disconnect frontCamera"]').click();
        smokeAssert(JSON.stringify(OpenKAIEditor.model.getValue('/console',['vBASE'])) === '["crop"]', 'Disconnect must remove exactly one list reference');
        smokeChange('#class-search', '', 'input');
    })()""")
    checks.append("multiple dependency append and single-reference removal")
    browser.evaluate(r"""(() => {
        OpenKAIEditor.loadDocument({APP:{class:'ModuleMgr'}, points:{class:'_PointCloud'},
            web:{class:'_WebGeometryBase',vGeometry:[{_GeometryBase:'points',label:'preserve'}]},
            extension:{release:17}, future:{class:'_FuturePlugin',opaque:{rows:[1,{text:'custom'}]},'/comment':'keep'}});
        OpenKAIEditor.selectNode('/web');
        smokeChange('[aria-label="Provider for vGeometry.*._GeometryBase"]', '/points');
        document.querySelector('[aria-label="Add connection for vGeometry.*._GeometryBase"]').click();
        let config = JSON.parse(OpenKAIEditor.exportConfig());
        smokeAssert(config.web.vGeometry.length === 2 && config.web.vGeometry[0].label === 'preserve', 'Wildcard connect must preserve sibling metadata');
        document.querySelector('[aria-label="Disconnect points"]').click();
        config = JSON.parse(OpenKAIEditor.exportConfig());
        smokeAssert(config.web.vGeometry[0].label === 'preserve' && !('_GeometryBase' in config.web.vGeometry[0]), 'Wildcard disconnect must preserve its object');
        smokeAssert(config.future.opaque.rows[1].text === 'custom' && config.extension.release === 17, 'Unknown fields must survive graph editing');
        document.getElementById('paste-config').click();
        document.getElementById('json-text').value = '{bad';
        document.getElementById('apply-json').click();
        smokeAssert(document.getElementById('json-dialog').open && document.getElementById('json-error').textContent, 'Invalid pasted JSON must remain editable');
        smokeAssert(JSON.parse(OpenKAIEditor.exportConfig()).extension.release === 17, 'Invalid import must preserve previous config');
        document.getElementById('json-text').value = JSON.stringify(config);
        document.getElementById('apply-json').click();
        smokeAssert(!document.getElementById('json-dialog').open, 'Valid pasted JSON must apply');
    })()""")
    checks.append("wildcard dependencies, unknown fields, valid/invalid paste import")
    browser.evaluate(r"""(() => {
        OpenKAIEditor.loadDocument({APP:{class:'ModuleMgr'},motor:{class:'_DDSM'},
            drive:{class:'_ApDrive',motors:{left:{kSpd:2,label:'keep'}}}});
        OpenKAIEditor.selectNode('/drive');
        smokeChange('[data-path=\'["motors","left","kSpd"]\']', '3');
        smokeChange('[aria-label="Provider for motors.*._ActuatorBase"]', '/motor');
        document.querySelector('[aria-label="Add connection for motors.*._ActuatorBase"]').click();
        const motors = OpenKAIEditor.model.getValue('/drive',['motors']);
        smokeAssert(motors.left._ActuatorBase === 'motor' && motors.left.kSpd === 3 && motors.left.label === 'keep', 'Named map connection must preserve row parameters');
        document.querySelector('[data-map-path]').value = 'right';
    })()""")
    drag(browser, '[data-id="/drive"] .dependency-port[data-path=\'["motors","*","_ActuatorBase"]\']',
         '[data-id="/motor"] .provider-port')
    browser.evaluate(r"""(() => {
        smokeAssert(OpenKAIEditor.model.getValue('/drive',['motors','right','_ActuatorBase']) === 'motor', 'Map port connection must use the selected key');
        OpenKAIEditor.loadDocument({APP:{class:'ModuleMgr'},motor:{class:'_DDSM'},drive:{class:'_ApDrive'}});
        OpenKAIEditor.selectNode('/drive');
        smokeChange('[aria-label="Provider for motors.*._ActuatorBase"]', '/motor');
        document.querySelector('[aria-label="Add connection for motors.*._ActuatorBase"]').click();
        smokeAssert(OpenKAIEditor.model.getValue('/drive',['motors']) === undefined, 'Missing map key must not modify the config');
        document.querySelector('[data-map-path]').value = '0';
        document.querySelector('[aria-label="Add connection for motors.*._ActuatorBase"]').click();
        const motors = OpenKAIEditor.model.getValue('/drive',['motors']);
        smokeAssert(motors && !Array.isArray(motors) && motors['0']._ActuatorBase === 'motor', 'Numeric object-map keys must remain object keys');
    })()""")
    checks.append("object-map row editing, picker/port connections and numeric keys")
    browser.evaluate(r"""(() => {
        OpenKAIEditor.loadDocument({APP:{class:'ModuleMgr'},camera:{class:'_Camera',bON:false},
            crop:{class:'_Crop',_VisionBase:'camera'}});
        const selector = '[data-path=\'["bON"]\']';
        const disabledCard = () => document.querySelector('[data-id="/camera"]').classList.contains('disabled-node');
        const disabledWarning = () => OpenKAIEditor.model.validate().some(item => item.code === 'disabled-dependency');
        OpenKAIEditor.selectNode('/camera');
        smokeAssert(document.querySelector(selector).tagName === 'SELECT' && document.querySelector(selector).value === 'false', 'bON must use a boolean selector');
        smokeAssert(disabledCard() && disabledWarning(), 'bON false must disable the card and warn about dependent instances');
        OpenKAIEditor.selectNode('/crop');
        smokeAssert(document.querySelector('[aria-label="Provider for _VisionBase"] option[value="/camera"]').textContent.includes('(disabled)'), 'Provider picker must label a false bON provider disabled');
        OpenKAIEditor.selectNode('/camera');
        smokeChange(selector, 'true');
        smokeAssert(JSON.parse(OpenKAIEditor.exportConfig()).camera.bON === true, 'Enabling a module must export boolean true');
        smokeAssert(!disabledCard() && !disabledWarning(), 'bON true must enable the card and clear disabled warnings');
        smokeChange(selector, 'false');
        smokeAssert(JSON.parse(OpenKAIEditor.exportConfig()).camera.bON === false, 'Disabling a module must export boolean false');
        smokeAssert(disabledCard() && disabledWarning(), 'Changing bON to false must refresh disabled indicators');
        smokeChange(selector, '');
        smokeAssert(!Object.hasOwn(JSON.parse(OpenKAIEditor.exportConfig()).camera, 'bON'), 'Runtime default must omit bON');
        smokeAssert(!disabledCard() && !disabledWarning(), 'Omitted bON must use the enabled runtime default');
        const legacy = JSON.parse(OpenKAIEditor.exportConfig());
        legacy.camera.bON = 0;
        OpenKAIEditor.loadDocument(legacy);
        OpenKAIEditor.selectNode('/camera');
        smokeAssert(document.querySelector(selector).selectedOptions[0].textContent === 'Existing value: 0', 'Legacy integer must remain visible for correction');
        smokeAssert(JSON.parse(OpenKAIEditor.exportConfig()).camera.bON === 0, 'Import must preserve legacy integer until edited');
        smokeAssert(OpenKAIEditor.model.validate().some(item => item.code === 'parameter-type' && item.path[0] === 'bON'), 'Legacy integer must be flagged as a type mismatch');
        smokeChange(selector, 'false');
        smokeAssert(JSON.parse(OpenKAIEditor.exportConfig()).camera.bON === false, 'Boolean selector must correct the legacy integer');
        smokeAssert(!OpenKAIEditor.model.validate().some(item => item.code === 'parameter-type' && item.path[0] === 'bON'), 'Boolean correction must clear the type warning');
    })()""")
    checks.append("boolean bON defaults, editing/export, disabled indicators and legacy integer correction")
    fixture = editor.parents[3] / 'jsonCfg/helloOK.json'
    root = browser.call("DOM.getDocument")["root"]["nodeId"]
    control = browser.call("DOM.querySelector", nodeId=root, selector="#config-file")["nodeId"]
    browser.call("DOM.setFileInputFiles", nodeId=control, files=[str(fixture)])
    wait_for(browser, "document.getElementById('filename').value === 'helloOK.json'")
    assert json.loads(browser.evaluate("OpenKAIEditor.exportConfig()")) == json.loads(fixture.read_text())
    checks.append("native file input import preserves the complete config")
    browser.evaluate("document.getElementById('copy-config').click()")
    wait_for(browser, "document.getElementById('status').textContent.includes('copied') || document.getElementById('status').textContent.includes('selected')")
    browser.evaluate("document.getElementById('json-dialog').close()")
    with tempfile.TemporaryDirectory(prefix="openkai-editor-download-") as directory:
        browser.call("Page.setDownloadBehavior", behavior="allow", downloadPath=directory)
        browser.evaluate("document.getElementById('download-config').click()")
        target_file = Path(directory) / 'helloOK.json'
        deadline = time.monotonic() + 10
        while not target_file.exists() and time.monotonic() < deadline:
            time.sleep(0.05)
        assert target_file.exists(), 'Download button must produce a local JSON file'
        assert json.loads(target_file.read_text()) == json.loads(fixture.read_text())
    checks.append("clipboard action and actual JSON file download")
    browser.evaluate(r"""(() => {
        OpenKAIEditor.loadDocument({APP:{class:'ModuleMgr'},camera:{class:'_Camera',thread:{class:'_Thread',FPS:30}}});
        OpenKAIEditor.selectNode('/camera/thread');
        document.getElementById('show-json').click();
        document.getElementById('json-text').value = JSON.stringify({APP:{class:'ModuleMgr'},camera:{class:'_Camera',sentinel:21}});
        document.getElementById('apply-json').click();
        smokeAssert(OpenKAIEditor.model.nodes().length === 2, 'Replacing document must remove deleted nested nodes');
        smokeAssert(!document.querySelector('.graph-node.selected'), 'Replacing selected nested node must clear selection');
    })()""")
    wait_for(browser, "JSON.parse(localStorage.getItem('openkai-config-editor-v1'))?.document.camera?.sentinel === 21")
    expected = json.loads(browser.evaluate("OpenKAIEditor.exportConfig()"))
    browser.call("Page.reload", ignoreCache=True)
    wait_for(browser, "document.body.dataset.ready === 'true' && document.getElementById('status').textContent.includes('Restored')")
    assert json.loads(browser.evaluate("OpenKAIEditor.exportConfig()")) == expected
    checks.append("selected nested-node replacement and draft restoration after reload")
    browser.evaluate("document.getElementById('load-example').click();document.getElementById('auto-layout').click();OpenKAIEditor.selectNode('/crop')")
    return checks


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--page", type=Path, default=Path(__file__).resolve().parents[1] / "index.html")
    parser.add_argument("--evaluate", type=Path, help="Run additional async JavaScript from a file")
    parser.add_argument("--screenshot", type=Path, help="Save the final browser view as a PNG")
    args = parser.parse_args()
    if not args.page.is_file():
        parser.error(f"Editor page does not exist: {args.page}")
    with Browser() as browser:
        result = run(browser, args.page.resolve())
        print(json.dumps(result, indent=2))
        if args.evaluate:
            print(json.dumps(browser.evaluate(args.evaluate.read_text()), indent=2))
        if args.screenshot:
            screenshot = browser.call("Page.captureScreenshot", format="png")
            args.screenshot.write_bytes(base64.b64decode(screenshot["data"]))
        exceptions = [event["params"] for event in browser.events
                      if event.get("method") == "Runtime.exceptionThrown"]
        errors = [event["params"]["entry"] for event in browser.events
                  if event.get("method") == "Log.entryAdded"
                  and event["params"]["entry"].get("level") == "error"]
        console_errors = [event["params"] for event in browser.events
                          if event.get("method") == "Runtime.consoleAPICalled"
                          and event["params"].get("type") == "error"]
        external_requests = [event["params"]["request"]["url"] for event in browser.events
                             if event.get("method") == "Network.requestWillBeSent"
                             and event["params"]["request"]["url"].startswith(("https:", "http:"))]
        if exceptions or errors or console_errors or external_requests:
            raise AssertionError(json.dumps({"exceptions": exceptions, "errors": errors,
                                             "consoleErrors": console_errors,
                                             "externalRequests": external_requests}, indent=2))
    print("PASS: standalone editor loaded offline without browser errors")


if __name__ == "__main__":
    main()
