#!/usr/bin/env python3
"""Manually run the RealSense USB integration test; never included in CTest.

Requires a connected D455 and an OpenKAI build with RealSense/WebSocket support.
Uses only Python's standard library. The source launch JSON is never modified.
Example:
    python3 test/RealSense/hardware_smoke.py --executable build/OpenKAI

Exercises camera controls, stream combinations, geometry transport, rollback,
save/reload and graceful shutdown. Changed controls are restored from the first
live schema in a finally block, including dependent automatic-exposure controls.
The launch profile's initial hardware settings are the restoration baseline.
"""

import argparse
import copy
import importlib.util
import json
import math
from pathlib import Path
import signal
import socket
import struct
import subprocess
import tempfile
import time
import urllib.request

MAX_MESSAGE_BYTES = 64 * 1024 * 1024


def require(condition, message):
    if not condition:
        raise AssertionError(message)


def load_websocket_class(root):
    source = root / "html/console/editor/tests/browser-smoke.py"
    spec = importlib.util.spec_from_file_location("openkai_stdlib_websocket", source)
    module = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(module)

    class WebSocket(module.DevTools):
        """Reuse masking/handshake code and assemble complete binary/text messages."""

        def receive_message(self, timeout=30):
            deadline = time.monotonic() + timeout
            message = bytearray()
            message_opcode = None
            while True:
                remaining = deadline - time.monotonic()
                if remaining <= 0:
                    raise TimeoutError("Timed out assembling a WebSocket message")
                self.socket.settimeout(remaining)
                first, second = self._read(2)
                require(not first & 0x70, "Unexpected WebSocket extension bits")
                opcode, final = first & 15, bool(first & 128)
                size = second & 127
                if size == 126:
                    size = struct.unpack("!H", self._read(2))[0]
                elif size == 127:
                    size = struct.unpack("!Q", self._read(8))[0]
                require(size <= MAX_MESSAGE_BYTES, "Oversized WebSocket fragment")
                mask = self._read(4) if second & 128 else None
                data = self._read(size)
                if mask:
                    data = bytes(value ^ mask[index % 4] for index, value in enumerate(data))
                if opcode >= 8:
                    require(final and size <= 125, "Malformed WebSocket control frame")
                    if opcode == 8:
                        raise EOFError("Server closed the WebSocket")
                    if opcode == 9:
                        self._send(data, 10)
                    else:
                        require(opcode == 10, "Unknown WebSocket control opcode")
                    continue
                if opcode in (1, 2):
                    require(message_opcode is None, "New message before continuation completed")
                    message_opcode = opcode
                else:
                    require(opcode == 0 and message_opcode is not None, "Unexpected continuation")
                message.extend(data)
                require(len(message) <= MAX_MESSAGE_BYTES, "Oversized complete WebSocket message")
                if final:
                    return message_opcode, bytes(message)

        def command(self, module_name, command, config=None):
            self.sequence += 1
            request = {"module": module_name, "cmd": command, "requestId": self.sequence}
            if config is not None:
                request["config"] = config
            self._send((json.dumps(request) + "EOJ").encode())
            deadline, pending = time.monotonic() + 45, ""
            decoder = json.JSONDecoder()
            while time.monotonic() < deadline:
                opcode, data = self.receive_message(deadline - time.monotonic())
                require(opcode == 1, "Console returned a binary message")
                pending += data.decode("utf-8")
                while pending:
                    pending = pending.lstrip()
                    if pending.startswith("EOJ"):
                        pending = pending[3:]
                        continue
                    try:
                        reply, consumed = decoder.raw_decode(pending)
                    except json.JSONDecodeError:
                        break
                    pending = pending[consumed:]
                    if reply.get("requestId") == self.sequence:
                        return reply
            raise TimeoutError(f"No response for {command}")

        def close(self):
            try:
                self._send(struct.pack("!H", 1000), 8)
            except OSError:
                pass
            super().close()

    return WebSocket


def available_port():
    with socket.socket() as reservation:
        reservation.bind(("127.0.0.1", 0))
        return reservation.getsockname()[1]


def verify_geometry(data):
    """Validate the complete current v6 point-cloud snapshot, not one fragment."""
    require(32 <= len(data) <= MAX_MESSAGE_BYTES, "Invalid geometry length")
    magic, version, kind, sequence, count, length, stamp = struct.unpack_from("<6IQ", data)
    require((magic, version, kind) == (0x36443357, 6, 1), "Wrong geometry protocol/type")
    require(count <= 1024 and length == len(data), "Geometry header length mismatch")
    offset, ids, point_count = 32, set(), 0
    for _ in range(count):
        require(offset + 40 <= length, "Truncated geometry object")
        object_id, vertices, size, opacity, *bounds = struct.unpack_from("<II8f", data, offset)
        require(object_id not in ids, "Duplicate geometry object ID")
        ids.add(object_id)
        require(math.isfinite(size) and size > 0 and opacity == 1, "Invalid geometry material")
        require(all(math.isfinite(v) for v in bounds), "Nonfinite geometry bounds")
        require(all(bounds[i] <= bounds[i + 3] for i in range(3)), "Reversed geometry bounds")
        offset += 40
        payload = (vertices * 15 + 3) // 4 * 4
        require(offset + payload <= length, "Truncated geometry positions/colors")
        for xyz in struct.iter_unpack("<fff", memoryview(data)[offset:offset + vertices * 12]):
            require(all(math.isfinite(v) for v in xyz), "Nonfinite point position")
        padding = data[offset + vertices * 15:offset + payload]
        require(not any(padding), "Nonzero geometry padding")
        offset += payload
        point_count += vertices
    require(offset == length, "Unexpected geometry trailing data")
    require(point_count > 100, "No useful point cloud received; aim the camera at a textured scene")
    print(f"Geometry: {point_count:,} points, {length:,} bytes, sequence {sequence}, stamp {stamp}", flush=True)
    return point_count


class Application:
    def __init__(self, args, websocket, launch_path, log_path):
        self.args, self.websocket = args, websocket
        self.launch_path, self.log_path = launch_path, log_path
        self.process = self.log = self.console = None

    def start(self):
        self.log = self.log_path.open("a")
        self.process = subprocess.Popen([str(self.args.executable), str(self.launch_path)],
                                        cwd=self.args.root, stdout=self.log, stderr=self.log)
        deadline = time.monotonic() + self.args.startup_timeout
        while time.monotonic() < deadline:
            require(self.process.poll() is None, "OpenKAI exited during startup")
            try:
                if self.console is None:
                    self.console = self.websocket(f"ws://127.0.0.1:{self.args.console_port}/")
                reply = self.command("getConfig")
                if reply.get("bSuccess") and reply.get("deviceOpen"):
                    time.sleep(0.5)
                    return self.command("getConfig")
            except (OSError, EOFError):
                if self.console:
                    self.console.close()
                self.console = None
            time.sleep(0.2)
        raise TimeoutError("RealSense did not open; inspect " + str(self.log_path))

    def command(self, command, config=None):
        return self.console.command(self.args.module, command, config)

    def apply(self, patch):
        reply = self.command("setConfig", patch)
        require(reply.get("bSuccess") and reply.get("deviceOpen"), f"Cannot apply {patch}: {reply}")
        return reply

    def stop(self):
        if self.console:
            self.console.close()
            self.console = None
        try:
            if self.process:
                if self.process.poll() is None:
                    self.process.send_signal(signal.SIGINT)
                try:
                    code = self.process.wait(timeout=self.args.shutdown_timeout)
                except subprocess.TimeoutExpired:
                    self.process.kill()
                    self.process.wait()
                    raise RuntimeError("OpenKAI did not shut down cleanly after SIGINT")
                require(code == 0, f"OpenKAI exited with code {code}")
                print("Clean SIGINT shutdown", flush=True)
        finally:
            if self.log:
                self.log.close()
                self.log = None

    def rates(self, label):
        time.sleep(0.5)  # Exclude frames queued around pipeline restart.
        before = self.command("getConfig")
        started = time.monotonic()
        time.sleep(self.args.sample_seconds)
        after = self.command("getConfig")
        elapsed = time.monotonic() - started
        require(after.get("deviceOpen"), f"Camera closed during {label}: {after}")
        delta = {key: after["status"][key] - before["status"][key]
                 for key in ("videoFrames", "accelSamples", "gyroSamples")}
        config = after["config"]
        video_enabled = config["bRGB"] or config["bDepth"] or config["bIR"]
        enabled = {"videoFrames": video_enabled, "accelSamples": config["bIMU"], "gyroSamples": config["bIMU"]}
        expected = {"videoFrames": max(config["devFPS"] if config["bRGB"] else 0,
                                        config["devFPSd"] if config["bDepth"] or config["bIR"] else 0),
                    "accelSamples": config["accelFPS"] or 63,
                    "gyroSamples": config["gyroFPS"] or 200}
        for key, active in enabled.items():
            if active:
                require(delta[key] / elapsed >= expected[key] * 0.5,
                        f"{label}: {key} too slow: {delta[key] / elapsed:.1f}/s")
            else:
                require(delta[key] == 0, f"{label}: disabled stream {key} still produced samples")
        print(label + ": " + ", ".join(f"{key}={value / elapsed:.1f}/s" for key, value in delta.items()), flush=True)
        return after

    def preview(self, enabled=True):
        reply = self.command("getIMU")
        require(reply.get("bSuccess") and reply.get("deviceOpen"), f"Cannot read IMU preview: {reply}")
        require(reply.get("enabled") == enabled, "Preview enabled state differs from capture")
        require("config" not in reply and "schema" not in reply, "IMU polling must not resend camera metadata")
        for key in ("tGyro", "tAcc", "tFusion"):
            require(isinstance(reply.get(key), str) and reply[key].isdigit(), "Capture timestamps must preserve nanoseconds as strings")
        if not enabled:
            require(not reply["available"] and not reply["orientationValid"], "Disabled IMU exposed stale data")
            require(reply["tGyro"] == reply["tAcc"] == reply["tFusion"] == "0", "Restart retained old IMU samples")
            return reply
        require(reply.get("available") and reply.get("orientationValid"), f"Missing live IMU/orientation: {reply}")
        for key, size in (("gyro", 3), ("acc", 3), ("quaternion", 4), ("rpy", 3)):
            require(len(reply.get(key, [])) == size and all(math.isfinite(v) for v in reply[key]), "Invalid preview " + key)
        require(all(int(reply[key]) > 0 for key in ("tGyro", "tAcc", "tFusion")), "Preview has no capture timestamps")
        require(math.isclose(sum(v * v for v in reply["quaternion"]), 1, abs_tol=1e-5), "Preview orientation is not normalized")
        return reply

    def geometry(self):
        stream = self.websocket(f"ws://127.0.0.1:{self.args.viewer_port}/stream/points")
        try:
            opcode, hello = stream.receive_message()
            require(opcode == 1 and json.loads(hello).get("version") == 6, "Missing geometry v6 hello")
            stream._send(b"start")
            deadline = time.monotonic() + 15
            while time.monotonic() < deadline:
                opcode, data = stream.receive_message(deadline - time.monotonic())
                if opcode == 2:
                    return verify_geometry(data)
            raise TimeoutError("No binary geometry snapshot")
        finally:
            stream.close()


def schema_fields(reply):
    return {(field.get("domain"), field["key"]): field for field in reply["schema"] if field.get("domain")}


def current_value(reply, domain, key):
    field = schema_fields(reply).get((domain, key), {})
    require(field.get("supported") and field.get("current") is not None, f"Missing device value: {domain}.{key}")
    return field["current"]


def different_value(field, preferred):
    minimum, maximum, step = field["min"], field["max"], field.get("step") or 1
    value = min(max(preferred, minimum), maximum)
    value = minimum + round((value - minimum) / step) * step
    if math.isclose(value, field["current"], rel_tol=1e-6, abs_tol=1e-6):
        value = value + step if value + step <= maximum else value - step
    require(minimum <= value <= maximum, "No alternative value in device range")
    return int(value) if field["type"] == "int" else value


def run(args):
    args.root = args.root.resolve()
    args.executable = (args.root / args.executable).resolve()
    args.config = (args.root / args.config).resolve()
    require(args.executable.is_file(), f"Executable not found: {args.executable}")
    launch = json.loads(args.config.read_text())
    require(launch[args.module]["class"] == "_RealSense", "Selected module is not RealSense")
    source_module = copy.deepcopy(launch[args.module])
    # Isolate listeners and output files from the user's usual launch config.
    args.console_port = args.console_port or available_port()
    args.viewer_port = args.viewer_port or available_port()
    while args.console_port == args.viewer_port:
        args.viewer_port = available_port()
    launch["console"]["bON"] = False
    launch["wsServer"].update({"host": "127.0.0.1", "port": args.console_port})
    launch["viewer"].update({"host": "127.0.0.1", "port": args.viewer_port})
    artifacts = args.artifacts.resolve() if args.artifacts else Path(tempfile.mkdtemp(prefix="openkai-realsense-hardware-"))
    artifacts.mkdir(parents=True, exist_ok=True)
    launch_path, log_path = artifacts / "launch.json", artifacts / "OpenKAI.log"
    launch_path.write_text(json.dumps(launch, indent=2) + "\n")
    print(f"Artifacts: {artifacts}", flush=True)
    app = Application(args, load_websocket_class(args.root), launch_path, log_path)
    baseline = None
    failure = None
    cleanup_errors = []
    try:
        reply = app.start()
        (artifacts / "initial-response.json").write_text(json.dumps(reply, indent=2) + "\n")
        baseline = copy.deepcopy(reply["config"])
        initial_fields = schema_fields(reply)
        # Save actual values of all controls this test or their SDK side effects
        # can touch; null configured values alone cannot restore hardware state.
        restore_keys = {
            "color": ("RS2_OPTION_EXPOSURE", "RS2_OPTION_ENABLE_AUTO_EXPOSURE", "RS2_OPTION_GAIN", "RS2_OPTION_BRIGHTNESS"),
            "depth": ("RS2_OPTION_AUTO_EXPOSURE_LIMIT", "RS2_OPTION_AUTO_EXPOSURE_LIMIT_TOGGLE"),
            "decimation": ("RS2_OPTION_FILTER_MAGNITUDE",),
        }
        for domain, keys in restore_keys.items():
            for key in keys:
                field = initial_fields.get((domain, key))
                if field and field.get("supported") and not field.get("readOnly") and field.get("current") is not None:
                    baseline["sensorOptions"].setdefault(domain, {})[key] = field["current"]
        app.apply({"bRGB": True, "bDepth": True, "bIMU": True, "bIR": False,
                   "bPCL": True, "bPCLrgb": True, "bAlign": False, "bDecimation": False})
        app.rates("RGB + depth + IMU")
        app.preview()
        with urllib.request.urlopen(f"http://127.0.0.1:{args.viewer_port}/", timeout=10) as response:
            require("RealSense" in response.read().decode(), "Wrong viewer served")
        app.geometry()

        exposure_key = "RS2_OPTION_EXPOSURE"
        auto_key = "RS2_OPTION_ENABLE_AUTO_EXPOSURE"
        before = app.command("getConfig")
        brightness = current_value(before, "color", "RS2_OPTION_BRIGHTNESS")
        exposure = different_value(schema_fields(before)[("color", exposure_key)], 170)
        app.apply({"sensorOptions": {"color": {auto_key: 0, exposure_key: exposure}}})
        checked = app.command("getConfig")
        require(math.isclose(current_value(checked, "color", exposure_key), exposure), "Color exposure did not reach the sensor")
        require(current_value(checked, "color", "RS2_OPTION_BRIGHTNESS") == brightness,
                "Exposure write changed brightness")
        limit_key = "RS2_OPTION_AUTO_EXPOSURE_LIMIT"
        field = schema_fields(checked)[("depth", limit_key)]
        require(not field["readOnly"], "Pre-stream exposure limit incorrectly disabled in viewer")
        limit = different_value(field, 190000)
        app.apply({"sensorOptions": {"depth": {limit_key: limit}}})
        require(math.isclose(current_value(app.command("getConfig"), "depth", limit_key), limit),
                "Pre-stream exposure limit did not reach the sensor")
        print("Exposure, brightness isolation and pre-stream control readback passed", flush=True)

        for label, patch in (
            ("Depth only", {"bRGB": False, "bDepth": True, "bIR": False, "bIMU": False, "bPCL": True, "bPCLrgb": False}),
            ("RGB only", {"bRGB": True, "bDepth": False, "bIR": False, "bIMU": False, "bPCL": False, "bPCLrgb": False}),
            ("IR only", {"bRGB": False, "bDepth": False, "bIR": True, "bIMU": False, "bPCL": False, "bPCLrgb": False}),
            ("IMU only", {"bRGB": False, "bDepth": False, "bIR": False, "bPCL": False, "bIMU": True}),
            ("RGB + depth without IMU", {"bRGB": True, "bDepth": True, "bPCL": True, "bPCLrgb": True, "bIMU": False}),
            ("Alignment + decimation + IMU", {"bIMU": True, "bAlign": True, "bDecimation": True,
                                             "sensorOptions": {"decimation": {"RS2_OPTION_FILTER_MAGNITUDE": 2}}}),
        ):
            app.apply(patch)
            state = app.rates(label)
            app.preview(state["config"]["bIMU"])
        app.geometry()

        before = app.command("getConfig")
        laser = schema_fields(before)[("depth", "RS2_OPTION_LASER_POWER")]
        invalid = app.command("setConfig", {"sensorOptions": {"depth": {"RS2_OPTION_LASER_POWER": laser["max"] + 1000}}})
        require(not invalid["bSuccess"] and invalid["deviceOpen"], "Invalid option range was not rejected")
        require(invalid["config"] == before["config"], "Invalid range changed configuration")
        invalid = app.command("setConfig", {"vSizeD": [123, 234]})
        require(not invalid["bSuccess"] and invalid["deviceOpen"], "Invalid profile did not roll back to an open camera")
        require(invalid["config"] == before["config"], "Invalid profile changed configuration")
        app.rates("Capture after failed-profile rollback")

        saved_reply = app.command("saveConfig")
        require(saved_reply["bSuccess"], f"Save failed: {saved_reply}")
        saved_launch = json.loads(launch_path.read_text())
        saved = saved_launch[args.module]
        expected_keys = set(source_module) | set(saved_reply["config"]) | {"name", "class", "bLog", "thread", "threadPP"}
        require(set(saved) == expected_keys, "Save introduced fields outside the current configuration schema")
        for key, value in saved_reply["config"].items():
            require(saved.get(key) == value, f"Save lost camera setting {key}")
        for key, value in source_module.items():
            if key.endswith(("frameOut", "streamOut")) or key in ("class", "bON"):
                require(saved.get(key) == value, f"Save changed base field/link {key}")
            elif key in ("thread", "threadPP"):
                for field, original in value.items():
                    require(saved[key].get(field) == original, f"Save changed {key}.{field}")
        app.stop()
        reloaded = app.start()
        require(reloaded["config"] == saved_reply["config"], "Saved configuration did not round trip through loadConfig")
        state = app.rates("Capture after saved-config reload")
        app.preview(state["config"]["bIMU"])
        app.geometry()
        print("Invalid-input rollback and save/reload passed", flush=True)
    except BaseException as error:
        failure = error
    finally:
        if baseline is not None and app.process and app.process.poll() is None:
            try:
                if app.console is None:
                    app.console = app.websocket(f"ws://127.0.0.1:{args.console_port}/")
                restored = app.apply(baseline)
                require(restored["deviceOpen"], "Restore did not reopen the camera")
                print("Restored original stream configuration and actual changed controls", flush=True)
            except Exception as error:
                cleanup_errors.append(f"Best-effort hardware restoration failed: {error}")
        try:
            app.stop()
        except Exception as error:
            cleanup_errors.append(str(error))
    if failure or cleanup_errors:
        details = "\n".join(([str(failure)] if failure else []) + cleanup_errors)
        tail = log_path.read_text(errors="replace")[-5000:] if log_path.exists() else ""
        raise RuntimeError(f"{details}\nArtifacts: {artifacts}\nLast backend output:\n{tail}") from failure
    print("RealSense hardware smoke test passed", flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--executable", type=Path, default=Path("build/OpenKAI"))
    parser.add_argument("--config", type=Path, default=Path("jsonCfg/RealSense.json"))
    parser.add_argument("--module", default="RealSense")
    parser.add_argument("--artifacts", type=Path, help="Keep isolated launch JSON and logs here (default: new temporary directory)")
    parser.add_argument("--console-port", type=int, default=0)
    parser.add_argument("--viewer-port", type=int, default=0)
    parser.add_argument("--startup-timeout", type=float, default=30)
    parser.add_argument("--shutdown-timeout", type=float, default=20)
    parser.add_argument("--sample-seconds", type=float, default=2)
    args = parser.parse_args()
    if min(args.startup_timeout, args.shutdown_timeout, args.sample_seconds) <= 0:
        parser.error("Timeouts and sample duration must be positive")
    run(args)


if __name__ == "__main__":
    main()
