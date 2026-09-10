import { GeometryConnection } from './wsStreamBase.js';
import { decodeFrame, STREAM_TYPES } from './protocol.js';
import { Viewer3D } from './viewer3D.js';

const $ = selector => document.querySelector(selector);
const viewer = new Viewer3D($('#viewport'));
const pending = new Map(), names = new Map(), visibility = new Map();
const counts = Object.fromEntries(STREAM_TYPES.map(type => [type, 0]));
const states = Object.fromEntries(STREAM_TYPES.map(type => [type, 'Ready']));
let configured = false, objectKey = '', bytes = 0, frames = 0, lastStats = performance.now();
const connections = Object.fromEntries(STREAM_TYPES.map(type => [type, new GeometryConnection({
  type,
  onHello(config) {
    for (const o of config.objects) names.set(o.id, o.name);
    if (!configured) {
      viewer.configure(config);
      configured = true;
      $('#grid').checked = config.showGrid;
    }
    viewer.picker.configure([...names].map(([id, name]) => ({ id, name })));
    $('#fit').disabled = $('#reset').disabled = false;
    $('#welcome').hidden = true;
    syncObjects();
  },
  onFrame(buffer, acknowledge) { pending.set(type, { frame: decodeFrame(buffer, type), acknowledge }); },
  onStatus(text) {
    states[type] = text;
    $('#status').textContent = STREAM_TYPES.every(t => states[t] === 'Connected') ? 'Connected' :
      STREAM_TYPES.every(t => states[t] === 'Stopped') ? 'Stopped' :
      STREAM_TYPES.map(t => `${t[0].toUpperCase() + t.slice(1)}: ${states[t]}`).join(' · ');
    syncConnectionButtons();
  },
  onReset() {
    pending.delete(type);
    viewer.clearStream(type);
    counts[type] = 0;
    syncObjects();
  }
})]));
function syncConnectionButtons() {
  const running = STREAM_TYPES.some(t => connections[t].running);
  $('#start').disabled = running;
  $('#stop').disabled = !running && !window.wsCmdActive();
  $('#fit').disabled = $('#reset').disabled = !configured;
  syncPicker();
}
function syncObjects() {
  const objects = [...viewer.objects.values()].sort((a, b) => a.id - b.id);
  const key = JSON.stringify(objects.map(o => [o.id, names.get(o.id)]));
  if (key === objectKey) return;
  objectKey = key;
  $('#objects').replaceChildren();
  if (!objects.length) $('#objects').textContent = 'No geometry sources';
  for (const o of objects) {
    const label = document.createElement('label'), checkbox = document.createElement('input');
    checkbox.type = 'checkbox'; checkbox.checked = visibility.get(o.id) ?? true;
    checkbox.addEventListener('change', () => { visibility.set(o.id, checkbox.checked); viewer.setVisible(o.id, checkbox.checked); });
    label.append(checkbox, document.createTextNode(names.get(o.id) || `Object ${o.id}`));
    $('#objects').append(label);
    viewer.setVisible(o.id, checkbox.checked);
  }
}
function syncPicker() {
  const count = viewer.picker.count;
  const canConfigure = window.wsSocket?.readyState === WebSocket.OPEN && viewer.picker.gridModules().length > 0;
  $('#picker-count').textContent = `${count.toLocaleString()} picked ${count === 1 ? 'cell' : 'cells'}`;
  $('#picker-clear').disabled = count === 0;
  $('#picker-load').disabled = !canConfigure;
  $('#grid-config-update').disabled = !canConfigure;
  $('#picker-load').title = 'Retrieve selections from the connected grid sources';
  $('#picker-send').disabled = count === 0 || window.wsSocket?.readyState !== WebSocket.OPEN;
  $('#picker-send').title = window.wsSocket?.readyState === WebSocket.OPEN ? '' : 'Connect the command WebSocket to send selections';
}
viewer.picker.onChange = () => { $('#picker-status').textContent = ''; syncPicker(); };
$('#grid-config').addEventListener('input', event => event.target.setCustomValidity(''));
$('#grid-config').addEventListener('submit', event => {
  event.preventDefault();
  const vector = name => ['x', 'y', 'z'].map(axis => $(`#grid-${name}-${axis}`));
  const origin = vector('origin'), size = vector('size');
  for (const input of [...origin, ...size]) {
    const value = Math.fround(input.valueAsNumber);
    input.setCustomValidity(!Number.isFinite(value) ? 'Enter a finite coordinate in metres.' :
      size.includes(input) && value <= 0 ? 'Cell size must be greater than zero.' : '');
  }
  if (!event.currentTarget.reportValidity()) return;
  for (const module of viewer.picker.gridModules()) {
    if (!window.wsSendCmd({ cmd: 'setGridConfig', module,
      vPorigin: origin.map(input => String(input.valueAsNumber)),
      vRootCellSize: size.map(input => String(input.valueAsNumber)) })) {
      $('#grid-config-status').textContent = 'Update request failed.';
      return;
    }
  }
  $('#grid-config-status').textContent = 'Updating grid…';
});
window.addEventListener('gridconfig', event => {
  const reply = event.detail;
  $('#grid-config-status').textContent = reply.bSuccess === true ? `Updated ${reply.module}.` : `Update failed for ${reply.module}.`;
});
$('#picker-load').addEventListener('click', () => {
  for (const command of viewer.picker.loadCommands()) {
    if (!window.wsSendCmd(command)) { $('#picker-status').textContent = 'Load request failed.'; return; }
  }
  $('#picker-status').textContent = 'Loading selected cells…';
});
window.addEventListener('cellselect', event => {
  try {
    const added = viewer.picker.mergeSelection(event.detail);
    $('#picker-status').textContent = `Added ${added} ${added === 1 ? 'cell' : 'cells'} from ${event.detail.module}.`;
  } catch (error) {
    $('#picker-status').textContent = `Load failed: ${error.message}`;
    window.wsCmdLog(`cellSelect: ${error.message}`);
  }
});
$('#picker-clear').addEventListener('click', () => viewer.picker.clear());
$('#picker-send').addEventListener('click', () => {
  let sent = 0;
  for (const command of viewer.picker.commands()) {
    if (!window.wsSendCmd(command)) {
      $('#picker-status').textContent = `Send failed. ${sent} cells sent; selections kept.`;
      syncPicker();
      return;
    }
    sent += command.cellIDs.length;
  }
  $('#picker-status').textContent = `Sent ${sent} ${sent === 1 ? 'cell' : 'cells'}.`;
});
function start(event) {
  event?.preventDefault();
  try {
    const endpoint = window.viewerEndpoint();
    configured = false;
    viewer.clear(); names.clear(); visibility.clear();
    bytes = frames = 0; lastStats = performance.now();
    for (const type of STREAM_TYPES) connections[type].start(endpoint);
    window.wsInit();
    syncConnectionButtons();
  } catch (error) { $('#status').textContent = error.message; }
}
function stop() {
  for (const type of STREAM_TYPES) connections[type].stop();
  window.wsStop(); syncConnectionButtons();
}
window.addEventListener('wscmdstatechange', syncConnectionButtons);
$('#connection').addEventListener('submit', start);
$('#stop').addEventListener('click', stop);
$('#fit').addEventListener('click', () => viewer.fit());
$('#reset').addEventListener('click', () => viewer.resetCamera());
$('#grid').addEventListener('change', () => { viewer.grid.visible = $('#grid').checked; });
$('#grid-solid').addEventListener('change', () => viewer.setGridSolid($('#grid-solid').checked));
$('#point-scale').addEventListener('input', () => viewer.setPointScale(Number($('#point-scale').value)));
for (const id of ['grid-min-level', 'grid-max-level']) {
  $(`#${id}`).addEventListener('input', () => {
    const min = $('#grid-min-level'), max = $('#grid-max-level');
    // Moving one end past the other moves the other end with it.
    if (Number(min.value) > Number(max.value)) {
      if (id === 'grid-min-level') max.value = min.value;
      else min.value = max.value;
    }
    $('#grid-min-level-value').textContent = min.value;
    $('#grid-max-level-value').textContent = max.value;
    viewer.setGridLevelRange(Number(min.value), Number(max.value));
  });
}
$('#start').disabled = false;

let animation;
function draw(now) {
  const credits = [];
  for (const [type, message] of pending) {
    pending.delete(type);
    try {
      viewer.update(message.frame);
      bytes += message.frame.bytes;
      counts[type] = message.frame.objects.reduce((n, o) => n + o.count, 0);
      credits.push(message.acknowledge);
    } catch (error) { connections[type].fail(error); }
  }
  try {
    syncObjects();
    viewer.render();
    for (const acknowledge of credits) acknowledge();
    if (credits.length) ++frames;
    if (now - lastStats >= 500) {
      const seconds = (now - lastStats) / 1000;
      $('#stats').textContent = `${counts.points.toLocaleString()} points · ${counts.lines.toLocaleString()} lines · ${counts.cells.toLocaleString()} cells · ${(frames / seconds).toFixed(1)} fps · ${(bytes / seconds / 1048576).toFixed(1)} MiB/s`;
      bytes = frames = 0; lastStats = now;
    }
  } catch (error) { $('#status').textContent = `Rendering: ${error.message}`; }
  animation = requestAnimationFrame(draw);
}
animation = requestAnimationFrame(draw);
window.addEventListener('pagehide', () => { for (const type of STREAM_TYPES) connections[type].stop(); cancelAnimationFrame(animation); viewer.dispose(); });
if (location.hash === '#connect') { history.replaceState(null, '', location.pathname + location.search); start(); }
