import { PROTOCOL_VERSION, PROTOCOL_NAME, decodeEvent, decodeChunk } from './protocol.js';
// A single render credit bounds transfer and browser work, including replay.
export class GLIMConnection {
  constructor({ onHello, onEvent, onStatus, onReset }) {
    Object.assign(this, { onHello, onEvent, onStatus, onReset });
    this.socket = null; this.timer = null; this.running = false;
  }
  start(endpoint) {
    this.stop();
    this.url = new URL('stream/glim', endpoint);
    this.url.protocol = endpoint.protocol === 'https:' ? 'wss:' : 'ws:';
    this.running = true; this.delay = 500; this.connect();
  }
  connect() {
    this.onReset(); this.onStatus('Connecting…');
    const socket = this.socket = new WebSocket(this.url);
    socket.binaryType = 'arraybuffer';
    let greeted = false, pending = false;
    this.timer = setTimeout(() => { if (this.socket === socket && !greeted) socket.close(); }, 10000);
    socket.onmessage = event => {
      if (this.socket !== socket) return;
      try {
        if (!greeted) {
          if (typeof event.data !== 'string') throw new Error('Missing GLIM server configuration');
          const hello = JSON.parse(event.data);
          if (hello.type !== 'hello' || hello.protocol !== PROTOCOL_NAME || hello.version !== PROTOCOL_VERSION || hello.stream !== 'glim')
            throw new Error('Incompatible GLIM backend');
          this.onHello(hello); greeted = true; clearTimeout(this.timer); this.delay = 500;
          socket.send('start'); this.onStatus('Connected'); return;
        }
        if (pending) throw new Error('GLIM backend exceeded stream credit');
        const message = typeof event.data === 'string' ? decodeEvent(event.data) : decodeChunk(event.data);
        pending = true;
        this.onEvent(message, () => {
          if (this.socket !== socket || socket.readyState !== WebSocket.OPEN || !pending) return;
          pending = false; socket.send('next');
        });
      } catch (error) { this.fail(error); }
    };
    socket.onerror = () => { if (this.socket === socket) this.onStatus('Connection failed'); };
    socket.onclose = () => {
      if (this.socket !== socket || !this.running) return;
      clearTimeout(this.timer); this.onReset(); this.onStatus(`Disconnected · retrying in ${this.delay / 1000}s`);
      this.timer = setTimeout(() => this.connect(), this.delay); this.delay = Math.min(this.delay * 2, 8000);
    };
  }
  fail(error) { this.stop(); this.onStatus(error.message); }
  stop() {
    this.running = false; clearTimeout(this.timer);
    const socket = this.socket; this.socket = null;
    if (socket) socket.close();
    this.onReset(); this.onStatus('Stopped');
  }
}
