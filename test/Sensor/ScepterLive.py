#!/usr/bin/env python3
"""Exercise Scepter Refresh/Edit/Save in its real browser while clouds stream.

Run from the repository root with the camera connected and other apps stopped:
  python3 test/Sensor/ScepterLive.py build/OpenKAI --ip 192.168.16.101

Launches only camera, geometry and command modules on temporary localhost ports.
A command client connects before Chrome to reproduce another viewer already open.
Only the host-side color cache age is edited; Save writes a temporary launch JSON.
The original age is restored before shutdown. Requires Chrome/Chromium, no Python
dependencies. Use --single-client to isolate SDK command scheduling from routing.
"""

import argparse
import base64
import importlib.util
import json
from pathlib import Path
import shutil
import signal
import subprocess
import tempfile
import time
import uuid


ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "line_scanner_live", ROOT / "test/Sensor/LineScanner/live.py")
HELPERS = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(HELPERS)


class Commands:
    def __init__(self, port):
        self.ws = HELPERS.WebSocket(f"ws://127.0.0.1:{port}/", timeout=12)
        self.buffer = ""

    def request(self):
        request_id = f"scepter-live-{uuid.uuid4().hex}"
        self.ws.send(json.dumps(dict(module="scepter", cmd="getConfig",
                                     requestId=request_id)) + "EOJ")
        started = time.monotonic()
        while time.monotonic() - started < 12:
            self.buffer = self.buffer.lstrip()
            if self.buffer.startswith("EOJ"):
                self.buffer = self.buffer[3:]
                continue
            try:
                reply, end = json.JSONDecoder().raw_decode(self.buffer)
            except ValueError:
                incoming = self.ws.receive()
                assert isinstance(incoming, str), "Expected text command response"
                self.buffer += incoming
                continue
            self.buffer = self.buffer[end:]
            if reply.get("requestId") == request_id:
                assert reply.get("bSuccess"), reply
                assert reply.get("deviceOpen"), reply
                print(f"First command client: getConfig in {time.monotonic() - started:.3f}s",
                      flush=True)
                return reply
        raise TimeoutError("First command client received no matching getConfig reply")

    def close(self):
        self.ws.close()


class Browser:
    def __init__(self, temp):
        chrome = (shutil.which("google-chrome") or shutil.which("chromium") or
                  shutil.which("chromium-browser"))
        if not chrome:
            raise RuntimeError("Chrome/Chromium required for browser validation")
        debug_port = HELPERS.free_port()
        self.process = subprocess.Popen([
            chrome, "--headless", "--no-sandbox", "--disable-dev-shm-usage",
            "--enable-unsafe-swiftshader", "--no-first-run", "--no-default-browser-check",
            "--window-size=1500,1000", f"--user-data-dir={temp}/chrome",
            f"--remote-debugging-port={debug_port}", "about:blank",
        ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
        self.cdp = None
        self.serial = 0
        self.errors = []
        try:
            pages = json.loads(HELPERS.wait_http(
                f"http://127.0.0.1:{debug_port}/json", self.process))
            self.cdp = HELPERS.WebSocket(next(
                page["webSocketDebuggerUrl"] for page in pages if page["type"] == "page"))
            self.call("Runtime.enable")
            self.call("Page.enable")
            # Observe the actual viewer's binary messages, including RGB bytes.
            self.call("Page.addScriptToEvaluateOnNewDocument", source="""
              window.__scepterLiveCloud = {frames: 0, colored: 0, points: 0, last: 0};
              const NativeWebSocket = window.WebSocket;
              window.WebSocket = class extends NativeWebSocket {
                constructor(...args) {
                  super(...args);
                  if (new URL(args[0], location.href).pathname !== '/stream/points') return;
                  this.addEventListener('message', event => {
                    if (!(event.data instanceof ArrayBuffer)) return;
                    const v = new DataView(event.data), rgb = new Uint8Array(event.data);
                    if (v.byteLength < 32 || v.getUint32(0, true) !== 0x36443357 ||
                        v.getUint32(4, true) !== 6 || v.getUint32(8, true) !== 1 ||
                        v.getUint32(20, true) !== v.byteLength) throw Error('Bad cloud header');
                    let at = 32, count = 0, colored = false;
                    for (let n = v.getUint32(16, true); n > 0; --n) {
                      const size = v.getUint32(at + 4, true);
                      at += 40;
                      for (let i = at + size * 12; i < at + size * 15 && !colored; i += 3)
                        colored = rgb[i] !== rgb[i + 1] || rgb[i + 1] !== rgb[i + 2];
                      count += size;
                      at += Math.ceil(size * 15 / 4) * 4;
                    }
                    if (at !== v.byteLength) throw Error('Bad cloud length');
                    if (!count) return;
                    const s = window.__scepterLiveCloud;
                    ++s.frames; if (colored) ++s.colored;
                    s.points = count; s.last = performance.now();
                  });
                }
              };
            """)
        except BaseException:
            self.close()
            raise

    def call(self, method, **params):
        self.serial += 1
        self.cdp.send(json.dumps(dict(id=self.serial, method=method, params=params)))
        while True:
            result = json.loads(self.cdp.receive())
            if result.get("method") == "Runtime.exceptionThrown":
                self.errors.append(result["params"])
            if result.get("id") == self.serial:
                assert "error" not in result, result
                return result.get("result", {})

    def evaluate(self, expression):
        result = self.call("Runtime.evaluate", expression=expression,
                           returnByValue=True, awaitPromise=True)
        assert "exceptionDetails" not in result, result
        return result.get("result", {}).get("value")

    def state(self):
        return self.evaluate("""({
          connected: window.wsSocket?.readyState === WebSocket.OPEN,
          controls: document.querySelectorAll('[data-key]').length,
          enabled: document.querySelector('#cameraFields')?.disabled === false,
          status: document.querySelector('#configStatus')?.textContent,
          stream: document.querySelector('#status')?.textContent,
          age: document.querySelector('#param-pclColorMaxAgeMs')?.value,
          saveEnabled: document.querySelector('#saveConfig')?.disabled === false,
          stats: document.querySelector('#stats')?.textContent,
          canvas: !!document.querySelector('canvas'),
          cloud: window.__scepterLiveCloud,
          commandLog: document.querySelector('#cmdState')?.value
        })""")

    def wait(self, description, predicate, timeout=12):
        started = time.monotonic()
        state = {}
        while time.monotonic() - started < timeout:
            state = self.state()
            assert not self.errors, self.errors
            if state and predicate(state):
                print(f"Browser {description}: {time.monotonic() - started:.3f}s", flush=True)
                return state
            time.sleep(0.1)
        raise TimeoutError(f"Browser {description}: {json.dumps(state)}")

    def set_age(self, value):
        self.evaluate(f"""(() => {{
          const input = document.querySelector('#param-pclColorMaxAgeMs');
          if (!input || input.disabled || document.querySelector('#cameraFields').disabled)
            throw Error('Color age field unavailable');
          input.value = {json.dumps(str(value))};
          input.dispatchEvent(new Event('change', {{bubbles: true}}));
        }})()""")
        return self.wait("apply color age", lambda s: s.get("enabled") and
                         s.get("age") == str(value) and s.get("status", "").startswith("Applied"))

    def close(self):
        if self.cdp:
            self.cdp.close()
        stop_process(self.process, signal.SIGTERM)


def stop_process(process, stop_signal=signal.SIGINT):
    if process.poll() is not None:
        return
    process.send_signal(stop_signal)
    try:
        process.wait(timeout=8)
    except subprocess.TimeoutExpired:
        process.kill()
        process.wait(timeout=5)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("--ip", default="192.168.16.101")
    parser.add_argument("--single-client", action="store_true")
    parser.add_argument("--screenshot", type=Path, default=Path("/tmp/openkai-scepter-live.png"))
    parser.add_argument("--log", type=Path, default=Path("/tmp/openkai-scepter-live.log"))
    args = parser.parse_args()
    source = json.loads((ROOT / "samples/AProver_LiDAR/AProver_LiDAR.json").read_text())
    config = {key: source[key] for key in (
        "APP", "scepter", "scepterTransform", "scPC", "camRGB", "camDepth",
        "scepterViewer", "wsConsole", "wsServer", "wsServer_rx", "wsServer_tx")}
    config["scepter"]["devURI"] = args.ip
    port, command_port = HELPERS.free_port(), HELPERS.free_port()
    while command_port == port:
        command_port = HELPERS.free_port()
    config["scepterViewer"].update(host="127.0.0.1", port=port)
    config["wsServer"].update(host="127.0.0.1", port=command_port)
    config["wsConsole"]["vBASE"] = ["scepter"]
    with tempfile.TemporaryDirectory(prefix="openkai-scepter-live-") as temp:
        launch = Path(temp) / "Scepter.json"
        launch.write_text(json.dumps(config, indent=2))
        browser = commands = None
        original_age = None
        with args.log.open("w") as log:
            process = subprocess.Popen([str(args.binary.resolve()), str(launch)],
                                       cwd=ROOT, stdout=log, stderr=log)
            try:
                HELPERS.wait_http(f"http://127.0.0.1:{port}/", process)
                if not args.single_client:
                    commands = Commands(command_port)
                browser = Browser(temp)
                browser.call("Page.navigate", url=f"http://127.0.0.1:{port}/?cmdPort={command_port}")
                browser.wait("page ready", lambda s: s.get("canvas"))
                browser.evaluate("document.querySelector('#start').click()")
                state = browser.wait("initial config and colored cloud", lambda s:
                                     s.get("connected") and s.get("enabled") and
                                     s.get("controls", 0) > 40 and s.get("cloud", {}).get("colored", 0) >= 3,
                                     timeout=30)
                original_age = int(state["age"])
                if commands:
                    commands.request()
                browser.evaluate("document.querySelector('#getConfig').click()")
                browser.wait("Refresh config", lambda s: s.get("enabled") and
                             s.get("status") == "Current parameters loaded")
                age = 1700 if original_age != 1700 else 1800
                browser.set_age(age)
                browser.evaluate("document.querySelector('#saveConfig').click()")
                state = browser.wait("Save config", lambda s: s.get("enabled") and
                                     s.get("status") == "Configuration saved to disk")
                baseline = state["cloud"].copy()
                saved = json.loads(launch.read_text())
                assert saved["scepter"]["pclColorMaxAgeMs"] == age, saved["scepter"]
                print(f"Temporary launch JSON saved pclColorMaxAgeMs={age}", flush=True)
                if commands:
                    assert commands.request()["config"]["pclColorMaxAgeMs"] == age
                state = browser.wait("colored streaming after Save", lambda s:
                                     s.get("cloud", {}).get("colored", 0) >= baseline["colored"] + 3 and
                                     s.get("stream") == "Connected")
                screenshot = browser.call("Page.captureScreenshot", format="png")
                args.screenshot.write_bytes(base64.b64decode(screenshot["data"]))
                print("Browser result:", json.dumps({key: state[key] for key in
                      ("controls", "status", "stream", "stats", "cloud")}), flush=True)
            finally:
                try:
                    if browser and original_age is not None:
                        browser.set_age(original_age)
                        print(f"Restored runtime color age to {original_age}ms", flush=True)
                finally:
                    try:
                        if browser:
                            browser.close()
                    finally:
                        try:
                            if commands:
                                commands.close()
                        finally:
                            stop_process(process)
    print(f"Scepter browser/config regression passed. Log: {args.log}; screenshot: {args.screenshot}")


if __name__ == "__main__":
    main()
