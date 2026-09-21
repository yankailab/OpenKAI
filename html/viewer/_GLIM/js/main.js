import { SLAMControls } from './slamControls.js';
import { GeometryConnection } from './wsStreamBase.js';
import { decodeFrame, STREAM_TYPES } from './protocol.js';
import { Viewer3D } from './viewer3D.js';

const $ = selector => document.querySelector(selector);
const viewer = new Viewer3D($('#viewport'));
const slamControls = new SLAMControls(viewer);
const pending = new Map();
const counts = Object.fromEntries(STREAM_TYPES.map(type => [type, 0]));
const states = Object.fromEntries(STREAM_TYPES.map(type => [type, 'Ready']));
let configured = false, bytes = 0, frames = 0, lastStats = performance.now();
const connections = Object.fromEntries(STREAM_TYPES.map(type => [type, new GeometryConnection({
  type,
  onHello(config) {
    if (!configured) { viewer.configure(config); configured = true; }
    $('#welcome').hidden = true;
  },
  onFrame(buffer, acknowledge) { pending.set(type, { frame: decodeFrame(buffer, type), acknowledge }); },
  onStatus(text) {
    states[type] = text;
    $('#status').textContent = STREAM_TYPES.every(t => states[t] === 'Connected') ? 'Connected' :
      STREAM_TYPES.every(t => states[t] === 'Stopped') ? 'Stopped' :
      STREAM_TYPES.map(t => `${t[0].toUpperCase() + t.slice(1)}: ${states[t]}`).join(' · ');
    const running = STREAM_TYPES.some(t => connections[t].running);
    $('#start').disabled = running;
    $('#stop').disabled = !running;
  },
  onReset() {
    pending.delete(type);
    viewer.clearStream(type);
    counts[type] = 0;
  }
})]));
function start(event) {
  event?.preventDefault();
  try {
    const endpoint = window.viewerEndpoint();
    window.wsInit();
    configured = false;
    viewer.clear();
    bytes = frames = 0; lastStats = performance.now();
    for (const type of STREAM_TYPES) connections[type].start(endpoint);
  } catch (error) { $('#status').textContent = error.message; }
}
function stop() {
  for (const type of STREAM_TYPES) connections[type].stop();
}
$('#connection').addEventListener('submit', start);
$('#stop').addEventListener('click', stop);
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
    viewer.render();
    slamControls.tick(now);
    for (const acknowledge of credits) acknowledge();
    if (credits.length) ++frames;
    if (now - lastStats >= 500) {
      const seconds = (now - lastStats) / 1000;
      $('#stats').textContent = `${counts.points.toLocaleString()} points · ${counts.lines.toLocaleString()} lines · ${(frames / seconds).toFixed(1)} fps · ${(bytes / seconds / 1048576).toFixed(1)} MiB/s`;
      bytes = frames = 0; lastStats = now;
    }
  } catch (error) { $('#status').textContent = `Rendering: ${error.message}`; }
  animation = requestAnimationFrame(draw);
}
animation = requestAnimationFrame(draw);
window.addEventListener('pagehide', () => { stop(); cancelAnimationFrame(animation); viewer.dispose(); slamControls.dispose(); });
if (location.hash === '#connect') { history.replaceState(null, '', location.pathname + location.search); start(); }
