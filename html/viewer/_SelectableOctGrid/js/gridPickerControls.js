const $ = selector => document.querySelector(selector);

export class GridPickerControls {
  constructor(picker) {
    this.picker = picker; this.pending = null; this.socket = null; this.sequence = 0;
    this.clientId = Array.from(crypto.getRandomValues(new Uint32Array(4)), value => value.toString(16)).join('-');
    this.onReply = event => this.reply(event.detail);
    this.onState = () => {
      const socket = window.wsSocket?.readyState === WebSocket.OPEN ? window.wsSocket : null;
      if (socket !== this.socket) {
        if (this.pending) {
          $('#picker-status').textContent = 'Connection changed. The last operation was not confirmed; Load to check selections.';
          this.clearPending();
        }
        this.socket = socket;
      }
      this.refresh();
    };
    window.addEventListener('cellselection', this.onReply);
    window.addEventListener('wscmdstatechange', this.onState);
    $('#picker-load').onclick = () => this.send('getCellSelection');
    $('#picker-apply').onclick = () => this.send('setCellSelection');
    $('#picker-save').onclick = () => this.send('saveConfig');
    $('#picker-clear').onclick = () => {
      if (this.pending) return;
      this.picker.clear();
      $('#picker-status').textContent = 'Selection cleared locally. Apply to clear the running selection.';
      this.refresh();
    };
    this.onState();
  }

  refresh() {
    const count = this.picker.count;
    const available = window.wsSocket?.readyState === WebSocket.OPEN && this.picker.gridModules().length > 0;
    $('#picker-count').textContent = `${count.toLocaleString()} picked ${count === 1 ? 'cell' : 'cells'}`;
    for (const action of ['load', 'apply', 'save']) $('#picker-' + action).disabled = !available || !!this.pending;
    $('#picker-clear').disabled = count === 0 || !!this.pending;
    this.picker.editable = !this.pending;
  }

  clearPending() { clearTimeout(this.timeout); this.pending = null; }

  send(cmd) {
    if (this.pending) return;
    const modules = this.picker.gridModules();
    if (!modules.length) return;
    const commands = cmd === 'setCellSelection' ? this.picker.commands() : modules.map(module => ({ cmd, module }));
    const batch = this.pending = { cmd, requests: new Map(), succeeded: 0, failures: [] };
    for (const command of commands) {
      const requestId = `picker-${this.clientId}-${++this.sequence}`;
      if (window.wsSendCmd({ ...command, requestId })) batch.requests.set(requestId, { cmd, module: command.module });
      else batch.failures.push(`${command.module}: request could not be sent`);
    }
    if (!batch.requests.size) { this.finish(); return; }
    $('#picker-status').textContent = cmd === 'getCellSelection' ? 'Loading running selections…' :
      cmd === 'setCellSelection' ? 'Applying selections…' : 'Saving running grid configs and selections…';
    this.timeout = setTimeout(() => {
      for (const { module } of batch.requests.values()) batch.failures.push(`${module}: no reply; Load to check the backend`);
      this.finish();
    }, 15000);
    this.refresh();
  }

  reply(reply) {
    const request = this.pending?.requests.get(reply.requestId);
    if (!request || reply.cmd !== request.cmd || reply.module !== request.module) return;
    this.pending.requests.delete(reply.requestId);
    try {
      if (reply.bSuccess !== true) throw new Error(reply.error || 'See the command console.');
      if (reply.cmd !== 'saveConfig') this.picker.replaceSelection(reply);
      ++this.pending.succeeded;
    } catch (error) {
      this.pending.failures.push(`${reply.module}: ${error.message}`);
    }
    if (!this.pending.requests.size) this.finish();
  }

  finish() {
    const batch = this.pending;
    this.clearPending();
    const grids = `${batch.succeeded} ${batch.succeeded === 1 ? 'grid' : 'grids'}`;
    const result = batch.cmd === 'getCellSelection' ? `Loaded selection for ${grids}` :
      batch.cmd === 'setCellSelection' ? `Applied selection for ${grids}` : `Saved ${grids} to JSON`;
    $('#picker-status').textContent = batch.failures.length ? `${result}. Failed: ${batch.failures.join('; ')}` : result + '.';
    this.refresh();
  }

  dispose() {
    this.clearPending();
    window.removeEventListener('cellselection', this.onReply);
    window.removeEventListener('wscmdstatechange', this.onState);
    this.picker.editable = true;
  }
}
