// Geometry transport only. Application JSON commands use wsCmdBase.js.
export class GeometryConnection {
  constructor({ onHello, onFrame, onStatus, onReset }) {
    Object.assign(this, { onHello, onFrame, onStatus, onReset });
    this.socket = null;
    this.timer = null;
    this.running = false;
  }
  start(endpoint) {
    this.stop();
    this.url = new URL('stream', endpoint);
    this.url.protocol = endpoint.protocol === 'https:' ? 'wss:' : 'ws:';
    this.running = true;
    this.delay = 500;
    this.connect();
  }
  connect() {
    this.onReset();
    this.onStatus('Connecting…');
    const socket = this.socket = new WebSocket(this.url);
    socket.binaryType = 'arraybuffer';
    let greeted = false;
    this.timer = setTimeout(() => { if (this.socket === socket && !greeted) socket.close(); }, 10000);
    socket.onmessage = event => {
      if (this.socket !== socket) return;
      try {
        if (typeof event.data === 'string') {
          const hello = JSON.parse(event.data);
          if (greeted || hello.type !== 'hello' || (hello.version !== 1 && hello.version !== 2)) throw new Error('Incompatible backend');
          this.onHello(hello);
          greeted = true;
          clearTimeout(this.timer);
          this.delay = 500;
          socket.send('start');
          this.onStatus('Connected');
        } else {
          if (!greeted) throw new Error('Missing server configuration');
          this.onFrame(event.data, () => {
            if (this.socket === socket && socket.readyState === WebSocket.OPEN) socket.send('next');
          });
        }
      } catch (error) { this.fail(error); }
    };
    socket.onerror = () => { if (this.socket === socket) this.onStatus('Connection failed'); };
    socket.onclose = () => {
      if (this.socket !== socket || !this.running) return;
      clearTimeout(this.timer);
      this.onStatus(`Disconnected · retrying in ${this.delay / 1000}s`);
      this.timer = setTimeout(() => this.connect(), this.delay);
      this.delay = Math.min(this.delay * 2, 8000);
    };
  }
  fail(error) { this.stop(); this.onStatus(error.message); }
  stop() {
    this.running = false;
    clearTimeout(this.timer);
    const socket = this.socket;
    this.socket = null;
    if (socket) socket.close();
    this.onReset();
    this.onStatus('Stopped');
  }
}
