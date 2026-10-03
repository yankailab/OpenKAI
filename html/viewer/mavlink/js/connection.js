import { parseMessage, socketUrl } from './protocol.js';

export class MavlinkConnection {
  constructor({ onState, onHello, onTelemetry, onReset }) {
    Object.assign(this, { onState, onHello, onTelemetry, onReset });
    this.generation = 0;
    this.active = false;
  }
  start(endpoint) {
    this.stop();
    this.endpoint = endpoint;
    this.url = socketUrl(endpoint);
    this.active = true;
    this.attempt = 0;
    this.connect(this.generation);
  }
  stop() {
    this.active = false;
    this.generation++;
    clearTimeout(this.timer);
    clearTimeout(this.watchdog);
    if (this.ws) { this.ws.onclose = null; this.ws.close(); this.ws = null; }
    this.onReset();
    this.onState('stopped', 'Disconnected');
  }
  connect(generation) {
    if (!this.active || generation !== this.generation) return;
    this.onReset();
    this.onState('connecting', 'Connecting to MAVLink stream…');
    const ws = this.ws = new WebSocket(this.url);
    let hello = false, protocolFailed = false, sequence = -1;
    const armWatchdog = () => {
      clearTimeout(this.watchdog);
      this.watchdog = setTimeout(() => { if (this.ws === ws) ws.close(4000, 'Telemetry timeout'); }, 15000);
    };
    armWatchdog();
    ws.onmessage = event => {
      if (this.ws !== ws || generation !== this.generation || protocolFailed) return;
      try {
        const message = parseMessage(event.data);
        if (message.type === 'hello') {
          if (hello) throw new Error('Duplicate MAVLink viewer hello');
          hello = true;
          this.attempt = 0;
          this.onHello(message.config, this.endpoint);
          this.onState('connected', 'Stream connected · waiting for MAVLink');
        } else {
          if (!hello) throw new Error('Telemetry arrived before hello');
          if (message.sequence <= sequence) return;
          sequence = message.sequence;
          this.onTelemetry(message);
        }
        armWatchdog();
      } catch (error) {
        protocolFailed = true;
        this.onState('error', `Stream protocol error: ${error.message}`);
        ws.close(1002, 'Invalid MAVLink viewer protocol');
      }
    };
    ws.onerror = () => {
      if (this.ws === ws && generation === this.generation) this.onState('error', 'WebSocket unavailable; check host, port and /stream/mavlink');
    };
    ws.onclose = () => {
      if (!this.active || generation !== this.generation) return;
      clearTimeout(this.watchdog);
      const delay = Math.min(10000, 1000 * 2 ** Math.min(this.attempt++, 4));
      this.onState('retrying', `Stream disconnected · reconnecting in ${delay / 1000}s`);
      this.timer = setTimeout(() => this.connect(generation), delay);
    };
  }
}
