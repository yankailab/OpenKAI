const $ = selector => document.querySelector(selector);
const integerLimits = {
  dTexpireCell: 18446744073709551615n,
  dTexpirePCL: 18446744073709551615n,
  nMaxCells: 2147483647n,
  nMaxLevel: 40n,
  nPminBuild: 2147483647n,
};
const decimal = /^-?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][+-]?\d+)?$/;

function vectorError(value, positive) {
  const number = typeof value === 'string' && decimal.test(value) ? Math.fround(Number(value)) : NaN;
  const underflow = number === 0 && /[1-9]/.test(value.split(/[eE]/)[0]);
  return !Number.isFinite(number) || underflow ? 'Enter a finite coordinate in metres within the float32 range.' :
    positive && number <= 0 ? 'Cell size must be greater than zero.' : '';
}

function integerError(value, limit) {
  return typeof value !== 'string' || !/^\d+$/.test(value) || BigInt(value) > limit ?
    `Enter a whole number from 0 to ${limit}.` : '';
}

export class GridConfigControls {
  constructor() {
    this.pending = null; this.sequence = 0; this.loaded = false; this.socket = null; this.modules = [];
    // Command replies are broadcast to every viewer connected to the server.
    this.clientId = Array.from(crypto.getRandomValues(new Uint32Array(4)), value => value.toString(16)).join('-');
    this.vectors = {
      vPorigin: ['x', 'y', 'z'].map(axis => $(`#grid-origin-${axis}`)),
      vRootCellSize: ['x', 'y', 'z'].map(axis => $(`#grid-size-${axis}`)),
    };
    this.integers = Object.fromEntries(Object.keys(integerLimits).map(key => [key, $(`#grid-${key}`)]));
    this.onState = () => {
      const socket = window.wsSocket?.readyState === WebSocket.OPEN ? window.wsSocket : null;
      if (socket === this.socket) { this.refresh(); return; }
      const pending = this.pending;
      this.clearPending(); this.loaded = false; this.socket = socket;
      if (socket && this.modules.length) this.send('getGridConfig');
      else $('#grid-config-status').textContent = socket ? 'Waiting for a grid source…' :
        pending?.cmd === 'saveConfig' ? 'Disconnected. Saving was not confirmed.' :
        pending?.cmd === 'setGridConfig' ? 'Disconnected. Apply was not confirmed; Load to check the grid.' :
          'Connect the stream and Commands to load the grid config.';
      this.refresh();
    };
    this.onReply = event => this.reply(event.detail);
    window.addEventListener('wscmdstatechange', this.onState);
    window.addEventListener('gridconfig', this.onReply);
    $('#grid-config-source').onchange = () => {
      this.clearPending(); this.loaded = false;
      this.send('getGridConfig');
    };
    $('#grid-config-load').onclick = () => this.send('getGridConfig');
    $('#grid-config-save').onclick = () => this.send('saveConfig');
    $('#grid-config').oninput = event => event.target.setCustomValidity('');
    $('#grid-config').onsubmit = event => {
      event.preventDefault();
      if (!this.loaded || this.pending) return;
      const config = {};
      for (const [key, inputs] of Object.entries(this.vectors)) {
        config[key] = inputs.map(input => input.value);
        inputs.forEach(input => input.setCustomValidity(vectorError(input.value, key === 'vRootCellSize')));
      }
      for (const [key, input] of Object.entries(this.integers)) {
        // Never round expiry durations through Number/valueAsNumber.
        config[key] = input.value;
        input.setCustomValidity(integerError(input.value, integerLimits[key]));
      }
      if (!event.currentTarget.reportValidity()) return;
      this.send('setGridConfig', config);
    };
    this.onState();
  }

  setModules(modules) {
    modules = [...new Set(modules)];
    if (modules.length === this.modules.length && modules.every((module, i) => module === this.modules[i])) return;
    const select = $('#grid-config-source'), previous = select.value;
    this.modules = modules;
    select.replaceChildren(...modules.map(module => {
      const option = document.createElement('option');
      option.value = option.textContent = module;
      return option;
    }));
    select.value = modules.includes(previous) ? previous : modules[0] || '';
    $('#grid-config-source-label').hidden = modules.length < 2;
    if (select.value !== previous || !modules.length) {
      this.clearPending(); this.loaded = false;
      if (this.socket && modules.length) this.send('getGridConfig');
      else $('#grid-config-status').textContent = 'Waiting for a connected grid source…';
    }
    this.refresh();
  }

  refresh() {
    const available = window.wsSocket?.readyState === WebSocket.OPEN && this.modules.length > 0;
    $('#grid-config-source').disabled = !!this.pending || !this.modules.length;
    $('#grid-config-load').disabled = !available || !!this.pending;
    $('#grid-config-fields').disabled = !available || !this.loaded || !!this.pending;
    $('#grid-config-apply').disabled = !available || !this.loaded || !!this.pending;
    $('#grid-config-save').disabled = !available || !this.loaded || !!this.pending;
  }

  clearPending() { clearTimeout(this.timeout); this.pending = null; }

  send(cmd, config) {
    if (this.pending) return;
    const module = $('#grid-config-source').value;
    if (!this.modules.includes(module)) return;
    const requestId = `grid-${this.clientId}-${++this.sequence}`;
    if (!window.wsSendCmd({ cmd, module, requestId, ...config })) {
      $('#grid-config-status').textContent = 'Grid request failed. Check the command connection.';
      this.refresh();
      return;
    }
    this.pending = { cmd, module, requestId };
    $('#grid-config-status').textContent = cmd === 'getGridConfig' ? 'Loading grid config…' :
      cmd === 'setGridConfig' ? 'Applying grid config…' : 'Saving running grid config and selection…';
    this.timeout = setTimeout(() => {
      this.clearPending();
      if (cmd !== 'saveConfig') this.loaded = false;
      this.refresh();
      $('#grid-config-status').textContent = cmd === 'saveConfig' ? 'Saving was not confirmed. Retry Save to persist the running config.' :
        cmd === 'setGridConfig' ? 'Apply was not confirmed. Load to check the grid.' :
        `No reply from ${module}. Check the command connection, then Load.`;
    }, 15000);
    this.refresh();
  }

  reply(reply) {
    if (!this.pending || ['cmd', 'module', 'requestId'].some(key => reply[key] !== this.pending[key])) return;
    this.clearPending();
    if (reply.bSuccess !== true) {
      if (reply.cmd === 'getGridConfig') this.loaded = false;
      $('#grid-config-status').textContent = `${reply.cmd === 'saveConfig' ? 'Saving grid config failed' : 'Grid command failed'}: ${reply.error || 'See the command console.'}`;
      this.refresh();
      return;
    }
    if (reply.cmd === 'saveConfig') {
      $('#grid-config-status').textContent = `Saved ${reply.module} to JSON.`;
      this.refresh();
      return;
    }
    const validVectors = Object.keys(this.vectors).every(key => Array.isArray(reply[key]) && reply[key].length === 3 &&
      reply[key].every(value => !vectorError(value, key === 'vRootCellSize')));
    if (!validVectors || Object.entries(integerLimits).some(([key, limit]) => integerError(reply[key], limit))) {
      this.loaded = false;
      $('#grid-config-status').textContent = 'Invalid grid config reply. Load to retry.';
      this.refresh();
      return;
    }
    for (const [key, inputs] of Object.entries(this.vectors)) inputs.forEach((input, axis) => {
      input.value = reply[key][axis]; input.setCustomValidity('');
    });
    for (const [key, input] of Object.entries(this.integers)) {
      input.value = reply[key]; input.setCustomValidity('');
    }
    this.loaded = true;
    $('#grid-config-status').textContent = reply.cmd === 'setGridConfig' ?
      `Applied ${reply.module}.` : `Loaded ${reply.module}.`;
    this.refresh();
  }

  dispose() {
    this.clearPending();
    window.removeEventListener('wscmdstatechange', this.onState);
    window.removeEventListener('gridconfig', this.onReply);
  }
}
