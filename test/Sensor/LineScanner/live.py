#!/usr/bin/env python3
"""Exercise SporSensor against hardware, including its served browser viewer.

Run from the repository root:
  python3 test/Sensor/LineScanner/live.py build/OpenKAI
Uses temporary configuration/ports, starts acquisition, then stops it on exit.
Temporarily tests software triggering and restores the original trigger mode.
Laser power is reapplied at its current value; other device tuning is preserved.
Requires the connected sensor and Chrome/Chromium, but no Python dependencies.
"""

import argparse
import base64
import hashlib
import json
import math
import os
from pathlib import Path
import shutil
import signal
import socket
import struct
import subprocess
import tempfile
import time
import urllib.request


class WebSocket:
    def __init__(self, url, timeout=20):
        from urllib.parse import urlsplit
        endpoint = urlsplit(url)
        self.sock = socket.create_connection((endpoint.hostname, endpoint.port), timeout)
        self.sock.settimeout(timeout)
        key = base64.b64encode(os.urandom(16)).decode()
        request = (f"GET {endpoint.path or '/'} HTTP/1.1\r\n"
                   f"Host: {endpoint.netloc}\r\nUpgrade: websocket\r\n"
                   f"Connection: Upgrade\r\nSec-WebSocket-Key: {key}\r\n"
                   "Sec-WebSocket-Version: 13\r\n\r\n")
        self.sock.sendall(request.encode())
        self.buffer = b""
        while b"\r\n\r\n" not in self.buffer:
            self.buffer += self.sock.recv(4096)
        header, self.buffer = self.buffer.split(b"\r\n\r\n", 1)
        assert b" 101 " in header, header
        accept = base64.b64encode(hashlib.sha1(
            (key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11").encode()).digest())
        assert accept.lower() in header.lower(), header

    def read(self, size):
        while len(self.buffer) < size:
            chunk = self.sock.recv(max(4096, size - len(self.buffer)))
            if not chunk:
                raise EOFError("WebSocket closed")
            self.buffer += chunk
        result, self.buffer = self.buffer[:size], self.buffer[size:]
        return result

    def send(self, text, opcode=1):
        data = text.encode() if isinstance(text, str) else text
        mask = os.urandom(4)
        length = len(data)
        header = bytes([0x80 | opcode])
        if length < 126:
            header += bytes([0x80 | length])
        elif length < 65536:
            header += bytes([0xFE]) + struct.pack("!H", length)
        else:
            header += bytes([0xFF]) + struct.pack("!Q", length)
        self.sock.sendall(header + mask + bytes(b ^ mask[i % 4] for i, b in enumerate(data)))

    def receive(self):
        chunks = []
        kind = None
        while True:
            first, second = self.read(2)
            length = second & 127
            if length == 126:
                length, = struct.unpack("!H", self.read(2))
            elif length == 127:
                length, = struct.unpack("!Q", self.read(8))
            assert length < 256 * 1024 * 1024
            mask = self.read(4) if second & 128 else None
            data = self.read(length)
            if mask:
                data = bytes(b ^ mask[i % 4] for i, b in enumerate(data))
            opcode = first & 15
            if opcode == 8:
                raise EOFError("WebSocket close frame")
            if opcode == 9:
                self.send(data, 10)
                continue
            if opcode == 10:
                continue
            if opcode:
                kind = opcode
            chunks.append(data)
            if first & 128:
                result = b"".join(chunks)
                return result.decode() if kind == 1 else result

    def close(self):
        try:
            self.send(b"", 8)
        except OSError:
            pass
        self.sock.close()


class Commands:
    def __init__(self, port):
        self.ws = WebSocket(f"ws://127.0.0.1:{port}/", timeout=60)
        self.buffer = ""
        self.serial = 0

    def request(self, command, **fields):
        self.serial += 1
        request_id = f"live-{self.serial}"
        self.ws.send(json.dumps(dict(module="sporSensor", cmd=command,
                                     requestId=request_id, **fields)) + "EOJ")
        deadline = time.monotonic() + 90
        while time.monotonic() < deadline:
            self.buffer = self.buffer.lstrip()
            if self.buffer.startswith("EOJ"):
                self.buffer = self.buffer[3:]
                continue
            try:
                reply, end = json.JSONDecoder().raw_decode(self.buffer)
            except ValueError:
                self.buffer += self.ws.receive()
                continue
            self.buffer = self.buffer[end:]
            if reply.get("requestId") == request_id:
                return reply
        raise TimeoutError(command)


def free_port():
    with socket.socket() as sock:
        sock.bind(("127.0.0.1", 0))
        return sock.getsockname()[1]


def wait_http(url, process, timeout=30):
    deadline = time.monotonic() + timeout
    while time.monotonic() < deadline:
        if process.poll() is not None:
            raise RuntimeError(f"Process exited with {process.returncode}")
        try:
            with urllib.request.urlopen(url, timeout=1) as response:
                return response.read()
        except OSError:
            time.sleep(0.1)
    raise TimeoutError(url)


def inspect_cloud(frame):
    assert isinstance(frame, bytes) and len(frame) >= 32
    magic, version, kind, _, count, length = struct.unpack_from("<6I", frame)
    assert (magic, version, kind, length) == (0x36443357, 6, 1, len(frame))
    at, points = 32, []
    for _ in range(count):
        _, size = struct.unpack_from("<2I", frame, at)
        at += 40
        positions = struct.unpack_from(f"<{size * 3}f", frame, at)
        points.extend(zip(positions[::3], positions[1::3], positions[2::3]))
        at += (size * 15 + 3) // 4 * 4
    assert at == len(frame)
    assert all(math.isfinite(c) for point in points for c in point)
    return points


def browser_check(root, port, command_port, temp, screenshot):
    chrome = shutil.which("google-chrome") or shutil.which("chromium")
    if not chrome:
        raise RuntimeError("Chrome/Chromium required for browser validation")
    debug_port = free_port()
    proc = subprocess.Popen([
        chrome, "--headless", "--no-sandbox", "--disable-dev-shm-usage",
        "--enable-unsafe-swiftshader", "--no-first-run", "--no-default-browser-check",
        "--window-size=1500,1000", f"--user-data-dir={temp}/chrome",
        f"--remote-debugging-port={debug_port}", "about:blank",
    ], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
    cdp = None
    try:
        pages = json.loads(wait_http(f"http://127.0.0.1:{debug_port}/json", proc))
        cdp = WebSocket(next(p["webSocketDebuggerUrl"] for p in pages if p["type"] == "page"))
        serial = 0
        errors = []

        def call(method, **params):
            nonlocal serial
            serial += 1
            cdp.send(json.dumps(dict(id=serial, method=method, params=params)))
            while True:
                result = json.loads(cdp.receive())
                if result.get("method") == "Runtime.exceptionThrown":
                    errors.append(result["params"])
                if result.get("id") == serial:
                    assert "error" not in result, result
                    return result.get("result", {})

        def evaluate(expression):
            result = call("Runtime.evaluate", expression=expression, returnByValue=True,
                          awaitPromise=True)
            assert "exceptionDetails" not in result, result
            return result.get("result", {}).get("value")

        call("Runtime.enable")
        call("Page.enable")
        call("Page.navigate", url=f"http://127.0.0.1:{port}/?cmdPort={command_port}#connect")
        deadline = time.monotonic() + 60
        state = {}
        while time.monotonic() < deadline:
            state = evaluate("""({
              title: document.title,
              controls: document.querySelectorAll('[data-key]').length,
              text: document.body.innerText,
              connected: typeof wsSocket !== 'undefined' && wsSocket?.readyState === 1,
              canvas: !!document.querySelector('canvas')
            })""")
            if state and state["connected"] and state["canvas"] and state["controls"] > 40:
                break
            time.sleep(0.3)
        assert state["connected"] and state["canvas"] and state["controls"] > 40, state
        assert not errors, errors
        result = call("Page.captureScreenshot", format="png")
        Path(screenshot).write_bytes(base64.b64decode(result["data"]))
        print("Browser:", {k: v for k, v in state.items() if k != "text"})
    finally:
        if cdp:
            cdp.close()
        proc.terminate()
        try:
            proc.wait(timeout=5)
        except subprocess.TimeoutExpired:
            proc.kill()
            proc.wait()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("binary", type=Path)
    parser.add_argument("--screenshot", default="/tmp/openkai-spor-viewer.png")
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[3]
    binary = args.binary.resolve()
    with tempfile.TemporaryDirectory(prefix="openkai-spor-live-") as temp:
        config = json.loads((root / "jsonCfg/SporSensor.json").read_text())
        config["console"]["bON"] = False
        port, command_port = free_port(), free_port()
        config["viewer"]["host"] = config["wsServer"]["host"] = "127.0.0.1"
        config["viewer"]["port"] = port
        config["wsServer"]["port"] = command_port
        filename = Path(temp) / "SporSensor.json"
        filename.write_text(json.dumps(config, indent=2))
        logfile = Path("/tmp/openkai-spor-live.log")
        commands = points = None
        restore_trigger = None
        with logfile.open("w") as output:
            proc = subprocess.Popen([str(binary), str(filename)], cwd=root,
                                    stdout=output, stderr=subprocess.STDOUT)
            try:
                page = wait_http(f"http://127.0.0.1:{port}/", proc)
                assert b"Spor" in page
                commands = Commands(command_port)
                deadline = time.monotonic() + 60
                while True:
                    reply = commands.request("getConfig")
                    if reply.get("deviceOpen"):
                        break
                    assert time.monotonic() < deadline, reply
                    time.sleep(0.5)
                assert reply["bSuccess"], reply
                schema = reply["schema"]
                print("Device open; schema:", len(schema), "fields/groups")
                Path("/tmp/openkai-spor-live-config.json").write_text(json.dumps(reply, indent=2))
                original_config = reply["config"]
                original_values = reply["device"]["values"]
                # Exercise a verified SDK setter without changing optical power.
                power = original_values["laserPower"]
                applied = commands.request("setConfig", config={"laserPower": power})
                assert applied["bSuccess"], applied
                assert applied["device"]["values"]["laserPower"] == power, applied
                reapplied = commands.request("setConfig", config={"laserPower": power})
                assert reapplied["bSuccess"], reapplied
                assert reapplied["device"]["values"]["laserPower"] == power, reapplied
                cleared = commands.request("setConfig", config={"laserPower": original_config["laserPower"]})
                assert cleared["bSuccess"], cleared
                stopped = commands.request("stopAcquisition")
                assert stopped["bSuccess"] and not stopped["status"]["acquiring"], stopped
                for _ in range(2):
                    started = commands.request("startAcquisition")
                    assert started["bSuccess"] and started["status"]["acquiring"], started
                # A sparse trigger stream must not block console commands or
                # reset acquisition when the SDK's receive deadline expires.
                restore_trigger = original_values["triggerMode"]
                triggered = commands.request("setConfig", config={"triggerMode": 3})
                assert triggered["bSuccess"], triggered
                time.sleep(original_config["streamTimeoutMs"] / 1000 + 0.5)
                before = commands.request("getConfig")
                assert before["deviceOpen"] and before["status"]["acquiring"], before
                trigger = commands.request("softwareTrigger")
                assert trigger["bSuccess"], trigger
                deadline = time.monotonic() + 10
                while True:
                    after = commands.request("getConfig")
                    if after["status"]["frameCount"] > before["status"]["frameCount"]:
                        break
                    assert time.monotonic() < deadline, after
                    time.sleep(0.1)
                restored = commands.request("setConfig", config={"triggerMode": restore_trigger})
                assert restored["bSuccess"], restored
                restore_trigger = None
                cleared = commands.request("setConfig", config={"triggerMode": original_config["triggerMode"]})
                assert cleared["bSuccess"], cleared
                print("Verified SDK setter, acquisition restart, idle timeout and software trigger passed")
                invalid = commands.request("setConfig", config={"pointStride": 0})
                assert not invalid["bSuccess"], invalid
                applied = commands.request("setConfig", config={"pointStride": 2, "yOffsetM": 0.0125})
                assert applied["bSuccess"], applied
                saved = commands.request("saveConfig")
                assert saved["bSuccess"], saved
                persisted = json.loads(filename.read_text())
                assert persisted["sporSensor"]["pointStride"] == 2
                assert persisted["sporSensor"]["yOffsetM"] == 0.0125
                assert persisted["viewer"] == config["viewer"]
                points = WebSocket(f"ws://127.0.0.1:{port}/stream/points")
                hello = json.loads(points.receive())
                assert hello["version"] == 6 and hello["stream"] == "points"
                points.send("start")
                deadline = time.monotonic() + 25
                cloud = []
                while time.monotonic() < deadline:
                    cloud = inspect_cloud(points.receive())
                    if cloud and all(abs(p[1] - 0.0125) < 1e-6 for p in cloud):
                        break
                    points.send("next")
                assert cloud, "No valid profiles; place a target in the sensor's measurement range"
                assert all(abs(p[1] - 0.0125) < 1e-6 for p in cloud), cloud[:5]
                print(f"Live protocol v6: {len(cloud)} finite points; example {cloud[0]}")
                restored = commands.request("setConfig", config={"pointStride": 1, "yOffsetM": 0})
                assert restored["bSuccess"], restored
                commands.ws.close()
                commands = None
                points.close()
                points = None
                browser_check(root, port, command_port, temp, args.screenshot)
            finally:
                try:
                    if restore_trigger is not None and commands:
                        restored = commands.request("setConfig", config={"triggerMode": restore_trigger})
                        assert restored["bSuccess"], restored
                finally:
                    if commands:
                        commands.ws.close()
                    if points:
                        points.close()
                    proc.send_signal(signal.SIGINT)
                    try:
                        proc.wait(timeout=30)
                    except subprocess.TimeoutExpired:
                        proc.kill()
                        proc.wait()
                        raise AssertionError("OpenKAI did not shut down within 30 seconds")
                    assert proc.returncode == 0, logfile.read_text()[-6000:]
        print(f"PASS: live sensor, configuration, persistence, stream, browser, shutdown. {logfile}")


if __name__ == "__main__":
    main()
