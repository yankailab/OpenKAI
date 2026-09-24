"""Deterministic accumulating-submap transport regression; no camera or packages."""
import json
import select
import struct
import subprocess
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parents[1] / 'webViewer3D'))
from integration import WebSocket


class Fixture:
    def __init__(self, executable, root):
        self.process = subprocess.Popen([executable, root], stdin=subprocess.PIPE,
                                        stdout=subprocess.PIPE, text=True)
        self.info = self.reply()

    def reply(self):
        assert select.select([self.process.stdout], [], [], 5)[0], 'Fixture response timed out'
        line = self.process.stdout.readline()
        assert line, f'Fixture exited with {self.process.poll()}'
        reply = json.loads(line)
        assert 'error' not in reply, reply
        return reply

    def command(self, command, **fields):
        self.process.stdin.write(json.dumps({'cmd': command, **fields}) + '\n')
        self.process.stdin.flush()
        return self.reply()

    def close(self):
        if self.process.poll() is None:
            try:
                self.process.communicate('{"cmd":"quit"}\n', timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait()
                raise
        assert self.process.returncode == 0


class Client:
    def __init__(self, port):
        self.ws = WebSocket(port, '/stream/glim')
        opcode, data = self.ws.receive()
        hello = json.loads(data)
        assert opcode == 1 and hello['protocol'] == 'openkai.glim'
        assert hello['version'] == 1 and hello['stream'] == 'glim'
        self.chunk_limit = hello['maxChunkPoints']
        self.session = None
        self.submaps = {}
        self.geometry_messages = 0
        self.reset_messages = 0
        self.ws.send('start')

    def accept(self, message):
        opcode, data = message
        if opcode == 1:
            value = json.loads(data)
            assert isinstance(value['session'], str) and isinstance(value['revision'], str)
            if value['type'] == 'reset':
                self.session = int(value['session'])
                self.submaps.clear()
                self.reset_messages += 1
                return 'reset', None
            assert int(value['session']) == self.session
            assert isinstance(value['id'], str)
            identity = value['id']
            assert len(value['pose']) == 16
            if value['type'] == 'submap':
                assert identity not in self.submaps, 'Duplicate geometry announcement'
                assert isinstance(value['timestampUs'], str)
                self.submaps[identity] = dict(value, received=0, chunks=[])
            else:
                assert value['type'] == 'pose' and identity in self.submaps
                self.submaps[identity]['pose'] = value['pose']
            return value['type'], identity
        assert opcode == 2
        magic, version, kind, header, session, identity, timestamp, total, offset, count, reserved = struct.unpack_from('<4I3Q4I', data)
        assert (magic, version, kind, header, reserved) == (0x314D4C47, 1, 1, 56, 0)
        assert session == self.session and 0 < count <= self.chunk_limit
        assert len(data) == header + count * 12
        submap = self.submaps[str(identity)]
        assert timestamp == int(submap['timestampUs']) and total == submap['pointCount']
        assert offset == submap['received'], 'Missing, duplicated, or reordered geometry chunk'
        assert offset + count <= total
        serial = identity - 9007199254741092
        for index, point in enumerate(struct.iter_unpack('<3f', memoryview(data)[header:]), offset):
            assert point == (float(index), float(serial), -float(index % 97))
        submap['received'] += count
        submap['chunks'].append((offset, count))
        self.geometry_messages += 1
        return 'chunk', str(identity)

    def receive(self):
        return self.accept(self.ws.receive())

    def next(self):
        self.ws.send('next')

    def drain(self, expected):
        while set(self.submaps) != set(expected) or any(
                self.submaps[key]['received'] != count for key, count in expected.items()):
            self.receive()
            self.next()
        assert all(self.submaps[key]['pointCount'] == count for key, count in expected.items())

    def quiet(self):
        assert not select.select([self.ws.socket], [], [], .08)[0], 'Unexpected message without new data or credit'

    def close(self):
        self.ws.close()


def receive_during(ws, trigger):
    """Run trigger while a fragmented binary message is only partially read."""
    data = bytearray()
    first_message = True
    while True:
        first, second = ws.exact(2)
        assert not second & 128
        size = second & 127
        if size == 126:
            size = struct.unpack('!H', ws.exact(2))[0]
        elif size == 127:
            size = struct.unpack('!Q', ws.exact(8))[0]
        assert first & 15 == (2 if first_message else 0)
        prefix = min(size, 64)
        data.extend(ws.exact(prefix))
        if first_message:
            assert len(data) >= 56
            trigger()
        data.extend(ws.exact(size - prefix))
        first_message = False
        if first & 128:
            return 2, bytes(data)


def main():
    executable, root = sys.argv[1:]
    fixture = Fixture(executable, root)
    clients = []
    try:
        port = fixture.info['port']
        for route in ('/stream', '/stream/points', '/stream/lines', '/stream/cells'):
            try:
                unexpected = WebSocket(port, route)
            except AssertionError as error:
                assert '404' in str(error)
            else:
                unexpected.close()
                raise AssertionError(f'Unexpected stream accepted: {route}')
        slow, fast = Client(port), Client(port)
        clients.extend([slow, fast])
        assert slow.receive() == fast.receive() == ('reset', None)
        assert slow.session == fast.session == int(fixture.info['session'])
        fast.next()
        # Keep slow's reset unacknowledged while several complete submaps arrive.
        first_count = fixture.info['maxChunkPoints'] + 3
        first = fixture.command('append', count=first_count)['ids'][0]
        second = fixture.command('append', count=9)['ids'][1]
        third = fixture.command('append', count=7)['ids'][2]
        expected = {first: first_count, second: 9, third: 7}
        fast.drain(expected)
        assert fast.submaps[first]['chunks'] == [(0, first_count - 3), (first_count - 3, 3)]
        slow.quiet()
        slow.next()
        slow.drain(expected)
        assert slow.geometry_messages == fast.geometry_messages == 4

        fixture.command('same')
        fixture.command('same')
        slow.quiet()
        fast.quiet()

        # Pose corrections update an existing object without retransmitting points.
        fixture.command('pose', id=first)
        for client in (slow, fast):
            count = client.geometry_messages
            assert client.receive() == ('pose', first)
            assert client.submaps[first]['pose'][12:15] == [1, 2, 3]
            client.next()
            client.quiet()
            assert client.geometry_messages == count

        fast.close()
        replay = Client(port)
        clients.append(replay)
        assert replay.receive() == ('reset', None)
        replay.next()
        replay.drain(expected)
        assert replay.submaps[first]['pose'][12:15] == [1, 2, 3]
        assert replay.geometry_messages == 4

        # Reset after a large binary message starts, before its remaining bytes
        # are read. Reuse the first ID in the new session to expose stale state.
        pending = Client(port)
        clients.append(pending)
        assert pending.receive() == ('reset', None)
        pending.next()
        assert pending.receive() == ('submap', first)
        pending.next()
        old_session = pending.session

        def reset():
            fixture.command('reset')
            fixture.command('append', count=5)

        assert pending.accept(receive_during(pending.ws, reset)) == ('chunk', first)
        pending.next()
        assert pending.receive() == ('reset', None)
        assert pending.session == old_session + 1 and not pending.submaps
        pending.next()
        pending.drain({first: 5})
        assert pending.submaps[first]['chunks'] == [(0, 5)]
        for client in (slow, replay):
            assert client.receive() == ('reset', None)
            assert client.session == pending.session
            client.next()
            client.drain({first: 5})
        fixture.command('same')
        for client in (slow, replay, pending):
            client.quiet()
        print('PASS: GLIM stream routes, chunk offsets, lossless slow-client replay, reconnect, pose-only updates, in-flight reset, unchanged-revision silence')
    finally:
        for client in clients:
            client.close()
        fixture.close()


if __name__ == '__main__':
    main()
