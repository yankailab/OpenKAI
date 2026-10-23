const $ = selector => document.querySelector(selector);
const labels = {
  lvxSN: 'Selected serial number', lvxIP: 'Sensor IP', lvxHostIP: 'Host IP override (0.0.0.0 = automatic)',
  lvxPortQuery: 'Sensor discovery UDP port', lvxHostPortCmd: 'Host command UDP port',
  receiveBufferBytes: 'UDP receive buffer (bytes)',
  lvxHostPortState: 'Status UDP port', lvxHostPortPCL: 'Point cloud UDP port', lvxHostPortIMU: 'IMU UDP port',
  lvxPCLdataType: 'Point coordinates', lvxPatternMode: 'Scan pattern', lvxDetectMode: 'Detection mode',
  lvxWorkMode: 'Work mode', lvxIMUdataEn: 'Embedded IMU', frameDurationMs: 'Point cloud frame duration (ms)',
  bIMUstab: 'Stabilize point cloud with IMU', bReflectivity: 'Color by reflectivity',
  vColorDefault: 'Point color [R, G, B]', vRangeD: 'Distance range [minimum, maximum] (m)',
  nMaxFramePoints: 'Maximum points per frame', tOutSec: 'Device timeout (s)'
};
const formatValue = value => value == null ? '' : typeof value === 'object' ? JSON.stringify(value) : String(value);
const errorText = errors => Object.entries(errors || {}).map(([key, value]) => `${labels[key] || key}: ${formatValue(value)}`).join(' · ');

export class LivoxControls {
  constructor() {
    this.fields = new Map(); this.pending = null; this.sequence = 0; this.loaded = false;
    this.applying = false; this.config = {}; this.schemaKey = '';
    this.onState = () => {
      const connected = window.wsSocket?.readyState === WebSocket.OPEN;
      if (!connected) {
        this.clearPending(); clearTimeout(this.pollTimer); this.loaded = this.applying = false;
        $('#deviceStatus').replaceChildren();
        $('#configStatus').textContent = 'Command connection disconnected';
      }
      this.refresh();
      if (connected && !this.loaded && !this.pending) this.send('getConfig');
    };
    this.onReply = event => this.reply(event.detail);
    this.onModule = () => {
      this.clearPending(); clearTimeout(this.pollTimer); this.loaded = this.applying = false;
      this.fields.clear(); this.config = {}; this.schemaKey = ''; $('#configSections').replaceChildren();
      $('#deviceStatus').replaceChildren(); this.onState();
    };
    window.addEventListener('wscmdstatechange', this.onState);
    window.addEventListener('livoxcommand', this.onReply);
    $('#getConfig').onclick = () => this.send('getConfig');
    $('#saveConfig').onclick = () => this.send('saveConfig');
    $('#lidarModule').addEventListener('change', this.onModule);
    this.onState();
  }

  refresh() {
    const connected = window.wsSocket?.readyState === WebSocket.OPEN;
    $('#getConfig').disabled = !connected || !!this.pending;
    $('#saveConfig').disabled = !connected || !this.loaded || !!this.pending || this.applying;
    $('#lidarFields').disabled = !connected || !this.loaded || !!this.pending || this.applying;
  }

  clearPending() { clearTimeout(this.timeout); this.pending = null; }

  send(cmd, config, background = false) {
    if (this.pending) return;
    const module = $('#lidarModule').value.trim();
    if (!module) { $('#configStatus').textContent = 'Enter a LiDAR module name'; return; }
    const requestId = `lidar-${++this.sequence}`;
    if (!window.wsSendCmd({ module, cmd, requestId, ...(config ? { config } : {}) })) {
      $('#configStatus').textContent = 'Command was not sent. Check the connection and command log.';
      return;
    }
    clearTimeout(this.pollTimer);
    this.pending = { module, cmd, requestId };
    if (!background) $('#configStatus').textContent = cmd === 'setConfig' ? 'Applying LiDAR setting…' :
      cmd === 'saveConfig' ? 'Saving configuration…' : 'Loading current parameters…';
    this.timeout = setTimeout(() => {
      this.pending = null; this.loaded = false; this.refresh();
      $('#configStatus').textContent = 'No reply. Check the module name, then refresh config.';
    }, 15000);
    this.refresh();
  }

  reply(reply) {
    if (!this.pending || ['module', 'cmd', 'requestId'].some(key => reply[key] !== this.pending[key])) return;
    this.clearPending();
    if (reply.schema && typeof reply.schema === 'object') {
      const schemaKey = JSON.stringify(reply.schema);
      if (schemaKey !== this.schemaKey) {
        const schema = Array.isArray(reply.schema) ? reply.schema :
          Object.entries(reply.schema).map(([key, spec]) => ({ ...spec, key }));
        this.build(schema); this.schemaKey = schemaKey;
      }
    }
    if (reply.config && typeof reply.config === 'object') { this.config = reply.config; this.loaded = true; }
    for (const { input, spec, readback } of this.fields.values()) {
      const value = spec.readOnly && spec.current != null ? spec.current : this.config[spec.key];
      const text = formatValue(value);
      if (input.tagName === 'SELECT') {
        input.querySelectorAll('[data-current]').forEach(option => option.remove());
        if (text && !Array.from(input.options).some(option => option.value === text)) {
          const option = document.createElement('option');
          option.value = text; option.textContent = `Current value: ${text}`; option.dataset.current = 'true';
          input.append(option);
        }
      }
      input.value = text;
      const observed = reply.status?.observed?.[spec.key];
      readback.hidden = observed == null;
      readback.textContent = observed == null ? '' : `Last sensor readback: ${
        spec.choices?.find(choice => choice.value === observed)?.label || formatValue(observed)}`;
    }
    this.applying = Number(reply.status?.pendingCommands) > 0;
    this.showStatus(reply.status);
    $('#configStatus').textContent = !reply.bSuccess ?
      (reply.error || errorText(reply.errors) || 'Command failed') :
      reply.status?.lastError ? `Device: ${reply.status.lastError}` :
      this.applying ? 'Settings queued · waiting for sensor acknowledgement…' :
      reply.cmd === 'saveConfig' ? 'Configuration saved to disk' :
      reply.cmd === 'setConfig' ? 'Applied · not saved to disk' :
      reply.status?.configApplied === false ? 'Parameters loaded · sensor settings are not yet applied' :
      reply.deviceOpen ? 'Current parameters loaded' : 'Parameters loaded · LiDAR is not connected';
    this.refresh();
    this.schedulePoll();
  }

  schedulePoll() {
    clearTimeout(this.pollTimer);
    this.pollTimer = setTimeout(() => {
      if (window.wsSocket?.readyState !== WebSocket.OPEN || !this.loaded) return;
      // Avoid replacing an unfinished text/number edit during periodic status reads.
      if (this.pending || (!this.applying && $('#lidarFields').contains(document.activeElement))) {
        this.schedulePoll(); return;
      }
      this.send('getConfig', undefined, true);
    }, this.applying ? 500 : 2000);
  }

  showStatus(status) {
    const container = $('#deviceStatus'); container.replaceChildren();
    if (!status || typeof status !== 'object') return;
    const labels = {
      state: 'State', serialNumber: 'Serial number', deviceType: 'Device type', commandPort: 'Sensor command UDP port',
      hostIP: 'Active host Ethernet IP', socketsOpen: 'UDP sockets open',
      configApplied: 'Settings applied', pendingCommands: 'Pending commands', pointPackets: 'Point packets',
      points: 'Points received', frames: 'Frames published', imuSamples: 'IMU samples',
      invalidPackets: 'Invalid packets', outOfOrderPackets: 'Out-of-order packets', frameOverflows: 'Frame overflows',
      commandErrors: 'Command errors', commandRetries: 'Command retries', lastError: 'Last device error'
    };
    for (const [key, label] of Object.entries(labels)) {
      if (status[key] == null || status[key] === '') continue;
      const term = document.createElement('dt'); term.textContent = label;
      const value = document.createElement('dd'); value.textContent = formatValue(status[key]);
      container.append(term, value);
    }
  }

  build(schema) {
    const container = $('#configSections'); container.replaceChildren(); this.fields.clear();
    const sections = new Map();
    for (const source of schema) {
      const spec = { ...source, type: ({integer:'int',number:'float',boolean:'bool'})[source.type] || source.type };
      if (!spec.key || this.fields.has(spec.key)) continue;
      const category = spec.category || 'LiDAR parameters';
      let section = sections.get(category);
      if (!section) {
        section = document.createElement('details'); section.open = !spec.readOnly;
        const summary = document.createElement('summary'); summary.textContent = category;
        section.append(summary); container.append(section); sections.set(category, section);
      }
      if (!spec.readOnly && spec.supported !== false) section.open = true;
      const row = document.createElement('div'); row.className = 'control';
      const label = document.createElement('label'); label.textContent = spec.label || labels[spec.key] || spec.key;
      label.title = spec.key;
      const choices = spec.choices?.length ? spec.choices : null;
      const input = document.createElement(choices || spec.type === 'bool' ? 'select' : 'input');
      input.id = `param-${spec.key}`; input.dataset.key = spec.key; input.title = spec.description || spec.key;
      input.disabled = spec.supported === false || !!spec.readOnly; label.htmlFor = input.id;
      if (input.tagName === 'SELECT') {
        for (const choice of choices || [{ value: true, label: 'On' }, { value: false, label: 'Off' }]) {
          const option = document.createElement('option'); option.value = String(choice.value);
          option.textContent = choice.label; input.append(option);
        }
      } else if (spec.type === 'int' || spec.type === 'float') {
        input.type = 'number'; input.step = spec.step > 0 ? spec.step : spec.type === 'int' ? '1' : 'any';
        if (spec.min != null) input.min = spec.min;
        if (spec.max != null) input.max = spec.max;
      } else {
        input.placeholder = spec.type === 'range' ? '[minimum, maximum]' :
          ['array', 'vec3'].includes(spec.type) ? '[values]' : '';
        if (spec.maxLength != null) input.maxLength = spec.maxLength;
      }
      input.onchange = () => {
        if (input.disabled || this.pending || this.applying || !this.loaded) return;
        try {
          if (!input.reportValidity()) return;
          const text = input.value.trim();
          let value;
          if (spec.type === 'bool') value = text === 'true';
          else if (['array', 'range', 'vec3'].includes(spec.type)) {
            value = JSON.parse(text);
            if (!Array.isArray(value) || !value.every(Number.isFinite)) throw Error('Enter an array of numbers');
            const length = spec.length || (spec.type === 'range' ? 2 : spec.type === 'vec3' ? 3 : null);
            if (length && value.length !== length) throw Error(`Enter exactly ${length} numbers`);
            if (value.some(v => (spec.min != null && v < spec.min) || (spec.max != null && v > spec.max)))
              throw Error(`Values must be between ${spec.min ?? '−∞'} and ${spec.max ?? '∞'}`);
            if (spec.type === 'range' && value[0] > value[1]) throw Error('Minimum must not exceed maximum');
          } else if (spec.type === 'int' || spec.type === 'float') {
            value = Number(text);
            if (!text || !Number.isFinite(value) || (spec.type === 'int' && !Number.isInteger(value))) throw Error('Enter a valid number');
          } else value = text;
          this.send('setConfig', { [spec.key]: value });
        } catch (error) { $('#configStatus').textContent = `${label.textContent}: ${error.message}`; }
      };
      row.append(label, input);
      const hint = text => { const item = document.createElement('small'); item.textContent = text; row.append(item); };
      if (spec.description) hint(spec.description);
      if (spec.supported === false) hint('Not available on this LiDAR.');
      else if (spec.readOnly) hint('Read-only.');
      if (spec.restart && !spec.readOnly && spec.supported !== false) hint('Changing this restarts capture.');
      const readback = document.createElement('small'); readback.hidden = true; row.append(readback);
      section.append(row); this.fields.set(spec.key, { input, spec, readback });
    }
  }

  dispose() {
    this.clearPending(); clearTimeout(this.pollTimer);
    window.removeEventListener('wscmdstatechange', this.onState);
    window.removeEventListener('livoxcommand', this.onReply);
    $('#lidarModule').removeEventListener('change', this.onModule);
  }
}
