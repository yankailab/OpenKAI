const $ = selector => document.querySelector(selector);
const labels = {
  SN: 'Serial number', tOutMs: 'Frame timeout (ms)', bRGB: 'RGB stream', bDepth: 'Depth stream',
  bIR: 'Infrared stream', bIMU: 'Embedded IMU', bPCL: 'Depth point cloud', bPCLrgb: 'RGB point cloud',
  bAlign: 'Align depth to color', devFPS: 'RGB frame rate', devFPSd: 'Depth frame rate',
  accelFPS: 'Accelerometer rate (Hz)', gyroFPS: 'Gyroscope rate (Hz)',
  vSizeRGB: 'RGB size [width, height]', vSizeD: 'Depth size [width, height]',
  vRangeD: 'Depth range [minimum, maximum] (m)', dOfs: 'Depth offset (m)',
  bDecimation: 'Decimation filter', bSpatial: 'Spatial filter', bTemporal: 'Temporal filter',
  bHoleFilling: 'Hole filling filter', bThreshold: 'Threshold filter'
};
const labelFor = key => labels[key] || key.replace(/^RS2_OPTION_/, '').replaceAll('_', ' ').toLowerCase().replace(/^./, c => c.toUpperCase());
const sectionLabels = { depth: 'Depth sensor', color: 'Color sensor', motion: 'Motion sensor', decimation: 'Decimation filter', spatial: 'Spatial filter', temporal: 'Temporal filter', holeFilling: 'Hole filling filter', threshold: 'Threshold filter' };
const fieldKey = spec => spec.domain ? `sensorOptions.${spec.domain}.${spec.key}` : spec.key;
const formatValue = value => value == null ? '' : typeof value === 'object' ? JSON.stringify(value) : String(value);
// SDK float ranges arrive as expanded float32 numbers (0.10000000149011612).
// Native HTML step validation uses tighter tolerance than the SDK and rejects
// ordinary values such as 4 with that step. Keep useful float32 precision in
// HTML metadata and displayed overrides; the backend validates exact SDK limits.
const sdkNumber = value => Number(Number(value).toPrecision(7));

function errorText(errors, prefix = '') {
  return Object.entries(errors || {}).flatMap(([key, value]) => {
    const path = prefix ? `${prefix}.${key}` : key;
    return value && typeof value === 'object' ? errorText(value, path) : [`${path}: ${value}`];
  }).join(' · ');
}

export class RealSenseControls {
  constructor() {
    this.fields = new Map(); this.pending = null; this.sequence = 0; this.loaded = false;
    this.onState = () => {
      const connected = window.wsSocket?.readyState === WebSocket.OPEN;
      if (!connected) {
        this.clearPending(); this.loaded = false;
        $('#configStatus').textContent = 'Command connection disconnected';
      }
      this.refresh();
      if (connected && !this.loaded && !this.pending) this.send('getConfig');
    };
    this.onReply = event => this.reply(event.detail);
    window.addEventListener('wscmdstatechange', this.onState);
    window.addEventListener('realsensecommand', this.onReply);
    $('#getConfig').onclick = () => this.send('getConfig');
    $('#saveConfig').onclick = () => this.send('saveConfig');
    $('#cameraModule').onchange = () => {
      this.clearPending(); this.loaded = false; this.fields.clear();
      $('#configSections').replaceChildren(); this.onState();
    };
    this.onState();
  }

  refresh() {
    const connected = window.wsSocket?.readyState === WebSocket.OPEN;
    $('#getConfig').disabled = !connected || !!this.pending;
    $('#saveConfig').disabled = !connected || !this.loaded || !!this.pending;
    $('#cameraFields').disabled = !connected || !this.loaded || !!this.pending;
  }

  clearPending() { clearTimeout(this.timeout); this.pending = null; }

  send(cmd, config) {
    if (this.pending) return;
    const module = $('#cameraModule').value.trim();
    if (!module) { $('#configStatus').textContent = 'Enter a camera module name'; return; }
    const requestId = `camera-${++this.sequence}`;
    if (!window.wsSendCmd({ module, cmd, requestId, ...(config ? { config } : {}) })) {
      $('#configStatus').textContent = 'Command was not sent. Check the connection and command log.';
      return;
    }
    this.pending = requestId;
    $('#configStatus').textContent = cmd === 'setConfig' ? 'Applying camera setting…' :
      cmd === 'saveConfig' ? 'Saving configuration…' : 'Loading current parameters…';
    this.timeout = setTimeout(() => {
      this.pending = null; this.loaded = false; this.refresh();
      $('#configStatus').textContent = 'No reply. Check the module name, then refresh config.';
    }, 15000);
    this.refresh();
  }

  reply(reply) {
    if (!this.pending || reply.module !== $('#cameraModule').value.trim() || reply.requestId !== this.pending) return;
    this.clearPending();
    if (reply.schema) this.build(reply.schema);
    if (reply.config) {
      for (const { input, spec } of this.fields.values()) {
        const value = spec.readOnly ? spec.current : spec.domain ?
          reply.config.sensorOptions?.[spec.domain]?.[spec.key] : reply.config[spec.key];
        // Preserve a saved enum value even if newer firmware changes its choices.
        if (input.tagName === 'SELECT') {
          input.querySelectorAll('[data-current]').forEach(option => option.remove());
          const text = formatValue(value);
          if (text && !Array.from(input.options).some(option => option.value === text)) {
            const option = document.createElement('option');
            option.value = text; option.textContent = `Current value: ${text}`; option.dataset.current = 'true';
            input.append(option);
          }
        }
        input.value = formatValue(spec.domain && spec.type === 'float' && input.type === 'number' && typeof value === 'number' && !spec.readOnly ? sdkNumber(value) : value);
      }
      this.loaded = true;
    }
    $('#configStatus').textContent = !reply.bSuccess ?
      (reply.error || errorText(reply.errors) || 'Command failed') :
      reply.cmd === 'saveConfig' ? 'Configuration saved to disk' :
      reply.cmd === 'setConfig' ? 'Applied · not saved to disk' :
      reply.deviceOpen ? 'Current parameters loaded' : 'Parameters loaded · camera is not open';
    this.refresh();
  }

  build(schema) {
    const container = $('#configSections'); container.replaceChildren(); this.fields.clear();
    const sections = new Map();
    for (const spec of schema) {
      const key = fieldKey(spec);
      if (this.fields.has(key)) continue;
      const category = spec.category || 'Camera';
      let section = sections.get(category);
      if (!section) {
        section = document.createElement('details');
        section.open = !spec.domain;
        const summary = document.createElement('summary'); summary.textContent = sectionLabels[category] || category;
        section.append(summary); container.append(section); sections.set(category, section);
      }
      const row = document.createElement('div'); row.className = 'control';
      const label = document.createElement('label');
      label.textContent = spec.label || labelFor(spec.key); label.title = key;
      const choices = spec.choices?.length ? spec.choices : null;
      const input = document.createElement(choices || spec.type === 'bool' ? 'select' : ['rect', 'object'].includes(spec.type) ? 'textarea' : 'input');
      input.id = `param-${key}`; input.dataset.key = key; input.title = spec.description || key;
      input.disabled = spec.supported === false || !!spec.readOnly;
      label.htmlFor = input.id;
      if (input.tagName === 'SELECT') {
        const values = choices || [{ value: true, label: 'On' }, { value: false, label: 'Off' }];
        for (const choice of [...(spec.nullable ? [{ value: '', label: 'Preserve camera setting' }] : []), ...values]) {
          const option = document.createElement('option');
          option.value = String(choice.value); option.textContent = choice.label;
          input.append(option);
        }
      } else if (spec.type === 'int' || spec.type === 'float') {
        const number = value => spec.domain && spec.type === 'float' ? sdkNumber(value) : value;
        input.type = 'number'; input.step = spec.step > 0 ? number(spec.step) : spec.type === 'int' ? '1' : 'any';
        if (spec.min != null) input.min = number(spec.min);
        if (spec.max != null) input.max = number(spec.max);
        input.placeholder = spec.nullable ? 'Preserve camera setting' : '';
      } else {
        input.placeholder = spec.type === 'size' ? '[width, height]' :
          spec.type === 'range' ? '[minimum, maximum]' :
          spec.type === 'array' ? '[values]' : spec.type === 'rect' ? '[x1, y1, x2, y2]' :
          spec.type === 'object' ? '{}' : spec.key === 'SN' ? 'Any connected camera' :
          spec.nullable ? 'Preserve camera setting' : '';
        if (input.tagName === 'TEXTAREA') input.rows = 3;
      }
      input.onchange = () => {
        if (input.disabled || this.pending) return;
        try {
          if (!input.reportValidity()) return;
          const text = input.value.trim();
          let value;
          if (!text && spec.nullable) value = null;
          else if (spec.type === 'bool') value = text === 'true';
          else if (spec.type === 'object') {
            value = JSON.parse(text);
            if (!value || typeof value !== 'object' || Array.isArray(value)) throw Error('Enter a JSON object');
          } else if (spec.type === 'rect') {
            value = JSON.parse(text);
            if (!Array.isArray(value) || value.length !== 4 || !value.every(n => Number.isInteger(n) && n >= 0 && n <= 32767))
              throw Error('Enter [x1, y1, x2, y2] with integer coordinates from 0 to 32767');
            if (value[0] > value[2] || value[1] > value[3]) throw Error('Minimum coordinates must not exceed maximum coordinates');
          } else if (['size', 'range', 'array'].includes(spec.type)) {
            value = JSON.parse(text);
            if (!Array.isArray(value) || !value.every(Number.isFinite)) throw Error('Enter an array of numbers');
            if ((spec.type === 'size' || spec.type === 'range') && value.length !== 2) throw Error('Enter exactly two numbers');
            if (spec.type === 'size' && !value.every(n => Number.isInteger(n) && n > 0)) throw Error('Dimensions must be positive integers');
          } else if (spec.type === 'int' || spec.type === 'float') {
            value = Number(text);
            if (!text || !Number.isFinite(value) || (spec.type === 'int' && !Number.isInteger(value))) throw Error('Enter a valid number');
          } else value = text;
          this.send('setConfig', spec.domain ? { sensorOptions: { [spec.domain]: { [spec.key]: value } } } : { [spec.key]: value });
        } catch (error) { $('#configStatus').textContent = `${label.textContent}: ${error.message}`; }
      };
      row.append(label, input);
      const hint = text => { const item = document.createElement('small'); item.textContent = text; row.append(item); };
      if (spec.description) hint(spec.description);
      if (spec.supported === false) hint('Not available on this camera.');
      else if (spec.readOnly) hint('Read-only device value.');
      else if (spec.current != null) hint(`Device at last load: ${formatValue(spec.current)}`);
      if (spec.restart && !spec.readOnly && spec.supported !== false) hint('Changing this restarts camera capture.');
      section.append(row); this.fields.set(key, { input, spec });
    }
  }

  dispose() {
    this.clearPending();
    window.removeEventListener('wscmdstatechange', this.onState);
    window.removeEventListener('realsensecommand', this.onReply);
  }
}
