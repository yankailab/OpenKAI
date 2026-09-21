const $ = selector => document.querySelector(selector);
const labels = { bRGB: 'RGB stream', bDepth: 'Depth stream', bIR: 'IR stream', bPCL: 'Depth point cloud', bPCLrgb: 'RGB point cloud', pclStride: 'Point cloud pixel stride', dScale: 'Point cloud scale', scScanTime: 'Device scan timeout (ms)', scExposureTimeToF: 'ToF exposure (µs)', scExposureTimeRGB: 'Color exposure (µs)', scColorAECMaxExposureTime: 'Color AEC max exposure (µs)' };
function labelFor(key) {
  return labels[key] || key.replace(/^sc/, '').replace(/([a-z0-9])([A-Z])/g, '$1 $2').replace(/([A-Z])([A-Z][a-z])/g, '$1 $2');
}

export class ScepterControls {
  constructor() {
    this.fields = new Map(); this.pending = null; this.sequence = 0; this.loaded = false;
    this.onState = () => {
      const connected = window.wsSocket?.readyState === WebSocket.OPEN;
      if (!connected) { this.clearPending(); this.loaded = false; $('#configStatus').textContent = 'Command connection disconnected'; }
      this.refresh();
      if (connected && !this.loaded && !this.pending) this.send('loadConfig');
    };
    this.onReply = event => this.reply(event.detail);
    window.addEventListener('wscmdstatechange', this.onState);
    window.addEventListener('sceptercommand', this.onReply);
    $('#loadConfig').onclick = () => this.send('loadConfig');
    $('#saveConfig').onclick = () => this.send('saveConfig');
    $('#cameraModule').onchange = () => { this.clearPending(); this.loaded = false; this.fields.clear(); $('#configSections').replaceChildren(); this.onState(); };
    this.refresh();
  }
  refresh() {
    const connected = window.wsSocket?.readyState === WebSocket.OPEN;
    $('#loadConfig').disabled = !connected || !!this.pending;
    $('#saveConfig').disabled = !connected || !this.loaded || !!this.pending;
    $('#cameraFields').disabled = !connected || !this.loaded || !!this.pending;
  }
  clearPending() { clearTimeout(this.timeout); this.pending = null; }
  send(cmd, config) {
    if (this.pending) return;
    const module = $('#cameraModule').value.trim();
    if (!module) { $('#configStatus').textContent = 'Enter a camera module name'; return; }
    const requestId = `camera-${++this.sequence}`;
    if (!window.wsSendCmd({ module, cmd, requestId, ...(config ? { config } : {}) })) return;
    this.pending = requestId;
    $('#configStatus').textContent = cmd === 'setConfig' ? 'Applying camera setting…' : cmd === 'saveConfig' ? 'Saving configuration…' : 'Loading current parameters…';
    this.timeout = setTimeout(() => {
      this.pending = null; this.loaded = false; this.refresh();
      $('#configStatus').textContent = 'No reply. Check the module name, then Load config to refresh.';
    }, 15000);
    this.refresh();
  }
  reply(j) {
    if (j.module !== $('#cameraModule').value.trim() || j.requestId !== this.pending) return;
    this.clearPending();
    if (j.schema) this.build(j.schema);
    if (j.config) {
      for (const [key, { input, spec }] of this.fields) {
        const value = j.config[key];
        input.value = value == null ? '' : typeof value === 'object' ? JSON.stringify(value) : String(value);
        input.title = spec.key;
      }
      this.loaded = true;
    }
    $('#configStatus').textContent = !j.bSuccess ? (j.error || Object.entries(j.errors || {}).map(([key, value]) => `${labelFor(key)}: ${value}`).join(' · ') || 'Command failed') :
      j.cmd === 'saveConfig' ? 'Configuration saved to disk' : j.cmd === 'setConfig' ? 'Applied · not saved to disk' : j.deviceOpen ? 'Current parameters loaded' : 'Parameters loaded · camera is not open';
    this.refresh();
  }
  build(schema) {
    const container = $('#configSections'); container.replaceChildren(); this.fields.clear();
    const sections = new Map();
    for (const spec of schema) {
      let section = sections.get(spec.category);
      if (!section) {
        section = document.createElement('details');
        section.open = spec.category === 'Streams and point cloud' || spec.category === 'Color';
        const summary = document.createElement('summary'); summary.textContent = spec.category;
        section.append(summary); container.append(section); sections.set(spec.category, section);
      }
      const row = document.createElement('div'); row.className = 'control';
      const label = document.createElement('label'); label.textContent = labelFor(spec.key); label.title = spec.key;
      const input = document.createElement(spec.type === 'bool' ? 'select' : spec.type === 'pairs' ? 'textarea' : 'input');
      input.id = `param-${spec.key}`; label.htmlFor = input.id;
      input.dataset.key = spec.key;
      input.disabled = spec.supported === false;
      if (spec.type === 'bool') {
        for (const [value, text] of [['true', 'On'], ['false', 'Off']]) {
          const option = document.createElement('option'); option.value = value; option.textContent = text; input.append(option);
        }
      } else if (spec.type === 'int' || spec.type === 'float') {
        input.type = 'number'; input.step = spec.type === 'int' ? '1' : 'any';
        if (spec.step > 0) input.step = spec.step;
        if (spec.min != null) input.min = spec.min;
        if (spec.max != null) input.max = spec.max;
        input.placeholder = '';
      } else {
        input.placeholder = spec.example ? JSON.stringify(spec.example) : '';
        if (spec.type === 'pairs') input.rows = 3;
        if (spec.maxLength != null) input.maxLength = spec.maxLength;
      }
      input.onchange = () => {
        try {
          if (!input.reportValidity()) return;
          const text = input.value.trim();
          let value;
          if (spec.type === 'bool') value = text === 'true';
          else if (spec.type === 'pairs') value = JSON.parse(text);
          else if (spec.type === 'int' || spec.type === 'float') {
            value = Number(text);
            if (!text || !Number.isFinite(value) || (spec.type === 'int' && !Number.isInteger(value))) throw Error('Enter a valid number');
          } else value = text;
          this.send('setConfig', { [spec.key]: value });
        } catch (error) { $('#configStatus').textContent = `${labelFor(spec.key)}: ${error.message}`; }
      };
      row.append(label, input);
      if (spec.supported === false || spec.current != null) {
        const hint = document.createElement('small');
        hint.textContent = spec.supported === false ? 'Not writable on this camera.' : `Device at last load: ${spec.current}`;
        row.append(hint);
      }
      if (spec.restart || spec.hint) {
        const hint = document.createElement('small'); hint.textContent = [spec.restart ? 'Changing this restarts camera capture.' : '', spec.hint || ''].filter(Boolean).join(' ');
        row.append(hint);
      }
      section.append(row); this.fields.set(spec.key, { input, spec });
    }
  }
  dispose() {
    this.clearPending();
    window.removeEventListener('wscmdstatechange', this.onState);
    window.removeEventListener('sceptercommand', this.onReply);
  }
}
