"""Small WebSocket peer emulating _WSconsole's JSON/EOJ command framing."""
import base64
import hashlib
import json
import socketserver
import struct
import threading


class CommandHandler(socketserver.StreamRequestHandler):
    def send_text(self, text):
        data = text.encode()
        header = bytes([0x81, len(data)]) if len(data) < 126 else b'\x81\x7e' + struct.pack('!H', len(data))
        self.wfile.write(header + data)

    def handle(self):
        try:
            headers = {}
            self.rfile.readline()
            while True:
                line = self.rfile.readline()
                if line in (b'\r\n', b''): break
                key, value = line.decode().split(':', 1)
                headers[key.lower()] = value.strip()
            accept = base64.b64encode(hashlib.sha1((headers['sec-websocket-key'] +
                '258EAFA5-E914-47DA-95CA-C5AB0DC85B11').encode()).digest())
            self.wfile.write(b'HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\n'
                             b'Connection: Upgrade\r\nSec-WebSocket-Accept: ' + accept + b'\r\n\r\n')
            self.send_text('{"cmd":"hb"}')
            while True:
                header = self.rfile.read(2)
                if len(header) != 2: return
                opcode, length = header[0] & 15, header[1] & 127
                if length == 126: length = struct.unpack('!H', self.rfile.read(2))[0]
                elif length == 127: length = struct.unpack('!Q', self.rfile.read(8))[0]
                mask = self.rfile.read(4)
                payload = bytes(c ^ mask[i % 4] for i, c in enumerate(self.rfile.read(length)))
                if opcode == 8:
                    self.wfile.write(bytes([0x88, len(payload)]) + payload)
                    return
                assert opcode == 1 and payload.endswith(b'EOJ'), payload
                command = json.loads(payload[:-3])
                self.server.received.append(command)
                reply = json.dumps({'cmd': 'ackTest', 'v': command['v'], 'text': 'quoted " } EOJ ' + 'x' * 1024})
                # Separate text messages, then a split optional terminator and another object.
                for at in range(0, len(reply), 512): self.send_text(reply[at:at + 512])
                self.send_text('E')
                self.send_text('OJ{"cmd":"hb"}')
        except (ConnectionError, OSError):
            pass


class CommandServer(socketserver.ThreadingTCPServer):
    allow_reuse_address = True
    daemon_threads = True

    def __init__(self):
        super().__init__(('127.0.0.1', 0), CommandHandler)
        self.received = []
        self.worker = threading.Thread(target=self.serve_forever, daemon=True)
        self.worker.start()

    @property
    def port(self):
        return self.server_address[1]

    def close(self):
        self.shutdown()
        self.server_close()
        self.worker.join()
