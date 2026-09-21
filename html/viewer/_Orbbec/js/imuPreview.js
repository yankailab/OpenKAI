import * as THREE from '../vendor/three.module.min.js';
const $ = selector => document.querySelector(selector);
const colors = ['#ff6666', '#69dc8d', '#669eff'];

export class IMUPreview {
  constructor() {
    this.streaming = false; this.pending = null; this.sequence = 0;
    this.lastSample = 0; this.lastDraw = 0; this.timestamps = [0, 0];
    this.graphs = Array.from({ length: 6 }, (_, i) => {
      const figure = document.createElement('figure'); figure.className = 'graph';
      const caption = document.createElement('figcaption');
      const name = document.createElement('span'); name.textContent = `${i < 3 ? 'Gyro' : 'Acc'} ${'XYZ'[i % 3]} (${i < 3 ? 'rad/s' : 'm/s²'})`;
      const value = document.createElement('span'); value.textContent = '—';
      const canvas = document.createElement('canvas'); canvas.setAttribute('aria-label', name.textContent);
      caption.append(name, value); figure.append(caption, canvas); $('#imuGraphs').append(figure);
      return { canvas, value, samples: [], color: colors[i % 3] };
    });
    this.scene = new THREE.Scene();
    this.camera = new THREE.PerspectiveCamera(35, 1, 0.1, 20);
    this.camera.position.set(3, 2, 3); this.camera.lookAt(0, 0, 0);
    this.axes = new THREE.Group(); this.scene.add(this.axes);
    for (let i = 0; i < 3; ++i) {
      const v = new THREE.Vector3(); v.setComponent(i, 1);
      this.axes.add(new THREE.ArrowHelper(v, new THREE.Vector3(), 1.25, colors[i], 0.22, 0.12));
    }
    this.scene.add(new THREE.AxesHelper(0.3));
    this.renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    this.renderer.setPixelRatio(Math.min(devicePixelRatio, 2));
    $('#orientation').append(this.renderer.domElement);
    this.onState = () => {
      if (window.wsSocket?.readyState !== WebSocket.OPEN) {
        this.streaming = false; this.pending = null; clearTimeout(this.timeout);
        $('#imuStatus').textContent = 'Stopped · command connection disconnected';
      }
      this.refresh();
    };
    this.onData = event => this.receive(event.detail);
    this.onReply = event => {
      const j = event.detail;
      if (j.module !== $('#imuModule').value.trim() || j.requestId !== this.pending) return;
      clearTimeout(this.timeout); this.pending = null;
      this.streaming = j.bSuccess && j.streaming;
      $('#imuStatus').textContent = j.bSuccess ? this.streaming ? 'Waiting for IMU samples…' : 'Stopped' : j.error || 'IMU command failed';
      this.refresh();
    };
    window.addEventListener('wscmdstatechange', this.onState);
    window.addEventListener('imudata', this.onData);
    window.addEventListener('orbbeccommand', this.onReply);
    $('#imuStart').onclick = () => this.command(true);
    $('#imuStop').onclick = () => this.command(false);
    this.refresh();
  }
  refresh() {
    const connected = window.wsSocket?.readyState === WebSocket.OPEN;
    $('#imuStart').disabled = !connected || this.streaming || !!this.pending;
    $('#imuStop').disabled = !connected || (!this.streaming && !this.pending);
    $('#imuModule').disabled = this.streaming || !!this.pending;
  }
  command(start) {
    const module = $('#imuModule').value.trim();
    if (!module) { $('#imuStatus').textContent = 'Enter an IMU module name'; return; }
    const requestId = `imu-${++this.sequence}`;
    if (!window.wsSendCmd({ module, cmd: start ? 'startStream' : 'stopStream', requestId })) return;
    this.pending = requestId; clearTimeout(this.timeout);
    if (start) {
      this.graphs.forEach(graph => { graph.samples = []; graph.value.textContent = '—'; });
      this.timestamps = [0, 0]; this.lastSample = performance.now();
      this.axes.quaternion.identity();
    }
    $('#imuStatus').textContent = start ? 'Starting IMU…' : 'Stopping IMU…';
    this.timeout = setTimeout(() => { this.pending = null; this.streaming = false; $('#imuStatus').textContent = 'No reply from IMU module'; this.refresh(); }, 10000);
    this.refresh();
  }
  receive(j) {
    if (j.module !== $('#imuModule').value.trim() || !this.streaming) return;
    const valid = (value, n) => Array.isArray(value) && value.length === n && value.every(Number.isFinite);
    if (!valid(j.gyro, 3) || !valid(j.acc, 3)) return;
    const now = performance.now(); this.lastSample = now;
    [j.gyro, j.acc].forEach((vector, kind) => {
      const t = kind ? j.tAcc : j.tGyro;
      if (!Number.isFinite(t) || t <= 0 || t === this.timestamps[kind]) return;
      if (t < this.timestamps[kind]) this.graphs.slice(kind * 3, kind * 3 + 3).forEach(graph => { graph.samples = []; });
      this.timestamps[kind] = t;
      vector.forEach((v, axis) => {
        const graph = this.graphs[kind * 3 + axis];
        graph.samples.push([now, v]);
        if (graph.samples.length > 1000) graph.samples.shift();
        graph.value.textContent = v.toFixed(3);
      });
    });
    if (j.orientationValid && valid(j.quaternion, 4) && Math.hypot(...j.quaternion) > 0) {
      const [w, x, y, z] = j.quaternion;
      this.axes.quaternion.set(x, y, z, w).normalize();
      if (valid(j.rpy, 3)) $('#angles').textContent = j.rpy.map((v, i) => `${['Roll', 'Pitch', 'Yaw'][i]} ${(v * 180 / Math.PI).toFixed(1)}°`).join(' · ');
    }
    $('#imuStatus').textContent = j.orientationValid ? 'Streaming · fused orientation' : j.fusion ? 'Streaming · waiting for fusion' : 'Streaming · fusion disabled';
  }
  render(now) {
    if (now - this.lastDraw < 66) return;
    this.lastDraw = now;
    if (this.streaming && now - this.lastSample > 2000) $('#imuStatus').textContent = 'Waiting for fresh IMU samples…';
    const container = $('#orientation'), width = container.clientWidth, height = container.clientHeight;
    if (width && height) {
      this.renderer.setSize(width, height, false); this.camera.aspect = width / height; this.camera.updateProjectionMatrix();
      this.renderer.render(this.scene, this.camera);
    }
    for (const graph of this.graphs) {
      const { canvas, samples } = graph;
      while (samples.length && samples[0][0] < now - 10000) samples.shift();
      const ratio = Math.min(devicePixelRatio, 2);
      const w = Math.round(canvas.clientWidth * ratio), h = Math.round(canvas.clientHeight * ratio);
      if (canvas.width !== w || canvas.height !== h) { canvas.width = w; canvas.height = h; }
      const ctx = canvas.getContext('2d'); ctx.clearRect(0, 0, w, h);
      const range = Math.max(0.01, ...samples.map(s => Math.abs(s[1]))) * 1.15;
      ctx.strokeStyle = '#253747'; ctx.lineWidth = ratio; ctx.beginPath(); ctx.moveTo(0, h / 2); ctx.lineTo(w, h / 2); ctx.stroke();
      ctx.fillStyle = '#8299ac'; ctx.font = `${9 * ratio}px system-ui`; ctx.fillText(`±${range.toFixed(2)} · 10 s`, 4 * ratio, 11 * ratio);
      ctx.strokeStyle = graph.color; ctx.lineWidth = 1.4 * ratio; ctx.beginPath();
      samples.forEach(([t, v], i) => { const x = w * (1 - (now - t) / 10000), y = h / 2 - v / range * h * 0.43; if (i) ctx.lineTo(x, y); else ctx.moveTo(x, y); }); ctx.stroke();
    }
  }
  dispose() {
    clearTimeout(this.timeout);
    window.removeEventListener('wscmdstatechange', this.onState);
    window.removeEventListener('imudata', this.onData);
    window.removeEventListener('orbbeccommand', this.onReply);
    this.scene.traverse(o => { o.geometry?.dispose(); o.material?.dispose(); });
    this.renderer.dispose();
  }
}
