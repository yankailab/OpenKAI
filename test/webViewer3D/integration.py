"""Standard-library integration checks; no npm or Python packages needed."""
import base64
import hashlib
import http.client
import json
import os
import socket
import struct
import subprocess
import sys
import tempfile
import time
from pathlib import Path


class WebSocket:
    def __init__(self, port, path='/stream/points'):
        self.socket = socket.create_connection(('127.0.0.1', port), timeout=3)
        key = base64.b64encode(os.urandom(16)).decode()
        self.socket.sendall((f'GET {path} HTTP/1.1\r\nHost: 127.0.0.1:{port}\r\n'
            f'Upgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: {key}\r\n'
            'Sec-WebSocket-Version: 13\r\n\r\n').encode())
        header = b''
        while not header.endswith(b'\r\n\r\n'):
            header += self.exact(1)
        if not header.startswith(b'HTTP/1.1 101 '):
            self.close()
            raise AssertionError(header)
        accept = base64.b64encode(hashlib.sha1((key + '258EAFA5-E914-47DA-95CA-C5AB0DC85B11').encode()).digest())
        assert accept.lower() in header.lower()

    def exact(self, length):
        data = bytearray()
        while len(data) < length:
            chunk = self.socket.recv(length - len(data))
            if not chunk:
                raise EOFError('WebSocket closed')
            data.extend(chunk)
        return bytes(data)

    def send(self, message, opcode=1, fin=True):
        payload = message.encode() if isinstance(message, str) else message
        mask = os.urandom(4)
        size = len(payload)
        length = bytes([128 | size]) if size < 126 else bytes([128 | 126]) + struct.pack('!H', size)
        self.socket.sendall(bytes([(128 if fin else 0) | opcode]) + length + mask +
                            bytes(c ^ mask[i % 4] for i, c in enumerate(payload)))

    def receive(self):
        data = bytearray()
        message_type = None
        while True:
            first, second = self.exact(2)
            size = second & 127
            if size == 126: size = struct.unpack('!H', self.exact(2))[0]
            elif size == 127: size = struct.unpack('!Q', self.exact(8))[0]
            assert not second & 128
            payload = self.exact(size)
            opcode = first & 15
            if opcode == 9:
                self.send(payload, 10)
                continue
            if opcode in (8, 10): return opcode, payload
            if message_type is None: message_type = opcode
            data.extend(payload)
            if first & 128: return message_type, bytes(data)

    def close(self):
        self.socket.close()


def request(port, target, method='GET'):
    conn = http.client.HTTPConnection('127.0.0.1', port, timeout=3)
    conn.request(method, target)
    response = conn.getresponse()
    result = response.status, dict(response.getheaders()), response.read()
    conn.close()
    return result


def check_frame(message):
    opcode, data = message
    assert opcode == 2
    magic, version, kind, sequence, count, size = struct.unpack_from('<6I', data)
    assert (magic, version, kind, count, size) == (0x34443357, 4, 1, 1, len(data))
    assert struct.unpack_from('<Q', data, 24)[0] == 123456789
    assert struct.unpack_from('<2I', data, 32) == (7, 200000)
    assert len(data) == 32 + 40 + 200000 * 16
    assert struct.unpack_from('<3f', data, 72) == (1, 0, -1)
    assert data[72 + 2400000:72 + 2400004] == bytes([0, 200, 240, 255])
    return sequence


def read_frame(data, expected_type):
    magic, version, kind, sequence, count, size = struct.unpack_from('<6I', data)
    assert (magic, version, kind, size) == (0x34443357, 4, expected_type, len(data))
    assert count <= 1024
    at, objects = 32, []
    for _ in range(count):
        object_id, n, point_size, opacity, *bounds = struct.unpack_from('<2I8f', data, at)
        at += 40
        obj = {'id': object_id, 'count': n, 'bounds': bounds, 'opacity': opacity}
        if kind == 3:
            obj['header'] = struct.unpack_from('<6fIIQ', data, at)
            at += 40
            obj['cells'] = memoryview(data)[at:at + n * 20]
            at += n * 20
        else:
            vertices = n * (2 if kind == 2 else 1)
            obj['positions'] = memoryview(data)[at:at + vertices * 12]
            at += vertices * 12
            obj['colors'] = memoryview(data)[at:at + vertices * 4]
            at += vertices * 4
        objects.append(obj)
    assert at == len(data)
    return objects


def main():
    fixture, root = sys.argv[1:]
    processes, clients = [], []
    with tempfile.TemporaryDirectory() as tmp:
        # Serve a root with an escaping symlink to prove containment.
        folder = Path(tmp) / 'web'
        folder.mkdir()
        (folder / 'index.html').write_text('fixture')
        (Path(tmp) / 'secret').write_text('private')
        (folder / 'escape').symlink_to(Path(tmp) / 'secret')
        try:
            p = subprocess.Popen([fixture, root, str(folder), '200000'], stdin=subprocess.PIPE, stdout=subprocess.PIPE, text=True)
            processes.append(p)
            ports = [int(port) for port in p.stdout.readline().split()]
            assert len(ports) == 2
            port, second = ports
            status, headers, body = request(port, '/')
            assert status == 200 and b'OpenKAI' in body
            assert request(port, '/js/protocol.js')[1]['Content-Type'].startswith('text/javascript')
            assert request(port, '/vendor/three.module.min.js', 'HEAD')[2] == b''
            assert request(port, '/missing')[0] == 404
            assert request(port, '/', 'POST')[0] == 405
            for path in ('/../secret', '/%2e%2e/secret', '/escape'):
                assert request(second, path)[0] == 403
            assert request(second, '/%00')[0] == 400
            # The old combined endpoint is deliberately gone.
            try:
                WebSocket(port, '/stream')
                raise RuntimeError('Legacy endpoint accepted')
            except AssertionError as error:
                assert '404' in str(error)
            # Type channels advance independently, even with no point ACKs.
            for kind, code, payload_size in [('lines', 2, 32), ('cells', 3, 40 + 41 * 20)]:
                typed = WebSocket(port, '/stream/' + kind); clients.append(typed)
                assert json.loads(typed.receive()[1])['stream'] == kind
                typed.send('start')
                _, data = typed.receive()
                assert struct.unpack_from('<I', data, 8)[0] == code
                assert len(data) == 32 + 40 + payload_size
            a, b, independent = [WebSocket(p) for p in (port, port, second)]
            clients.extend([a, b, independent])
            for c in (a, b, independent):
                opcode, hello = c.receive()
                assert opcode == 1 and json.loads(hello)['version'] == 4
                c.send('start')
                check_frame(c.receive())
            blocked = WebSocket(port)
            clients.append(blocked)
            blocked.receive()
            blocked.socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 1024)
            blocked.send('start')  # Do not read the multi-megabyte write at all.
            # No ACK means no queued snapshots, while a second peer continues.
            a.socket.settimeout(.15)
            try:
                a.receive()
                raise AssertionError('Server sent a frame without credit')
            except socket.timeout:
                pass
            a.socket.settimeout(3)
            for _ in range(3):
                b.send('next')
                check_frame(b.receive())
                for typed, kind in zip(clients[:2], (2, 3)):
                    typed.send('next')
                    read_frame(typed.receive()[1], kind)
            a.send('ne', fin=False)
            a.send('xt', opcode=0)
            assert check_frame(a.receive()) > 3
            b.send('ping payload', opcode=9)
            assert b.receive() == (10, b'ping payload')
            b.send('pause')
            b.send('next')
            b.socket.settimeout(.1)
            try:
                b.receive()
                raise AssertionError('Paused stream sent data')
            except socket.timeout:
                pass
            b.socket.settimeout(3)
            b.send('start')
            check_frame(b.receive())
            # Shutdown with live clients must finish and release both listeners.
            for p in processes:
                p.communicate('\n', timeout=3)
                assert p.returncode == 0
            for port in ports:
                with socket.socket() as s:
                    s.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
                    s.bind(('127.0.0.1', port))
            print('PASS: HTTP assets/HEAD/traversal, binary layout, fragmentation, ping, flow control, independent servers, shutdown')
        finally:
            for c in clients: c.close()
            for p in processes:
                if p.poll() is None:
                    p.kill()
                    p.wait()


if __name__ == '__main__':
    main()
