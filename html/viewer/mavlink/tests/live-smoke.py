#!/usr/bin/env python3
"""Exercise real UDP MAVLink -> MavlinkStream -> WebSocket -> Cesium/three.js.

Uses the example configuration with temporary loopback ports, Chrome and only
Python's standard library. No autopilot is contacted and no command is sent.
"""
import argparse
import base64
import errno
import importlib.util
import json
import math
from pathlib import Path
import signal
import socket
import struct
import subprocess
import tempfile
import threading
import time
import urllib.request

ROOT = Path(__file__).resolve().parents[4]
spec = importlib.util.spec_from_file_location(
    "browser_tools", ROOT / "html/console/editor/tests/browser-smoke.py")
browser_tools = importlib.util.module_from_spec(spec)
spec.loader.exec_module(browser_tools)


class Browser(browser_tools.Browser):
    def __exit__(self, *args):
        try:
            super().__exit__(*args)
        except OSError as error:
            if error.errno != errno.ENOTEMPTY:
                raise
            # Chrome child processes may finish profile writes after exit.
            for attempt in range(20):
                time.sleep(0.1)
                try:
                    self.directory.cleanup()
                    return
                except OSError as retry:
                    if retry.errno != errno.ENOTEMPTY or attempt == 19:
                        raise


def free_port(kind):
    with socket.socket(socket.AF_INET, kind) as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def mavlink_packet(sequence, message_id, payload, crc_extra):
    header = bytes((len(payload), sequence % 256, 1, 1, message_id))
    crc = 0xFFFF
    for value in header + payload + bytes((crc_extra,)):
        tmp = value ^ (crc & 0xFF)
        tmp ^= (tmp << 4) & 0xFF
        crc = (crc >> 8) ^ (tmp << 8) ^ (tmp << 3) ^ (tmp >> 4)
    return b"\xfe" + header + payload + struct.pack("<H", crc)


class Telemetry:
    def __init__(self, port):
        self.port = port
        self.stop = threading.Event()
        self.thread = threading.Thread(target=self.run, daemon=True)

    def run(self):
        sequence = 0
        with socket.socket(socket.AF_INET, socket.SOCK_DGRAM) as sock:
            while not self.stop.is_set():
                boot_ms = int(time.monotonic() * 1000) % (2 ** 32)
                messages = [
                    (0, struct.pack("<IBBBBB", 4, 2, 3, 128, 4, 3), 50),
                    (30, struct.pack("<Iffffff", boot_ms, math.radians(15),
                                     math.radians(-10), math.radians(90), 0, 0, 0), 39),
                    (33, struct.pack("<IiiiihhhH", boot_ms, 356812000, 1397671000,
                                     120000, 60000, 300, 400, -50, 9000), 104),
                    (1, struct.pack("<IIIHHhHHHHHHb", 0, 0, 0, 100, 15800, 230,
                                    0, 0, 0, 0, 0, 0, 76), 124),
                ]
                for mid, payload, crc in messages:
                    sock.sendto(mavlink_packet(sequence, mid, payload, crc), ("127.0.0.1", self.port))
                    sequence += 1
                self.stop.wait(0.05)

    def close(self):
        self.stop.set()
        if self.thread.is_alive():
            self.thread.join(timeout=2)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", type=Path, default=ROOT / "build/OpenKAI")
    parser.add_argument("--models-root", type=Path, default=Path("/home/kai/dev/models/webMavlink"))
    parser.add_argument("--screenshot", type=Path)
    parser.add_argument("--overview-screenshot", type=Path, help="Also verify buildings from a wider Chiyoda view")
    args = parser.parse_args()
    config = json.loads((ROOT / "jsonCfg/WebMavlinkStream.json").read_text())
    port, udp_port = free_port(socket.SOCK_STREAM), free_port(socket.SOCK_DGRAM)
    config["viewer"].update(host="127.0.0.1", port=port,
                            webRoot=str(ROOT / "html/viewer/mavlink"), modelsRoot=str(args.models_root))
    config["udpMavlink"]["portLocal"] = udp_port
    telemetry = Telemetry(udp_port)
    with tempfile.TemporaryDirectory(prefix="openkai-mavlink-live-") as temp:
        cfg = Path(temp) / "viewer.json"
        cfg.write_text(json.dumps(config))
        with (Path(temp) / "backend.log").open("w+") as log:
            process = subprocess.Popen([str(args.executable.resolve()), str(cfg)], cwd=ROOT,
                                       stdout=log, stderr=subprocess.STDOUT)
            try:
                url = f"http://localhost:{port}/"
                deadline = time.monotonic() + 20
                while True:
                    try:
                        with urllib.request.urlopen(url, timeout=1) as response:
                            assert response.status == 200
                        break
                    except OSError:
                        if process.poll() is not None or time.monotonic() > deadline:
                            log.seek(0)
                            raise AssertionError("Backend did not start:\n" + log.read()[-5000:])
                        time.sleep(0.1)
                with Browser() as browser:
                    browser.call("Network.emulateNetworkConditions", offline=False, latency=0,
                                 downloadThroughput=-1, uploadThroughput=-1)
                    browser.call("Page.navigate", url=url)
                    browser_tools.wait_for(browser, "document.querySelector('#start') && !document.querySelector('#start').disabled", timeout=30)
                    unit_tests = browser.evaluate("(async()=>{const tests=await import('./tests/unit-tests.js');return tests.runTests()})()")
                    browser.evaluate("document.querySelector('#start').click()")
                    # Empty startup must not fabricate an aircraft at (0,0).
                    time.sleep(0.5)
                    assert browser.evaluate("document.querySelector('#latitude').textContent") == "—"
                    telemetry.thread.start()
                    browser_tools.wait_for(browser, "document.querySelector('#latitude').textContent.includes('35.6812')", timeout=20)
                    values = browser.evaluate("Object.fromEntries(['roll','pitch','yaw','latitude','longitude','altitude','speed','battery'].map(id=>[id,document.getElementById(id).textContent]))")
                    assert "15" in values["roll"] and "-10" in values["pitch"] and "90" in values["yaw"], values
                    assert "120" in values["altitude"] and "5.0" in values["speed"], values
                    assert "76" in values["battery"], values
                    browser_tools.wait_for(browser, "document.querySelectorAll('canvas').length >= 2")
                    browser_tools.wait_for(browser, "(async()=>{const {map,attitudePreview}=await import('./js/main.js');return map.model?.ready && map.model.show && attitudePreview?.model?.visible && map.buildings.length > 0})()", timeout=30)
                    time.sleep(4)
                    browser.evaluate("document.querySelector('#center').click()")
                    # Building textures refine progressively; all selected tiles may
                    # exceed a headless GPU's cache. Require actual ready content.
                    browser_tools.wait_for(browser, "(async()=>{const {map}=await import('./js/main.js');return map.viewer.scene.globe.tilesLoaded && map.buildings.every(t=>t.statistics.numberOfTilesWithContentReady > 0)})()", timeout=30)
                    if args.screenshot:
                        args.screenshot.parent.mkdir(parents=True, exist_ok=True)
                        args.screenshot.write_bytes(base64.b64decode(browser.call("Page.captureScreenshot", format="png")["data"]))
                    scene = browser.evaluate("document.querySelector('#scene-status').textContent")
                    assert browser.evaluate("document.querySelectorAll('#scene-status [data-state=error]').length") == 0, scene
                    render_state = browser.evaluate("(async()=>{const {map}=await import('./js/main.js');return {tiles:map.buildings.reduce((n,t)=>n+t.statistics.numberOfTilesWithContentReady,0),webgl:map.viewer.scene.context.webgl2,model:map.model.ready}})()")
                    assert render_state["tiles"] > 0 and render_state["model"], render_state
                    if args.overview_screenshot:
                        browser.evaluate("""(async()=>{
                            const {map}=await import('./js/main.js'), C=globalThis.Cesium;
                            window.renderedWesternBuildings=false;
                            for (const tileset of map.buildings) tileset.tileVisible.addEventListener(tile=>{
                                const location=C.Cartographic.fromCartesian(tile.boundingSphere.center);
                                if (C.Math.toDegrees(location.longitude)<139.745) window.renderedWesternBuildings=true;
                            });
                            map.viewer.camera.lookAtTransform(C.Matrix4.IDENTITY);
                            map.viewer.camera.setView({destination:C.Cartesian3.fromDegrees(139.756,35.687,3500),
                                orientation:{heading:0,pitch:-Math.PI/2,roll:0}});
                            map.viewer.scene.requestRender();
                        })()""")
                        browser_tools.wait_for(browser, "window.renderedWesternBuildings", timeout=45)
                        browser_tools.wait_for(browser, "(async()=>{const {map}=await import('./js/main.js');return map.viewer.scene.globe.tilesLoaded})()", timeout=30)
                        args.overview_screenshot.parent.mkdir(parents=True, exist_ok=True)
                        args.overview_screenshot.write_bytes(base64.b64decode(browser.call("Page.captureScreenshot", format="png")["data"]))
                        assert browser.evaluate("document.querySelectorAll('#scene-status [data-state=error]').length") == 0
                    assert not any(event.get("method") == "Runtime.exceptionThrown" for event in browser.events), [
                        event for event in browser.events if event.get("method") == "Runtime.exceptionThrown"]
                    # Chrome's host resolver blocks the internet; also reject any attempted external requests.
                    external = [event["params"]["request"]["url"] for event in browser.events
                                if event.get("method") == "Network.requestWillBeSent"
                                and event["params"]["request"]["url"].startswith(("http://", "https://"))
                                and not event["params"]["request"]["url"].startswith(url)]
                    assert not external, external
                    assert not any(event.get("method") == "Network.webSocketFrameSent" for event in browser.events), "Viewer sent an application message"
                    telemetry.close()
                    browser_tools.wait_for(browser, "document.querySelector('#vehicle-state').textContent.includes('STALE')", timeout=10)
                    browser.evaluate("document.querySelector('#stop').click()")
                    browser_tools.wait_for(browser, "document.querySelector('#latitude').textContent === '—'")
                    browser.evaluate("document.querySelector('#start').click()")
                    time.sleep(0.5)
                    assert browser.evaluate("document.querySelector('#latitude').dataset.stale === 'true' || document.querySelector('#latitude').textContent === '—'"), "Reconnect revived stale position"
                    assert browser.evaluate("(async()=>{const {map}=await import('./js/main.js');return !map.model?.show})()"), "Reconnect showed stale aircraft as live"
                    print(json.dumps({"telemetry": values, "scene": scene, "offline": True,
                                      "readOnly": True, "staleAndReconnect": True,
                                      "unitTests": unit_tests}, indent=2))
            finally:
                telemetry.close()
                process.send_signal(signal.SIGINT)
                try:
                    process.wait(timeout=10)
                except subprocess.TimeoutExpired:
                    process.kill()
                    process.wait()


if __name__ == "__main__":
    main()
