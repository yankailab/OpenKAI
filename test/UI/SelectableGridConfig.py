#!/usr/bin/env python3
"""Exercise all selectable-grid panels' Load/Apply/Save separation without hardware.

  python3 test/UI/SelectableGridConfig.py build/OpenKAI

Launches only a grid, transform, viewer and transport on temporary localhost ports.
All writes go to a temporary launch JSON. Requires Chrome/Chromium; no Python
dependencies. Includes a restart to verify the saved configuration loads cleanly.
"""

import argparse
import base64
import importlib.util
import json
import math
from pathlib import Path
import subprocess
import tempfile


ROOT = Path(__file__).resolve().parents[2]
SPEC = importlib.util.spec_from_file_location(
    "scepter_live", ROOT / "test/Sensor/ScepterLive.py")
LIVE = importlib.util.module_from_spec(SPEC)
SPEC.loader.exec_module(LIVE)
SCALARS = ("dTexpireCell", "dTexpirePCL", "nMaxCells", "nMaxLevel", "nPminBuild")
INPUT_IDS = {
    "vPorigin": [f"grid-origin-{axis}" for axis in "xyz"],
    "vRootCellSize": [f"grid-size-{axis}" for axis in "xyz"],
    **{key: f"grid-{key}" for key in SCALARS},
}


def input_values(config):
    return {key: [str(value) for value in config[key]] if isinstance(ids, list)
            else str(config[key]) for key, ids in INPUT_IDS.items()}


def inputs_match(actual, expected):
    return all(actual.get(key) == str(expected[key]) for key in SCALARS) and all(
        isinstance(actual.get(key), list) and len(actual[key]) == 3 and
        [float(value) for value in actual[key]] == expected[key]
        for key in ("vPorigin", "vRootCellSize"))


def transform_matches(actual, expected):
    return all(len(actual.get(key, [])) == 3 and all(
        math.isclose(float(value), target, abs_tol=1e-8)
        for value, target in zip(actual[key], expected[key])) for key in ("vT", "vAngle"))


class GridConfigBrowser(LIVE.Browser):
    def state(self):
        return self.evaluate("""(() => {
          const ids = %s;
          const inputs = Object.fromEntries(Object.entries(ids).map(([key, value]) =>
            [key, Array.isArray(value) ? value.map(id => document.getElementById(id)?.value) :
              document.getElementById(value)?.value]));
          return {
            ready: !!document.querySelector('#viewport canvas') &&
              document.querySelector('#start')?.disabled === false,
            connected: window.wsSocket?.readyState === WebSocket.OPEN,
            enabled: document.querySelector('#grid-config-fields')?.disabled === false &&
              document.querySelector('#grid-config-load')?.disabled === false &&
              document.querySelector('#grid-config-apply')?.disabled === false &&
              document.querySelector('#grid-config-save')?.disabled === false,
            transformEnabled: document.querySelector('#transform-fields')?.disabled === false &&
              document.querySelector('#transform-save')?.disabled === false,
            transform: {
              vT: ['x', 'y', 'z'].map(axis => document.getElementById('transform-' + axis)?.value),
              vAngle: ['roll', 'pitch', 'yaw'].map(axis => document.getElementById('transform-' + axis)?.value),
            },
            pickerEnabled: ['load', 'apply', 'save'].every(action =>
              document.getElementById('picker-' + action)?.disabled === false),
            picked: window.__gridConfigViewer?.picker.count || 0,
            pickerStatus: document.querySelector('#picker-status')?.textContent,
            status: document.querySelector('#grid-config-status')?.textContent,
            stream: document.querySelector('#status')?.textContent,
            inputs,
            sliders: ['grid-min-level', 'grid-max-level'].map(id => document.getElementById(id)?.value),
            header: window.__gridConfigViewer?.picker.sources.get('octGrid')?.header,
            frames: window.__gridConfigFrames || 0,
            replies: window.__gridConfigReplies || [],
            sent: window.__gridConfigSent || [],
            commandLog: document.querySelector('#cmdState')?.value,
          };
        })()""" % json.dumps(INPUT_IDS))

    def observe(self):
        self.evaluate("""(async () => {
          const { Viewer3D } = await import('./js/viewer3D.js');
          const update = Viewer3D.prototype.update;
          window.__gridConfigFrames = 0;
          Viewer3D.prototype.update = function(frame) {
            window.__gridConfigViewer = this;
            if (frame.type === 'cells') ++window.__gridConfigFrames;
            return update.call(this, frame);
          };
          window.__gridConfigReplies = [];
          const handle = window.handleCmd;
          window.handleCmd = function(reply) {
            window.__gridConfigReplies.push(reply);
            return handle(reply);
          };
          window.__gridConfigSent = [];
          const send = window.wsSendCmd;
          window.wsSendCmd = function(command) {
            window.__gridConfigSent.push(command);
            return send(command);
          };
        })()""")

    def fill(self, values):
        self.evaluate("""(() => {
          const ids = %s, values = %s;
          for (const [key, value] of Object.entries(values)) {
            const names = Array.isArray(ids[key]) ? ids[key] : [ids[key]];
            const entries = Array.isArray(value) ? value : [value];
            names.forEach((id, i) => {
              const input = document.getElementById(id);
              if (!input || input.disabled || document.querySelector('#grid-config-fields').disabled)
                throw Error('Grid config input unavailable: ' + id);
              input.value = String(entries[i]);
              input.dispatchEvent(new Event('input', {bubbles: true}));
            });
          }
        })()""" % (json.dumps(INPUT_IDS), json.dumps(values)))

    def click(self, element):
        self.evaluate(f"document.getElementById({json.dumps(element)}).click()")

    def action(self, element, cmd, module="octGrid"):
        baseline = len(self.state()["sent"])
        self.click(element)
        commands = [command for command in self.state()["sent"][baseline:]
                    if command.get("cmd") == cmd and command.get("module") == module]
        assert len(commands) == 1, (element, commands, self.state())
        request = commands[0]
        state = self.wait(element, lambda state: any(
            reply.get("requestId") == request["requestId"] for reply in state["replies"]))
        reply = next(reply for reply in state["replies"] if reply.get("requestId") == request["requestId"])
        assert reply.get("bSuccess") is True, reply
        return state

    def fill_transform(self, values):
        self.evaluate("""(() => {
          const values = %s, ids = {vT: ['x', 'y', 'z'], vAngle: ['roll', 'pitch', 'yaw']};
          if (document.querySelector('#transform-fields').disabled) throw Error('Transform unavailable');
          for (const [key, entries] of Object.entries(values)) entries.forEach((value, i) => {
            const input = document.getElementById('transform-' + ids[key][i]);
            input.value = String(value);
            input.dispatchEvent(new Event('input', {bubbles: true}));
          });
        })()""" % json.dumps(values))

    def load(self, expected):
        state = self.action("grid-config-load", "getGridConfig")
        assert state["enabled"] and inputs_match(state["inputs"], expected), state
        return state


def verify_saved(launch, expected):
    saved = json.loads(launch.read_text())["octGrid"]
    for key in INPUT_IDS:
        assert saved[key] == expected[key], (key, saved[key], expected[key])
    return saved


def run_browser(binary, launch, port, command_port, temp, log):
    process = subprocess.Popen([str(binary), str(launch)], cwd=ROOT, stdout=log, stderr=log)
    browser = None
    try:
        LIVE.HELPERS.wait_http(f"http://127.0.0.1:{port}/", process)
        browser = GridConfigBrowser(temp)
        browser.call("Page.navigate", url=f"http://127.0.0.1:{port}/?cmdPort={command_port}")
        browser.wait("grid page ready", lambda state: state.get("ready"))
        browser.observe()
        browser.click("start")
        browser.wait("grid connected and config loaded", lambda state:
                     state["connected"] and state["enabled"] and state["transformEnabled"] and
                     state["pickerEnabled"] and state.get("header") and
                     state["stream"] == "Connected")
        return process, browser
    except BaseException:
        if browser:
            browser.close()
        LIVE.stop_process(process)
        raise


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("--screenshot", type=Path, default=Path("/tmp/openkai-grid-config.png"))
    parser.add_argument("--log", type=Path, default=Path("/tmp/openkai-grid-config.log"))
    args = parser.parse_args()
    source = json.loads((ROOT / "samples/AProver_LiDAR/AProver_LiDAR.json").read_text())
    config = {key: source[key] for key in
              ("APP", "octGrid", "scepterTransform", "viewer", "jsonCmd", "wsServer", "wsServer_rx", "wsServer_tx")}
    initial = dict(vPorigin=[1, 2, 3], vRootCellSize=[10, 10, 12],
                   dTexpireCell=1500000000, dTexpirePCL=500000000,
                   nMaxCells=1000, nMaxLevel=5, nPminBuild=10)
    root_selection = ["0" * 32]
    config["octGrid"].update(initial, vPCLframesIn=[], vSelectedCells=root_selection)
    config["octGrid"].pop("_TransformRigid", None)
    config["scepterTransform"].update(vT=[0, 0, 0], vR=[0, 0, 0, 1])
    initial_transform = dict(vT=[0, 0, 0], vAngle=[0, 0, 0])
    applied_transform = dict(vT=[1.25, -2.5, 3.75], vAngle=[10, -20, 30])
    port, command_port = LIVE.HELPERS.free_port(), LIVE.HELPERS.free_port()
    while command_port == port:
        command_port = LIVE.HELPERS.free_port()
    config["viewer"].update(host="127.0.0.1", port=port, vGeometry=[],
                            webRoot=str(ROOT / "html/viewer/_SelectableOctGrid"))
    config["wsServer"].update(host="127.0.0.1", port=command_port)
    config["jsonCmd"]["vBASE"] = ["octGrid", "scepterTransform", "viewer"]
    updated = dict(vPorigin=[-1.25, 2.5, 4], vRootCellSize=[4, 6, 8],
                   dTexpireCell=18446744073709551615, dTexpirePCL=9007199254740993,
                   nMaxCells=321, nMaxLevel=3, nPminBuild=7)
    with tempfile.TemporaryDirectory(prefix="openkai-grid-config-") as temp:
        launch = Path(temp) / "SelectableGrid.json"
        launch.write_text(json.dumps(config, indent=2))
        process = browser = None
        with args.log.open("w") as log:
            try:
                process, browser = run_browser(args.binary.resolve(), launch, port, command_port, temp, log)
                assert inputs_match(browser.state()["inputs"], initial), browser.state()
                # Explicit Load replaces unsaved input, including both root vectors.
                browser.fill({key: "1" for key in SCALARS})
                browser.fill(dict(vPorigin=[0, 0, 0], vRootCellSize=[1, 1, 1]))
                browser.load(initial)
                browser.evaluate("""(() => {
                  for (const [id, value] of [['grid-min-level', '2'], ['grid-max-level', '4']]) {
                    const input = document.getElementById(id);
                    input.value = value;
                    input.dispatchEvent(new Event('input', {bubbles: true}));
                  }
                })()""")
                # Save does not apply a grid draft. Apply changes runtime only.
                browser.fill(input_values(updated))
                state = browser.action("grid-config-save", "saveConfig")
                verify_saved(launch, initial)
                assert inputs_match(state["inputs"], updated), state
                before_apply = launch.read_bytes()
                state = browser.action("grid-config-apply", "setGridConfig")
                assert launch.read_bytes() == before_apply, "Grid Apply wrote the launch JSON"
                command = next(command for command in reversed(state["sent"])
                               if command["cmd"] == "setGridConfig")
                assert {key: command[key] for key in INPUT_IDS} == input_values(updated), command
                assert state["sliders"] == ["2", "4"], state
                # Another module saves the shared bound JSON; unapplied grid JSON
                # must still hold the previously saved configuration.
                browser.action("transform-save", "saveConfig", "scepterTransform")
                verify_saved(launch, initial)
                state = browser.wait("new root/depth streamed", lambda state:
                                     state.get("header") == dict(origin=updated["vPorigin"],
                                     size=updated["vRootCellSize"], maxLevel=updated["nMaxLevel"]))
                # Save persists applied backend state without consuming a newer draft.
                frames = state["frames"]
                browser.fill({"nMaxCells": "999"})
                browser.wait("stream leaves draft config unchanged", lambda state:
                             state["frames"] >= frames + 3 and state["inputs"]["nMaxCells"] == "999")
                state = browser.action("grid-config-save", "saveConfig")
                assert state["inputs"]["nMaxCells"] == "999", state
                verify_saved(launch, updated)
                browser.load(updated)
                before_invalid = len(browser.state()["sent"])
                browser.fill({"dTexpireCell": "-1"})
                browser.click("grid-config-apply")
                assert browser.evaluate("document.querySelector('#grid-config').checkValidity()") is False
                assert len(browser.state()["sent"]) == before_invalid, browser.state()
                browser.load(updated)
                # Bypass form validation to verify backend validation is atomic too.
                invalid = dict(command, requestId="grid-config-invalid", nMaxLevel="-1")
                browser.evaluate(f"window.wsSendCmd({json.dumps(invalid)})")
                state = browser.wait("backend rejects invalid complete config", lambda state:
                                     any(reply.get("requestId") == invalid["requestId"]
                                         for reply in state["replies"]))
                reply = next(reply for reply in state["replies"]
                             if reply.get("requestId") == invalid["requestId"])
                assert reply.get("bSuccess") is False, reply
                verify_saved(launch, updated)
                browser.load(updated)
                assert browser.state()["sliders"] == ["2", "4"], browser.state()

                # A selected root works without point-cloud hardware. Clear stays
                # local; Apply may send an empty selection; only Save persists it.
                state = browser.action("picker-load", "getCellSelection")
                assert state["picked"] == 1, state
                browser.click("picker-clear")
                assert browser.state()["picked"] == 0, browser.state()
                state = browser.action("picker-save", "saveConfig")
                assert state["picked"] == 0, state
                assert verify_saved(launch, updated)["vSelectedCells"] == root_selection
                state = browser.action("picker-load", "getCellSelection")
                assert state["picked"] == 1, state
                browser.click("picker-clear")
                before_apply = launch.read_bytes()
                state = browser.action("picker-apply", "setCellSelection")
                assert state["picked"] == 0, state
                assert launch.read_bytes() == before_apply, "Picker Apply wrote the launch JSON"
                selection_command = next(command for command in reversed(state["sent"])
                                         if command["cmd"] == "setCellSelection")
                assert selection_command["cellIDs"] == [] and selection_command["nMaxLevel"] == 3
                browser.action("transform-save", "saveConfig", "scepterTransform")
                assert verify_saved(launch, updated)["vSelectedCells"] == root_selection, "Picker Apply mutated bound JSON"
                state = browser.action("picker-load", "getCellSelection")
                assert state["picked"] == 0, state
                browser.action("picker-save", "saveConfig")
                assert verify_saved(launch, updated)["vSelectedCells"] == []

                # Transform uses its existing commands, with independent Apply
                # and Save buttons. Saving another module must not save its draft.
                assert transform_matches(browser.state()["transform"], initial_transform)
                browser.fill_transform(applied_transform)
                state = browser.action("transform-save", "saveConfig", "scepterTransform")
                assert transform_matches(state["transform"], applied_transform), state
                saved_transform = json.loads(launch.read_text())["scepterTransform"]
                assert saved_transform["vT"] == [0, 0, 0] and saved_transform["vR"] == [0, 0, 0, 1]
                before_apply = launch.read_bytes()
                state = browser.action("transform-apply", "setConfig", "scepterTransform")
                assert transform_matches(state["transform"], applied_transform), state
                assert launch.read_bytes() == before_apply, "Transform Apply wrote the launch JSON"
                browser.action("grid-config-save", "saveConfig")
                saved_transform = json.loads(launch.read_text())["scepterTransform"]
                assert saved_transform["vT"] == [0, 0, 0] and saved_transform["vR"] == [0, 0, 0, 1], "Transform Apply mutated bound JSON"
                state = browser.action("transform-load", "getConfig", "scepterTransform")
                assert transform_matches(state["transform"], applied_transform), state
                transform_draft = dict(vT=[9, 8, 7], vAngle=[40, 50, 60])
                browser.fill_transform(transform_draft)
                state = browser.action("transform-save", "saveConfig", "scepterTransform")
                assert transform_matches(state["transform"], transform_draft), state
                saved_transform = json.loads(launch.read_text())["scepterTransform"]
                assert saved_transform["vT"] == applied_transform["vT"] and saved_transform["vR"] != [0, 0, 0, 1]
                state = browser.action("transform-load", "getConfig", "scepterTransform")
                assert transform_matches(state["transform"], applied_transform), state
                screenshot = browser.call("Page.captureScreenshot", format="png")
                args.screenshot.write_bytes(base64.b64decode(screenshot["data"]))
                browser.close()
                browser = None
                LIVE.stop_process(process)
                process = None
                # A fresh process must load the exact uint64 JSON values and new depth.
                restart_temp = Path(temp) / "restart"
                restart_temp.mkdir()
                process, browser = run_browser(args.binary.resolve(), launch, port, command_port,
                                               str(restart_temp), log)
                assert inputs_match(browser.state()["inputs"], updated), browser.state()
                browser.load(updated)
                assert verify_saved(launch, updated)["vSelectedCells"] == []
                assert transform_matches(browser.state()["transform"], applied_transform), browser.state()
                state = browser.action("picker-load", "getCellSelection")
                assert state["picked"] == 0, state
            finally:
                try:
                    if browser:
                        browser.close()
                finally:
                    if process:
                        LIVE.stop_process(process)
    print(f"Selectable-grid config browser regression passed. Log: {args.log}; screenshot: {args.screenshot}")


if __name__ == "__main__":
    main()
