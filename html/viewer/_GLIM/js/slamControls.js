const $ = selector => document.querySelector(selector);
const number = (value, digits = 3) => Number.isFinite(value) ? value.toFixed(digits) : '—';
const PARAMETERS = [
  ['', 'General', [['bMapping', 'Global mapping', 'checkbox'], ['nMinPoints', 'Minimum points', 10, 1000000, 1]]],
  ['preprocess', 'Input filtering', [['distanceNear', 'Near distance · m', 0, 100000, 'any'], ['distanceFar', 'Far distance · m', 0.001, 100000, 'any'], ['voxelResolution', 'Voxel size · m', 0.001, 1000, 'any'], ['targetPoints', 'Target points · 0 auto', 0, 1000000, 1], ['kNeighbors', 'Neighbors', 3, 1000, 1], ['threads', 'Threads', 1, 128, 1]]],
  ['odometry', 'Odometry', [['voxelResolution', 'Voxel size · m', 0.001, 1000, 'any'], ['iterations', 'Iterations', 1, 1000, 1], ['threads', 'Threads', 1, 128, 1]]],
  ['submap', 'Submaps', [['keyframes', 'Keyframes / submap', 2, 1000, 1], ['keyframeStrategy', 'Keyframe selection', ['OVERLAP', 'DISPLACEMENT']], ['keyframeTranslation', 'Translation · m', 0, 10000, 'any'], ['keyframeRotation', 'Rotation · rad', 0, 6.283185307179586, 'any'], ['maxOverlap', 'Maximum overlap', 0, 1, 'any'], ['voxelResolution', 'Voxel size · m', 0.001, 1000, 'any']]],
  ['global', 'Global map', [['voxelResolution', 'Voxel size · m', 0.001, 1000, 'any'], ['loopDistance', 'Loop distance · m', 0, 100000, 'any'], ['loopOverlap', 'Loop overlap', 0, 1, 'any']]]
];
export class SLAMControls {
  constructor(viewer) {
    this.viewer = viewer; this.serial = 0; this.status = null; this.pending = null;
    this.lastReply = 0; this.connected = false; this.config = null; this.dirty = false; this.listeners = [];
    const listen = (target, type, handler) => {
      target.addEventListener(type, handler); this.listeners.push(() => target.removeEventListener(type, handler));
    };
    listen(window, 'wscmdstatechange', () => this.connectionChanged());
    listen(window, 'slamcommand', event => this.receive(event.detail));
    listen($('#slamModule'), 'change', () => { this.clear(); this.request('getConfig'); });
    listen($('#slamStart'), 'click', () => {
      if (!this.dirty) { this.request('start'); return; }
      const config = this.readConfig(); if (config) this.request('setConfig', { config }, 'start');
    });
    for (const cmd of ['stop', 'reset']) listen($(`#slam${cmd[0].toUpperCase()}${cmd.slice(1)}`), 'click', () => this.request(cmd));
    listen($('#savePointCloud'), 'click', () => this.request('savePointCloud'));
    listen($('#saveParameters'), 'click', () => {
      const config = this.readConfig(); if (config) this.request('saveConfig', { config });
    });
    listen($('#loadParameters'), 'click', () => this.request('loadConfig'));
    listen($('#parameterForm'), 'submit', event => event.preventDefault());
    listen($('#parameterFields'), 'input', () => {
      this.dirty = true; $('#parameterStatus').textContent = 'Unsaved edits · applied on Start'; this.parameterDependencies();
    });
    listen($('#fitMap'), 'click', () => viewer.fit());
    listen($('#resetView'), 'click', () => viewer.resetCamera());
    listen($('#pointColorMode'), 'change', () => viewer.setPointColorMode($('#pointColorMode').value));
    viewer.setPointColorMode($('#pointColorMode').value);
    listen($('#followSensor'), 'change', () => { viewer.followSensor = $('#followSensor').checked; });
    listen($('#showSensor'), 'change', () => { viewer.showSensor = $('#showSensor').checked; });
    listen($('#showTrajectory'), 'change', () => { viewer.trajectory.visible = $('#showTrajectory').checked; });
    for (const id of ['forwardAxis', 'fovH', 'fovV', 'fovRange']) listen($(`#${id}`), 'change', () => this.configureFoV());
    this.configureFoV(); this.connectionChanged();
  }
  clear() {
    this.pending = null; this.status = null; this.lastReply = 0; this.config = null; this.dirty = false;
    this.viewer.setSensorPose(null); this.viewer.clearTrajectory(); this.viewer.trajectorySession = null;
    $('#parameterFields').replaceChildren(); $('#parameterStatus').textContent = 'Connect to load parameters';
    for (const id of ['posX', 'posY', 'posZ', 'roll', 'pitch', 'yaw', 'quatX', 'quatY', 'quatZ', 'quatW', 'mapPoints', 'submaps', 'liveFrames', 'frames', 'frameRate', 'processingMs']) $(`#${id}`).textContent = '—';
    $('#poseStatus').textContent = 'No pose received';
  }
  connectionChanged() {
    const connected = window.wsSocket?.readyState === WebSocket.OPEN;
    if (connected !== this.connected) this.clear();
    this.connected = connected;
    if (connected) this.request('getConfig');
    else {
      $('#slamStatus').textContent = 'Disconnected'; $('#slamStatus').dataset.state = 'disconnected';
      $('#commandStatus').textContent = 'Connect to control SLAM';
    }
    this.buttons();
  }
  buttons() {
    const busy = this.pending && this.pending.cmd !== 'getStatus';
    const ready = this.connected && this.status && performance.now() - this.lastReply < 5000 && !busy;
    $('#slamStart').disabled = !ready || this.status.tracking || !this.config;
    $('#slamStop').disabled = !ready || !this.status.tracking;
    $('#slamReset').disabled = !ready;
    $('#savePointCloud').disabled = !ready || !this.status.canSavePointCloud;
    $('#slamModule').disabled = Boolean(busy);
    const editable = ready && !this.status.tracking && this.config;
    $('#parameterFields').disabled = !editable;
    $('#saveParameters').disabled = $('#loadParameters').disabled = !editable;
  }
  request(cmd, fields = {}, followup = null) {
    if (!this.connected || (this.pending && this.pending.cmd !== 'getStatus')) return;
    const module = $('#slamModule').value.trim();
    if (!module) { $('#commandStatus').textContent = 'Enter a SLAM module name'; return; }
    const requestId = `glim-${++this.serial}`;
    if (!window.wsSendCmd({ ...fields, cmd, module, requestId })) return;
    this.pending = { cmd, module, requestId, followup, since: performance.now() };
    const message = { start: 'Starting SLAM…', stop: 'Finishing and optimizing the map…', reset: 'Stopping and clearing the map…', getConfig: 'Loading SLAM parameters…', setConfig: 'Applying parameters…', saveConfig: 'Saving parameters…', loadConfig: 'Loading saved parameters…', savePointCloud: 'Saving point cloud on the backend…' }[cmd];
    if (message) $('#commandStatus').textContent = message;
    this.buttons();
  }
  receive(reply) {
    if (!this.pending || reply.module !== this.pending.module || reply.requestId !== this.pending.requestId) return;
    const { cmd, followup } = this.pending; this.pending = null; this.lastReply = performance.now();
    if (reply.status) { this.status = reply.status; this.display(reply.status); this.viewer.setSensorPose(reply.status); }
    if (reply.bSuccess === false) $('#commandStatus').textContent = reply.error || 'SLAM command failed';
    else {
      if (reply.config && ['getConfig', 'setConfig', 'saveConfig', 'loadConfig'].includes(cmd)) this.showConfig(reply.config);
      if (cmd === 'saveConfig') $('#parameterStatus').textContent = `Saved on backend: ${reply.configFile || 'configuration JSON'}`;
      else if (cmd === 'loadConfig') $('#parameterStatus').textContent = 'Loaded saved parameters';
      else if (cmd === 'setConfig') $('#parameterStatus').textContent = 'Parameters applied · use Save parameters to keep them';
      if (cmd === 'savePointCloud') $('#exportStatus').textContent = `Saved ${(reply.points || 0).toLocaleString()} points on backend: ${reply.path}`;
      if (cmd !== 'getStatus') $('#commandStatus').textContent = { start: 'SLAM started', stop: 'Stopped · map retained', reset: 'Map and pose cleared', getConfig: 'Parameters loaded', setConfig: 'Parameters applied', saveConfig: 'Parameters saved', loadConfig: 'Saved parameters loaded', savePointCloud: 'Point cloud saved' }[cmd] || 'Command completed';
      if (followup) this.request(followup);
    }
    this.buttons();
  }
  showConfig(config) {
    this.config = config; this.dirty = false;
    const parent = $('#parameterFields'); parent.replaceChildren();
    for (const [group, title, fields] of PARAMETERS) {
      const source = group ? config[group] : config; if (!source) continue;
      const heading = document.createElement('h3'); heading.textContent = title; parent.append(heading);
      for (const [key, text, kind, max, step] of fields) {
        if (source[key] === undefined) continue;
        const label = document.createElement('label'); label.textContent = text;
        const input = document.createElement(Array.isArray(kind) ? 'select' : 'input');
        input.dataset.config = group ? `${group}.${key}` : key;
        input.id = `param-${input.dataset.config.replaceAll('.', '-')}`;
        if (Array.isArray(kind)) for (const value of kind) { const option = document.createElement('option'); option.value = value; option.textContent = value === 'OVERLAP' ? 'Overlap' : 'Displacement'; input.append(option); }
        else if (kind === 'checkbox') input.type = 'checkbox';
        else { input.type = 'number'; input.min = kind; input.max = max; input.step = step; input.required = true; }
        if (kind === 'checkbox') input.checked = source[key]; else input.value = source[key];
        label.append(input); parent.append(label);
      }
    }
    $('#parameterStatus').textContent = this.status?.tracking ? 'Stop SLAM to edit parameters' : 'Parameters ready';
    this.parameterDependencies();
  }
  parameterDependencies() {
    const strategy = $('#param-submap-keyframeStrategy')?.value;
    for (const key of ['keyframeTranslation', 'keyframeRotation', 'maxOverlap']) {
      const input = $(`#param-submap-${key}`); if (input) input.disabled = strategy === 'OVERLAP' ? key !== 'maxOverlap' : key === 'maxOverlap';
    }
  }
  readConfig() {
    if (!$('#parameterForm').reportValidity()) return null;
    const config = JSON.parse(JSON.stringify(this.config));
    for (const input of $('#parameterFields').querySelectorAll('[data-config]')) {
      const path = input.dataset.config.split('.'), key = path.pop();
      const source = path.length ? config[path[0]] : config;
      source[key] = input.type === 'checkbox' ? input.checked : input.type === 'number' ? Number(input.value) : input.value;
    }
    if (config.nMinPoints <= config.preprocess.kNeighbors) { $('#parameterStatus').textContent = 'Minimum points must exceed the neighbor count'; return null; }
    if (config.preprocess.targetPoints && config.preprocess.targetPoints < config.nMinPoints) { $('#parameterStatus').textContent = 'Target points must be zero (automatic) or at least the minimum points'; return null; }
    if (config.preprocess.distanceFar <= config.preprocess.distanceNear) { $('#parameterStatus').textContent = 'Far distance must be greater than near distance'; return null; }
    return config;
  }
  display(status) {
    $('#slamStatus').textContent = { stopped: 'Stopped', tracking: 'Tracking', initializing: 'Initializing', waiting: 'Waiting for pose', error: 'SLAM error' }[status.state] || status.state;
    $('#slamStatus').dataset.state = status.state;
    $('#poseStatus').textContent = status.poseFresh ? 'Pose available · map frame' : status.poseValid ? 'Last pose · no fresh estimate' : 'Waiting for a pose';
    for (const [ids, values, digits] of [[['posX', 'posY', 'posZ'], status.position, 3], [['roll', 'pitch', 'yaw'], status.angles, 1], [['quatX', 'quatY', 'quatZ', 'quatW'], status.orientation, 4]])
      ids.forEach((id, i) => { $(`#${id}`).textContent = status.poseValid ? number(values?.[i], digits) : '—'; });
    for (const id of ['mapPoints', 'submaps', 'liveFrames', 'frames']) $(`#${id}`).textContent = Number.isFinite(status[id]) ? status[id].toLocaleString() : '—';
    $('#frameRate').textContent = status.tracking && status.frameIntervalMs > 0 ? number(1000 / status.frameIntervalMs, 1) : '—';
    $('#processingMs').textContent = status.tracking ? number(status.processingMs, 1) : '—';
    if (this.config && !this.dirty && !this.pending) $('#parameterStatus').textContent = status.tracking ? 'Stop SLAM to edit parameters' : $('#parameterStatus').textContent === 'Stop SLAM to edit parameters' ? 'Parameters ready' : $('#parameterStatus').textContent;
    $('#mapEmpty').firstChild.textContent = status.mapping ? 'Waiting for the first completed submap' : 'Global mapping is off · showing pose and trajectory';
  }
  configureFoV() {
    const fields = ['fovH', 'fovV', 'fovRange'].map(id => $(`#${id}`));
    if (fields.some(field => !field.value || !field.checkValidity())) { $('#commandStatus').textContent = 'Enter a valid FoV and range'; return; }
    this.viewer.configureSensor(...fields.map(field => Number(field.value)), $('#forwardAxis').value);
  }
  tick(now) {
    const showDistance = this.viewer.pointColorUniforms.eyeColorEnabled.value && this.viewer.hasDistanceRange;
    $('#distanceLegend').hidden = !showDistance;
    if (showDistance) { const range = this.viewer.pointColorUniforms.eyeColorRange.value; $('#distanceNear').textContent = `${number(range.x, 2)} m · near`; $('#distanceFar').textContent = `${number(range.y, 2)} m · far`; }
    if (!this.connected) return;
    if (this.pending && now - this.pending.since > (this.pending.cmd === 'getStatus' ? 5000 : 120000)) {
      $('#commandStatus').textContent = `No reply from ${this.pending.module}; check the module name and backend log`; this.pending = null;
    }
    if ((!this.pending || this.pending.cmd === 'getStatus') && now - this.lastReply > 5000) {
      $('#slamStatus').textContent = 'Status unavailable'; $('#slamStatus').dataset.state = 'stale'; $('#poseStatus').textContent = 'Telemetry is stale'; this.viewer.markSensorStale();
    }
    if (!this.pending && now - this.lastReply >= (this.status?.tracking ? 100 : 500)) this.request('getStatus');
    this.buttons();
  }
  dispose() { for (const remove of this.listeners) remove(); }
}
