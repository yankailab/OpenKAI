const $ = selector => document.querySelector(selector);
const radiansPerDegree = Math.PI / 180;

export class TransformControls {
  constructor() {
    this.pending = null; this.sequence = 0; this.loaded = false; this.socket = null;
    // Shared command servers broadcast replies to every viewer tab.
    this.clientId = Array.from(crypto.getRandomValues(new Uint32Array(4)), value => value.toString(16)).join('-');
    this.translation = ['x', 'y', 'z'].map(axis => $(`#transform-${axis}`));
    this.rotation = ['roll', 'pitch', 'yaw'].map(axis => $(`#transform-${axis}`));
    this.onState = () => {
      const socket = window.wsSocket?.readyState === WebSocket.OPEN ? window.wsSocket : null;
      if (socket === this.socket) { this.refresh(); return; }
      const pending = this.pending;
      this.clearPending(); this.loaded = false; this.socket = socket;
      if (socket) this.send('getConfig');
      else $('#transform-status').textContent = pending?.cmd === 'saveConfig' ?
        'Disconnected. Transform applied; saving was not confirmed.' : 'Command connection disconnected.';
      this.refresh();
    };
    this.onReply = event => this.reply(event.detail);
    window.addEventListener('wscmdstatechange', this.onState);
    window.addEventListener('transformcommand', this.onReply);
    $('#transform-module').oninput = () => {
      this.loaded = false;
      $('#transform-status').textContent = 'Load the selected transform before editing.';
      this.refresh();
    };
    $('#transform-module').onchange = () => this.send('getConfig');
    $('#transform-load').onclick = () => this.send('getConfig');
    $('#transform-config').oninput = event => event.target.setCustomValidity('');
    $('#transform-config').onsubmit = event => {
      event.preventDefault();
      if (!this.loaded || this.pending) return;
      for (const input of [...this.translation, ...this.rotation]) {
        input.setCustomValidity(Number.isFinite(input.valueAsNumber) ? '' : 'Enter a finite number.');
      }
      if (!event.currentTarget.reportValidity()) return;
      this.send('setConfig', {
        vT: this.translation.map(input => input.valueAsNumber),
        vAngle: this.rotation.map(input => input.valueAsNumber * radiansPerDegree),
      });
    };
    this.onState();
  }

  refresh() {
    const connected = window.wsSocket?.readyState === WebSocket.OPEN;
    $('#transform-module').disabled = !!this.pending;
    $('#transform-load').disabled = !connected || !!this.pending;
    $('#transform-fields').disabled = !connected || !this.loaded || !!this.pending;
    $('#transform-save').disabled = !connected || !this.loaded || !!this.pending;
  }

  clearPending() { clearTimeout(this.timeout); this.pending = null; }

  send(cmd, config) {
    if (this.pending) return;
    const module = $('#transform-module').value.trim();
    if (!module || module.includes('EOJ')) {
      $('#transform-status').textContent = 'Enter a backend instance name without the reserved EOJ delimiter.';
      return;
    }
    const requestId = `transform-${this.clientId}-${++this.sequence}`;
    if (!window.wsSendCmd({ cmd, module, requestId, ...(config ? { config } : {}) })) {
      $('#transform-status').textContent = cmd === 'saveConfig' ?
        'Transform applied; save request failed.' : 'Transform request failed. Check the command connection.';
      this.refresh();
      return;
    }
    this.pending = { cmd, module, requestId };
    $('#transform-status').textContent = cmd === 'getConfig' ? 'Loading transform…' :
      cmd === 'setConfig' ? 'Applying transform…' : 'Transform applied; saving to JSON…';
    this.timeout = setTimeout(() => {
      this.clearPending(); this.loaded = false; this.refresh();
      $('#transform-status').textContent = cmd === 'saveConfig' ?
        'Transform applied; saving was not confirmed. Load to check the backend.' :
        `No reply from ${module}. Check the instance name and connection, then Load.`;
    }, 15000);
    this.refresh();
  }

  reply(reply) {
    if (!this.pending || ['cmd', 'module', 'requestId'].some(key => reply[key] !== this.pending[key])) return;
    this.clearPending();
    if (reply.bSuccess !== true) {
      $('#transform-status').textContent = `${reply.cmd === 'saveConfig' ? 'Transform applied; save failed' : 'Transform command failed'}: ${reply.error || 'See the command console.'}`;
      this.refresh();
      return;
    }
    const config = reply.config;
    if (!config || !['vT', 'vAngle'].every(key => Array.isArray(config[key]) && config[key].length === 3 &&
        config[key].every(value => typeof value === 'number' && Number.isFinite(value))) ||
        config.vAngle.some(value => !Number.isFinite(value / radiansPerDegree))) {
      this.loaded = false;
      $('#transform-status').textContent = 'Invalid transform reply. Load to retry.';
      this.refresh();
      return;
    }
    this.translation.forEach((input, axis) => { input.value = config.vT[axis]; });
    this.rotation.forEach((input, axis) => { input.value = config.vAngle[axis] / radiansPerDegree; });
    this.loaded = true;
    if (reply.cmd === 'setConfig') this.send('saveConfig');
    else $('#transform-status').textContent = reply.cmd === 'saveConfig' ?
      `Saved ${reply.module} to JSON.` : `Loaded ${reply.module}.`;
    this.refresh();
  }

  dispose() {
    this.clearPending();
    window.removeEventListener('wscmdstatechange', this.onState);
    window.removeEventListener('transformcommand', this.onReply);
  }
}
