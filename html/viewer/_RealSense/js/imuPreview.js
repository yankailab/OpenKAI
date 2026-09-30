import * as THREE from '../vendor/three.module.min.js';
const $ = selector => document.querySelector(selector);
const colors = ['#ff6666', '#69dc8d', '#669eff'];
const validVector = (value, size) => Array.isArray(value) && value.length === size && value.every(Number.isFinite);
// Capture timestamps are strings of nanoseconds, including values above Number.MAX_SAFE_INTEGER.
const captureTime = value => typeof value === 'string' && /^[0-9]+$/.test(value) ? BigInt(value) : 0n;

export class IMUPreview {
  constructor() {
    this.streaming = false; this.pending = null; this.sequence = 0;
    this.module = ''; this.available = false; this.lastSample = 0; this.lastDraw = 0; this.timestamps = [0n, 0n];
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
        this.stop('Stopped · command connection disconnected');
      }
      this.refresh();
    };
    this.onModule = () => this.stop('Stopped · camera module changed');
    this.onReply = event => this.receive(event.detail);
    window.addEventListener('wscmdstatechange', this.onState);
    window.addEventListener('realsensecommand', this.onReply);
    $('#cameraModule').addEventListener('change', this.onModule);
    $('#imuStart').onclick = () => this.start();
    $('#imuStop').onclick = () => this.stop();
    this.refresh();
  }

  refresh() {
    const connected = window.wsSocket?.readyState === WebSocket.OPEN;
    $('#imuStart').disabled = !connected || this.streaming;
    $('#imuStop').disabled = !this.streaming;
  }

  resetOrientation() {
    this.axes.quaternion.identity();
    $('#angles').textContent = 'Roll — · Pitch — · Yaw —';
  }

  clearData(resetTime = true) {
    this.available = false;
    this.graphs.forEach(graph => { graph.samples = []; graph.value.textContent = '—'; });
    if (resetTime) this.timestamps = [0n, 0n];
    this.resetOrientation();
  }

  start() {
    if (this.streaming) return;
    this.module = $('#cameraModule').value.trim();
    if (!this.module) { $('#imuStatus').textContent = 'Enter a camera module name'; return; }
    if (window.wsSocket?.readyState !== WebSocket.OPEN) {
      this.stop('Stopped · command connection disconnected'); return;
    }
    this.clearData(); this.streaming = true; this.lastSample = performance.now();
    $('#imuStatus').textContent = 'Waiting for IMU samples…';
    this.refresh(); this.poll();
  }

  stop(message = 'Stopped') {
    clearTimeout(this.timer); clearTimeout(this.timeout);
    this.streaming = false; this.pending = null;
    this.clearData(); $('#imuStatus').textContent = message;
    this.refresh();
  }

  poll() {
    if (!this.streaming || this.pending) return;
    const requestId = `imu-${++this.sequence}`;
    this.pending = requestId;
    this.timeout = setTimeout(() => this.stop('No reply from camera IMU'), 15000);
    if (!window.wsSendCmd({ module: this.module, cmd: 'getIMU', requestId })) {
      this.stop('IMU request was not sent · check the command connection');
    }
  }

  receive(j) {
    if (!this.streaming || !this.pending || j.cmd !== 'getIMU' ||
        j.module !== this.module || j.requestId !== this.pending) return;
    clearTimeout(this.timeout); this.pending = null;
    if (!j.bSuccess) { this.stop(j.error || 'IMU command failed'); return; }
    // One outstanding request bounds traffic when the camera or network is busy.
    this.timer = setTimeout(() => this.poll(), 100);
    if (!j.available) {
      this.clearData();
      $('#imuStatus').textContent = !j.enabled ? 'Embedded IMU is disabled in camera controls' :
        !j.deviceOpen ? 'Camera is not open' : 'Waiting for fresh IMU samples…';
      return;
    }
    const times = [captureTime(j.tGyro), captureTime(j.tAcc)];
    if (!validVector(j.gyro, 3) || !validVector(j.acc, 3) || times.some(t => t === 0n)) {
      this.clearData(); $('#imuStatus').textContent = 'Invalid IMU sample'; return;
    }
    if (times.every((t, i) => t === this.timestamps[i])) return;
    this.available = true;
    const now = performance.now();
    [j.gyro, j.acc].forEach((vector, kind) => {
      const t = times[kind];
      if (t === this.timestamps[kind]) return;
      if (t < this.timestamps[kind]) {
        this.graphs.slice(kind * 3, kind * 3 + 3).forEach(graph => { graph.samples = []; });
      }
      this.timestamps[kind] = t; this.lastSample = now;
      vector.forEach((v, axis) => {
        const graph = this.graphs[kind * 3 + axis];
        graph.samples.push([now, v]);
        if (graph.samples.length > 1000) graph.samples.shift();
        graph.value.textContent = v.toFixed(3);
      });
    });
    const oriented = j.orientationValid && validVector(j.quaternion, 4) &&
      Math.hypot(...j.quaternion) > 0 && validVector(j.rpy, 3);
    if (oriented) {
      const [w, x, y, z] = j.quaternion;
      this.axes.quaternion.set(x, y, z, w).normalize();
      $('#angles').textContent = j.rpy.map((v, i) => `${['Roll', 'Pitch', 'Yaw'][i]} ${(v * 180 / Math.PI).toFixed(1)}°`).join(' · ');
    } else this.resetOrientation();
    $('#imuStatus').textContent = oriented ? 'Streaming · fused orientation' : 'Streaming · waiting for orientation';
  }

  render(now) {
    if (now - this.lastDraw < 66) return;
    this.lastDraw = now;
    if (this.streaming && this.available && now - this.lastSample > 2000) {
      this.clearData(false); $('#imuStatus').textContent = 'Waiting for fresh IMU samples…';
    }
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
    clearTimeout(this.timer); clearTimeout(this.timeout);
    this.streaming = false; this.pending = null;
    window.removeEventListener('wscmdstatechange', this.onState);
    window.removeEventListener('realsensecommand', this.onReply);
    $('#cameraModule').removeEventListener('change', this.onModule);
    this.scene.traverse(o => { o.geometry?.dispose(); o.material?.dispose(); });
    this.renderer.dispose();
  }
}
