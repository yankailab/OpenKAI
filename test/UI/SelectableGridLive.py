#!/usr/bin/env python3
"""Verify live Scepter occupancy and mouse selection in the selectable-grid UI.

Run with the camera connected and other camera apps stopped:
  python3 test/UI/SelectableGridLive.py build/OpenKAI --ip 192.168.16.101

Uses only camera, grid, viewer and command modules from AProver_LiDAR on temporary
localhost ports. No autopilot or rover modules run. Sending a selection writes
only a temporary launch JSON. Requires Chrome/Chromium; no Python dependencies.
"""

import argparse
import base64
import importlib.util
import json
from pathlib import Path
import subprocess
import tempfile
import time


ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "scepter_live", ROOT / "test/Sensor/ScepterLive.py")
LIVE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(LIVE)


class GridBrowser(LIVE.Browser):
    def state(self):
        return self.evaluate("""(() => {
          const viewer = window.__gridLiveViewer;
          return {
            ready: !!document.querySelector('#viewport canvas') &&
              document.querySelector('#start')?.disabled === false,
            stream: document.querySelector('#status')?.textContent,
            stats: document.querySelector('#stats')?.textContent,
            connected: window.wsSocket?.readyState === WebSocket.OPEN,
            occupied: viewer ? [...viewer.objects.values()].reduce((n, object) =>
              n + (object.boxes?.geometry.instanceCount || 0), 0) : 0,
            picked: viewer?.picker.count || 0,
            selections: viewer?.picker.commands() || [],
            pickerStatus: document.querySelector('#picker-status')?.textContent,
            gridFrames: window.__gridLiveFrames,
            cloud: window.__scepterLiveCloud,
            selectionReply: window.__gridLiveReplies?.findLast(
              reply => reply.cmd === 'octGridCellSelect'),
            commandLog: document.querySelector('#cmdState')?.value,
          };
        })()""")

    def observe_viewer(self):
        # Instrument the existing class before Start; this captures the page's
        # real viewer without adding debugging globals to production code.
        self.evaluate("""(async () => {
          const { Viewer3D } = await import('./js/viewer3D.js');
          const update = Viewer3D.prototype.update;
          window.__gridLiveFrames = {received: 0, occupied: 0, emptyAfterOccupied: 0};
          Viewer3D.prototype.update = function(frame) {
            window.__gridLiveViewer = this;
            const result = update.call(this, frame);
            if (frame.type === 'cells') {
              const counts = window.__gridLiveFrames;
              ++counts.received;
              if (frame.objects.some(object => object.count > 0)) ++counts.occupied;
              else if (counts.occupied) ++counts.emptyAfterOccupied;
            }
            return result;
          };
          window.__gridLiveReplies = [];
          const handle = window.handleCmd;
          window.handleCmd = function(reply) {
            window.__gridLiveReplies.push(reply);
            return handle(reply);
          };
        })()""")

    def cell_target(self):
        return self.evaluate("""(() => {
          const viewer = window.__gridLiveViewer;
          viewer.camera.updateMatrixWorld();
          const rect = viewer.renderer.domElement.getBoundingClientRect();
          let best = null;
          for (const object of viewer.objects.values()) {
            const boxes = object.boxes;
            if (!object.visible || !boxes?.geometry.instanceCount) continue;
            const centers = boxes.geometry.getAttribute('cellCenter').array;
            for (let i = 0; i < boxes.geometry.instanceCount; ++i) {
              const position = viewer.camera.position.clone().fromArray(centers, i * 3)
                .project(viewer.camera);
              if (Math.abs(position.x) > .85 || Math.abs(position.y) > .85 ||
                  position.z < -1 || position.z > 1) continue;
              const depth = boxes.grid.cells[boxes.cellIndices[i] * 20] & 63;
              const distance = position.x ** 2 + position.y ** 2;
              if (!best || depth > best.depth || (depth === best.depth && distance < best.distance))
                best = {depth, distance, x: rect.left + (position.x + 1) * rect.width / 2,
                  y: rect.top + (1 - position.y) * rect.height / 2};
            }
          }
          return best;
        })()""")

    def pick_cell(self, timeout=15):
        # Occupancy may expire between camera frames, particularly on Wi-Fi.
        # Project a fresh cell for each attempt; never retain a stale target
        # across the camera-fit settling delay or a missing grid snapshot.
        started = time.monotonic()
        state = {}
        while time.monotonic() - started < timeout:
            state = self.state()
            assert not self.errors, self.errors
            if state["picked"] == 1:
                print(f"Browser mouse cell selection: {time.monotonic() - started:.3f}s", flush=True)
                return state
            assert state["picked"] == 0, state
            point = self.cell_target()
            if point:
                for event in ("mouseMoved", "mousePressed", "mouseReleased"):
                    params = dict(type=event, x=point["x"], y=point["y"])
                    if event != "mouseMoved":
                        params.update(button="left", clickCount=1)
                    self.call("Input.dispatchMouseEvent", **params)
            time.sleep(0.05)
        raise TimeoutError(f"Browser mouse cell selection: {json.dumps(state)}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("--ip", default="192.168.16.101")
    parser.add_argument("--screenshot", type=Path, default=Path("/tmp/openkai-grid-live.png"))
    parser.add_argument("--log", type=Path, default=Path("/tmp/openkai-grid-live.log"))
    args = parser.parse_args()
    source = json.loads((ROOT / "samples/AProver_LiDAR/AProver_LiDAR.json").read_text())
    config = {key: source[key] for key in (
        "APP", "scepter", "scepterTransform", "scPC", "camRGB", "camDepth",
        "octGrid", "viewer", "wsConsole", "wsServer", "wsServer_rx", "wsServer_tx")}
    config["scepter"]["devURI"] = args.ip
    config["octGrid"]["vSelectedCells"] = []
    port, command_port = LIVE.HELPERS.free_port(), LIVE.HELPERS.free_port()
    while command_port == port:
        command_port = LIVE.HELPERS.free_port()
    config["viewer"].update(host="127.0.0.1", port=port)
    config["wsServer"].update(host="127.0.0.1", port=command_port)
    config["wsConsole"]["vBASE"] = ["scepter", "scepterTransform", "octGrid", "viewer"]
    with tempfile.TemporaryDirectory(prefix="openkai-grid-live-") as temp:
        launch = Path(temp) / "SelectableGrid.json"
        launch.write_text(json.dumps(config, indent=2))
        browser = None
        with args.log.open("w") as log:
            process = subprocess.Popen([str(args.binary.resolve()), str(launch)],
                                       cwd=ROOT, stdout=log, stderr=log)
            try:
                LIVE.HELPERS.wait_http(f"http://127.0.0.1:{port}/", process)
                browser = GridBrowser(temp)
                browser.call("Page.navigate", url=f"http://127.0.0.1:{port}/?cmdPort={command_port}")
                browser.wait("grid page ready", lambda state: state.get("ready"))
                browser.observe_viewer()
                browser.evaluate("document.querySelector('#start').click()")
                state = browser.wait("occupied grid and point cloud", lambda state:
                                     state.get("stream") == "Connected" and state.get("connected") and
                                     state.get("occupied", 0) > 0 and
                                     state.get("gridFrames", {}).get("occupied", 0) >= 3 and
                                     state.get("cloud", {}).get("frames", 0) >= 3, timeout=30)
                print("Initial occupancy:", json.dumps({key: state[key] for key in
                      ("occupied", "gridFrames", "cloud")}), flush=True)
                # Allow automatic fitting/orbit damping to settle before mouse input.
                time.sleep(0.5)
                browser.wait("fresh occupied grid after camera fit", lambda state:
                             state.get("occupied", 0) > 0)
                state = browser.pick_cell()
                selected = state["selections"]
                assert len(selected) == 1 and selected[0]["module"] == "octGrid", selected
                browser.evaluate("document.querySelector('#picker-send').click()")
                state = browser.wait("Send acknowledgement", lambda state:
                                     state.get("selectionReply") is not None)
                assert state["selectionReply"].get("bSuccess") is True, state
                saved = json.loads(launch.read_text())
                assert saved["octGrid"]["vSelectedCells"] == selected[0]["cellIDs"], saved["octGrid"]
                browser.evaluate("document.querySelector('#picker-clear').click()")
                browser.wait("clear browser selection", lambda state: state["picked"] == 0)
                browser.evaluate("document.querySelector('#picker-load').click()")
                state = browser.wait("Load saved selection", lambda state:
                                     state["picked"] == 1 and state["pickerStatus"].startswith("Added 1 cell"))
                assert state["selections"] == selected, state
                baseline = state["cloud"]["frames"]
                state = browser.wait("streaming after selection", lambda state:
                                     state.get("stream") == "Connected" and state.get("occupied", 0) > 0 and
                                     state.get("cloud", {}).get("frames", 0) >= baseline + 3)
                screenshot = browser.call("Page.captureScreenshot", format="png")
                args.screenshot.write_bytes(base64.b64decode(screenshot["data"]))
                print("Browser result:", json.dumps({key: state[key] for key in
                      ("occupied", "picked", "stats", "gridFrames", "cloud", "selectionReply")}), flush=True)
            finally:
                try:
                    if browser:
                        browser.close()
                finally:
                    LIVE.stop_process(process)
    print(f"Selectable-grid browser regression passed. Log: {args.log}; screenshot: {args.screenshot}")


if __name__ == "__main__":
    main()
