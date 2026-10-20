const $ = selector => document.querySelector(selector);
const labelFor = key => key.replace(/([a-z0-9])([A-Z])/g, '$1 $2').replaceAll('_', ' ').replace(/^./, c => c.toUpperCase());
const formatValue = value => value == null ? '' : typeof value === 'object' ? JSON.stringify(value) : String(value);
const areaKeys = ['areaXRangeMm', 'areaZRangeMm', 'areaXStartMm', 'areaZStartMm'];
const defaultActions = [
  { cmd: 'startAcquisition', label: 'Start acquisition' },
  { cmd: 'stopAcquisition', label: 'Stop acquisition' },
  { cmd: 'softwareTrigger', label: 'Software trigger', hint: 'Requires software trigger mode and running acquisition.' },
  { cmd: 'resetSensorArea', label: 'Reset sensor area' },
  { cmd: 'setMaximumFrameRate', label: 'Use maximum frame rate', hint: 'Sets the rate to the current sensor maximum. This does not track future limits.' },
  { cmd: 'readProject', label: 'Read project' }
];

function errorText(errors, prefix = '') {
  return Object.entries(errors || {}).map(([key, value]) => {
    const path = prefix ? `${prefix}.${key}` : key;
    return value && typeof value === 'object' ? errorText(value, path) : `${path}: ${value}`;
  }).join(' · ');
}

export class SporSensorControls {
  constructor() {
    this.fields = new Map(); this.pending = null; this.sequence = 0; this.loaded = false;
    this.areaDirty = false; this.pendingArea = null;
    this.onState = () => {
      const connected = window.wsSocket?.readyState === WebSocket.OPEN;
      if (!connected) {
        this.clearPending(); this.loaded = false;
        $('#configStatus').textContent = 'Command connection disconnected';
        $('#sensorState').textContent = 'Command connection disconnected. Reconnect to refresh sensor status.';
      }
      this.refresh();
      if (connected && !this.loaded && !this.pending) this.send('getConfig');
    };
    this.onReply = event => this.reply(event.detail);
    window.addEventListener('wscmdstatechange', this.onState);
    window.addEventListener('sporsensorcommand', this.onReply);
    $('#getConfig').onclick = () => this.send('getConfig');
    $('#saveConfig').onclick = () => this.send('saveConfig');
    $('#readModel').onclick = () => {
      const guid = $('#modelGuid').value.trim();
      const deviceId = $('#modelDeviceId').value.trim();
      if (!guid) { $('#configStatus').textContent = 'Enter a model GUID to inspect'; return; }
      this.send('readModel', null, { guid, ...(deviceId ? { deviceId } : {}) });
    };
    $('#cameraModule').onchange = () => {
      this.clearPending(); this.loaded = false; this.fields.clear();
      this.areaDirty = false; this.pendingArea = null;
      $('#configSections').replaceChildren(); this.buildActions(defaultActions);
      $('#inspectionPanel').hidden = true; this.onState();
    };
    this.buildActions(defaultActions);
    this.onState();
  }

  refresh() {
    const connected = window.wsSocket?.readyState === WebSocket.OPEN;
    $('#getConfig').disabled = !connected || !!this.pending;
    $('#saveConfig').disabled = !connected || !this.loaded || !!this.pending || this.areaDirty;
    $('#cameraFields').disabled = !connected || !this.loaded || !!this.pending;
    $('#sensorActions').disabled = !connected || !this.loaded || !!this.pending;
  }

  clearPending() { clearTimeout(this.timeout); this.pending = null; }

  send(cmd, config, parameters = {}) {
    if (this.pending) return;
    const module = $('#cameraModule').value.trim();
    if (!module) { $('#configStatus').textContent = 'Enter a scanner module name'; return; }
    const requestId = `spor-${++this.sequence}`;
    if (!window.wsSendCmd({ module, cmd, requestId, ...parameters, ...(config ? { config } : {}) })) {
      $('#configStatus').textContent = 'Command was not sent. Check the connection and command log.';
      return;
    }
    this.pending = requestId;
    $('#configStatus').textContent = cmd === 'setConfig' ? 'Applying scanner setting…' :
      cmd === 'saveConfig' ? 'Saving configuration…' : cmd === 'getConfig' ? 'Loading current parameters…' :
      `${labelFor(cmd)}…`;
    // Settings calls discover endpoints, write a complete model and verify a fresh
    // snapshot. Allow them more time than one native control acknowledgement.
    this.timeout = setTimeout(() => {
      this.pending = null; this.loaded = false; this.refresh();
      $('#configStatus').textContent = 'No reply. Check the module name and sensor connection, then refresh config.';
    }, 120000);
    this.refresh();
    return requestId;
  }

  reply(reply) {
    if (!this.pending || reply.module !== $('#cameraModule').value.trim() || reply.requestId !== this.pending) return;
    const areaReply = reply.requestId === this.pendingArea;
    const areaDraft = this.areaDirty && !areaReply ? Object.fromEntries(areaKeys.map(key => [key, this.fields.get(key)?.input.value])) : null;
    if (areaReply) { this.areaDirty = false; this.pendingArea = null; }
    this.clearPending();
    if (Array.isArray(reply.schema)) this.build(reply.schema);
    if (Array.isArray(reply.actions)) this.buildActions(reply.actions);
    if (reply.config) {
      for (const { input, spec } of this.fields.values()) {
        const value = spec.readOnly || spec.readonly ? spec.current ?? reply.config[spec.key] : reply.config[spec.key];
        if (input.tagName === 'SELECT') {
          input.querySelectorAll('[data-current]').forEach(option => option.remove());
          const text = formatValue(value);
          if (text && !Array.from(input.options).some(option => option.value === text)) {
            const option = document.createElement('option');
            option.value = text; option.textContent = `Current value: ${text}`; option.dataset.current = 'true';
            input.append(option);
          }
        }
        input.value = areaDraft && areaKeys.includes(spec.key) ? areaDraft[spec.key] : formatValue(value);
      }
      this.loaded = true;
    }
    if (reply.status || reply.device || reply.deviceOpen != null) {
      const status = typeof reply.status === 'string' ? reply.status : { deviceOpen: reply.deviceOpen, ...reply.status };
      $('#sensorState').textContent = typeof status === 'string' ? status : JSON.stringify(status, null, 2);
    }
    const snapshot = reply.result ?? reply.project ?? reply.model ?? reply.models ?? reply.snapshot ?? reply.device;
    if (snapshot != null) {
      $('#inspectionPanel').hidden = false;
      if (reply.cmd === 'readProject' || reply.cmd === 'readModel') $('#inspectionPanel').open = true;
      $('#inspectionState').textContent = JSON.stringify(snapshot, null, 2);
    }
    $('#configStatus').textContent = !reply.bSuccess ?
      (reply.error || errorText(reply.errors) || 'Command failed') :
      reply.cmd === 'saveConfig' ? 'Configuration saved to disk' :
      reply.cmd === 'setConfig' ? reply.pending ? 'Stored · waiting for sensor connection · not saved to disk' : 'Applied · not saved to disk' :
      reply.cmd === 'getConfig' ? reply.deviceOpen ? 'Current parameters loaded' : 'Parameters loaded · scanner is not open' :
      `${labelFor(reply.cmd)} completed`;
    if (this.areaDirty) $('#configStatus').textContent += ' · Area edits pending; click Apply area or Preserve area.';
    this.refresh();
  }

  applyArea(preserve) {
    if (this.pending) return;
    try {
      const patch = {};
      for (const key of areaKeys) {
        const field = this.fields.get(key);
        if (!field || field.input.disabled) throw Error('Sensor area controls are unavailable');
        if (preserve) { patch[key] = null; continue; }
        const text = field.input.value.trim();
        const value = Number(text);
        if (!text || !Number.isFinite(value)) throw Error('Enter all four area values before applying');
        if (!field.input.reportValidity()) return;
        patch[key] = value;
      }
      this.pendingArea = this.send('setConfig', patch) || null;
    } catch (error) { $('#configStatus').textContent = error.message; }
  }

  buildActions(actions) {
    const container = $('#actionButtons'); container.replaceChildren();
    for (const spec of actions) {
      const command = typeof spec === 'string' ? spec : spec.cmd;
      if (!command) continue;
      const button = document.createElement('button'); button.type = 'button';
      button.id = `action-${command}`;
      button.textContent = spec.label || labelFor(command);
      button.title = spec.hint || '';
      button.disabled = spec.supported === false;
      button.onclick = () => this.send(command);
      container.append(button);
    }
  }

  build(schema) {
    const container = $('#configSections');
    const openSections = new Set(Array.from(container.children).filter(section => section.open).map(section => section.dataset.category));
    const firstBuild = this.fields.size === 0;
    container.replaceChildren(); this.fields.clear();
    const sections = new Map();
    for (const spec of schema) {
      if (!spec.key || this.fields.has(spec.key)) continue;
      const category = spec.category || 'Scanner';
      let section = sections.get(category);
      if (!section) {
        section = document.createElement('details'); section.dataset.category = category;
        section.open = firstBuild ? sections.size < 2 : openSections.has(category);
        const summary = document.createElement('summary'); summary.textContent = category;
        section.append(summary); container.append(section); sections.set(category, section);
      }
      const row = document.createElement('div'); row.className = 'control';
      const label = document.createElement('label');
      label.textContent = spec.label || labelFor(spec.key); label.title = spec.key;
      const choices = spec.choices?.length ? spec.choices : spec.options?.length ? spec.options : null;
      const structured = ['array', 'object', 'json', 'pairs'].includes(spec.type);
      const input = document.createElement(choices || spec.type === 'bool' ? 'select' : structured ? 'textarea' : 'input');
      input.id = `param-${spec.key}`; input.dataset.key = spec.key; input.title = spec.description || spec.key;
      input.disabled = spec.supported === false || !!spec.readOnly || !!spec.readonly;
      label.htmlFor = input.id;
      if (input.tagName === 'SELECT') {
        const values = choices || [{ value: true, label: 'On' }, { value: false, label: 'Off' }];
        for (const choice of [...(spec.nullable ? [{ value: '', label: 'Preserve sensor setting' }] : []), ...values]) {
          const option = document.createElement('option');
          option.value = String(choice.value); option.textContent = choice.label;
          input.append(option);
        }
      } else if (spec.type === 'int' || spec.type === 'float') {
        input.type = 'number'; input.step = spec.step > 0 ? spec.step : spec.type === 'int' ? '1' : 'any';
        if (spec.min != null) input.min = spec.min;
        if (spec.max != null) input.max = spec.max;
        input.placeholder = spec.nullable ? 'Preserve sensor setting' : '';
      } else {
        input.placeholder = spec.example != null ? JSON.stringify(spec.example) :
          spec.nullable ? 'Preserve sensor setting' : structured ? spec.type === 'object' ? '{}' : '[values]' : '';
        if (input.tagName === 'TEXTAREA') input.rows = 3;
        if (spec.maxLength != null) input.maxLength = spec.maxLength;
      }
      input.onchange = () => {
        if (input.disabled || this.pending) return;
        if (areaKeys.includes(spec.key)) {
          this.areaDirty = true;
          $('#configStatus').textContent = 'Area edits pending; click Apply area to send all four values together.';
          this.refresh();
          return;
        }
        try {
          if (!input.reportValidity()) return;
          const text = input.value.trim();
          let value;
          if (!text && spec.nullable) value = null;
          else if (spec.type === 'bool') {
            if (text !== 'true' && text !== 'false') throw Error('Select On or Off');
            value = text === 'true';
          } else if (structured) {
            value = JSON.parse(text);
            if (spec.type === 'object' && (!value || typeof value !== 'object' || Array.isArray(value)))
              throw Error('Enter a JSON object');
            if (['array', 'pairs'].includes(spec.type) && !Array.isArray(value)) throw Error('Enter a JSON array');
          } else if (spec.type === 'int' || spec.type === 'float') {
            value = Number(text);
            if (!text || !Number.isFinite(value) || (spec.type === 'int' && !Number.isSafeInteger(value))) throw Error('Enter a valid number');
          } else value = text;
          this.send('setConfig', { [spec.key]: value });
        } catch (error) { $('#configStatus').textContent = `${label.textContent}: ${error.message}`; }
      };
      row.append(label, input);
      const hint = text => { const item = document.createElement('small'); item.textContent = text; row.append(item); };
      if (spec.description) hint(spec.description);
      if (spec.hint && spec.hint !== spec.description) hint(spec.hint);
      if (spec.supported === false) hint('Not available on this sensor.');
      else if (spec.readOnly || spec.readonly) hint('Read-only device value.');
      else if (spec.current != null) hint(`Device at last load: ${formatValue(spec.current)}`);
      if (spec.restart && !input.disabled) hint('Changing this reconnects scanner capture.');
      section.append(row); this.fields.set(spec.key, { input, spec });
    }
    if (areaKeys.every(key => this.fields.has(key))) {
      const section = this.fields.get(areaKeys.at(-1)).input.closest('details');
      const actions = document.createElement('div'); actions.className = 'actions';
      for (const [id, label, preserve] of [['applyArea', 'Apply area', false], ['preserveArea', 'Preserve area', true]]) {
        const button = document.createElement('button'); button.type = 'button'; button.id = id;
        button.textContent = label; button.onclick = () => this.applyArea(preserve);
        actions.append(button);
      }
      const hint = document.createElement('small'); hint.className = 'hint';
      hint.textContent = 'Enter all four dimensions, then Apply area. Preserve area removes all four overrides without resetting the sensor ROI.';
      section.append(actions, hint);
    }
  }

  dispose() {
    this.clearPending();
    window.removeEventListener('wscmdstatechange', this.onState);
    window.removeEventListener('sporsensorcommand', this.onReply);
  }
}
