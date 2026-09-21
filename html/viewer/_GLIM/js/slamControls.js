const $ = selector => document.querySelector(selector);
const number = (value, digits = 3) => Number.isFinite(value) ? value.toFixed(digits) : '—';

export class SLAMControls {
  constructor(viewer) {
    this.viewer = viewer;
    this.serial = 0;
    this.status = null;
    this.pending = null;
    this.lastReply = 0;
    this.connected = false;
    this.listeners = [];
    const listen = (target, type, handler) => {
      target.addEventListener(type, handler);
      this.listeners.push(() => target.removeEventListener(type, handler));
    };
    listen(window, 'wscmdstatechange', () => this.connectionChanged());
    listen(window, 'slamcommand', event => this.receive(event.detail));
    listen($('#slamModule'), 'change', () => {
      this.clear(); this.request('getStatus');
    });
    for (const cmd of ['start', 'stop', 'reset'])
      listen($(`#slam${cmd[0].toUpperCase()}${cmd.slice(1)}`), 'click', () => this.request(cmd));
    listen($('#fitMap'), 'click', () => viewer.fit());
    listen($('#resetView'), 'click', () => viewer.resetCamera());
    listen($('#pointColorMode'), 'change', () => viewer.setPointColorMode($('#pointColorMode').value));
    viewer.setPointColorMode($('#pointColorMode').value);
    listen($('#followSensor'), 'change', () => { viewer.followSensor = $('#followSensor').checked; });
    listen($('#showSensor'), 'change', () => { viewer.showSensor = $('#showSensor').checked; });
    for (const id of ['forwardAxis', 'fovH', 'fovV', 'fovRange'])
      listen($(`#${id}`), 'change', () => this.configureFoV());
    this.configureFoV();
    this.connectionChanged();
  }
  clear() {
    this.pending = null; this.status = null; this.lastReply = 0;
    this.viewer.setSensorPose(null);
    for (const id of ['posX', 'posY', 'posZ', 'roll', 'pitch', 'yaw', 'quatX', 'quatY', 'quatZ', 'quatW', 'mapPoints', 'submaps', 'liveFrames', 'frames', 'frameRate', 'processingMs'])
      $(`#${id}`).textContent = '—';
    $('#poseStatus').textContent = 'No pose received';
  }
  connectionChanged() {
    const connected = window.wsSocket?.readyState === WebSocket.OPEN;
    if (connected !== this.connected) this.clear();
    this.connected = connected;
    if (connected) this.request('getStatus');
    else {
      $('#slamStatus').textContent = 'Disconnected';
      $('#slamStatus').dataset.state = 'disconnected';
      $('#commandStatus').textContent = 'Connect to control SLAM';
    }
    this.buttons();
  }
  buttons() {
    const busy = this.pending && this.pending.cmd !== 'getStatus';
    const ready = this.connected && this.status && performance.now() - this.lastReply < 5000 && !busy;
    $('#slamStart').disabled = !ready || this.status.tracking;
    $('#slamStop').disabled = !ready || !this.status.tracking;
    $('#slamReset').disabled = !ready;
    $('#slamModule').disabled = Boolean(busy);
  }
  request(cmd) {
    if (!this.connected || (this.pending && this.pending.cmd !== 'getStatus')) return;
    const module = $('#slamModule').value.trim();
    if (!module) { $('#commandStatus').textContent = 'Enter a SLAM module name'; return; }
    const requestId = `glim-${++this.serial}`;
    if (!window.wsSendCmd({ cmd, module, requestId })) return;
    this.pending = { cmd, module, requestId, since: performance.now() };
    if (cmd !== 'getStatus') $('#commandStatus').textContent = { start: 'Starting SLAM…', stop: 'Finishing and optimizing the map…', reset: 'Stopping and clearing the map…' }[cmd];
    this.buttons();
  }
  receive(reply) {
    if (!this.pending || reply.module !== this.pending.module || reply.requestId !== this.pending.requestId) return;
    const cmd = this.pending.cmd;
    this.pending = null;
    this.lastReply = performance.now();
    if (reply.status) {
      this.status = reply.status;
      this.display(reply.status);
      this.viewer.setSensorPose(reply.status);
    }
    if (reply.bSuccess === false) $('#commandStatus').textContent = reply.error || 'SLAM command failed';
    else if (cmd !== 'getStatus') {
      $('#commandStatus').textContent = { start: 'SLAM started', stop: 'Stopped · map retained', reset: 'Map and pose cleared' }[cmd];
      if (cmd === 'reset' || cmd === 'start') { this.viewer.clear(); this.viewer.boundTypes.clear(); }
    } else $('#commandStatus').textContent = reply.status?.mapping ? 'Connected · global mapping enabled' : 'Connected · odometry only';
    this.buttons();
  }
  display(status) {
    $('#slamStatus').textContent = { stopped: 'Stopped', tracking: 'Tracking', initializing: 'Initializing', waiting: 'Waiting for pose', error: 'SLAM error' }[status.state] || status.state;
    $('#slamStatus').dataset.state = status.state;
    $('#poseStatus').textContent = status.poseFresh ? 'Pose available · map frame' : status.poseValid ? 'Last pose · no fresh estimate' : 'Waiting for a pose';
    for (const [ids, values, digits] of [
      [['posX', 'posY', 'posZ'], status.position, 3],
      [['roll', 'pitch', 'yaw'], status.angles, 1],
      [['quatX', 'quatY', 'quatZ', 'quatW'], status.orientation, 4]
    ]) ids.forEach((id, i) => { $(`#${id}`).textContent = status.poseValid ? number(values?.[i], digits) : '—'; });
    for (const id of ['mapPoints', 'submaps', 'liveFrames', 'frames'])
      $(`#${id}`).textContent = Number.isFinite(status[id]) ? status[id].toLocaleString() : '—';
    $('#frameRate').textContent = status.tracking && status.frameIntervalMs > 0 ? number(1000 / status.frameIntervalMs, 1) : '—';
    $('#processingMs').textContent = status.tracking ? number(status.processingMs, 1) : '—';
  }
  configureFoV() {
    const fields = ['fovH', 'fovV', 'fovRange'].map(id => $(`#${id}`));
    if (fields.some(field => !field.value || !field.checkValidity())) {
      $('#commandStatus').textContent = 'Enter a valid FoV and range'; return;
    }
    this.viewer.configureSensor(...fields.map(field => Number(field.value)), $('#forwardAxis').value);
  }
  tick(now) {
    const showDistance = this.viewer.pointColorUniforms.eyeColorEnabled.value && this.viewer.hasDistanceRange;
    $('#distanceLegend').hidden = !showDistance;
    if (showDistance) {
      const range = this.viewer.pointColorUniforms.eyeColorRange.value;
      $('#distanceNear').textContent = `${number(range.x, 2)} m · near`;
      $('#distanceFar').textContent = `${number(range.y, 2)} m · far`;
    }
    if (!this.connected) return;
    if (this.pending && now - this.pending.since > (this.pending.cmd === 'getStatus' ? 5000 : 120000)) {
      $('#commandStatus').textContent = `No reply from ${this.pending.module}; check the module name and backend log`;
      this.pending = null;
    }
    if ((!this.pending || this.pending.cmd === 'getStatus') && now - this.lastReply > 5000) {
      $('#slamStatus').textContent = 'Status unavailable'; $('#slamStatus').dataset.state = 'stale';
      $('#poseStatus').textContent = 'Telemetry is stale';
      this.viewer.markSensorStale();
    }
    if (!this.pending && now - this.lastReply >= 250) this.request('getStatus');
    this.buttons();
  }
  dispose() { for (const remove of this.listeners) remove(); }
}
