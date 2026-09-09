import { GeometryConnection } from './wsStreamBase.js';
import { decodeFrame } from './protocol.js';
import { Viewer3D } from './viewer3D.js';

const $ = selector => document.querySelector(selector);
const viewer = new Viewer3D($('#viewport'));
let pending = null, names = new Map(), visibility = new Map(), objectKey = '';
let bytes = 0, frames = 0, lastStats = performance.now(), latestCounts = [0, 0];
const connection = new GeometryConnection({
  onHello(config) {
    names = new Map(config.objects.map(o => [o.id, o.name]));
    visibility.clear();
    viewer.configure(config);
    $('#grid').checked = config.showGrid;
    $('#fit').disabled = $('#reset').disabled = false;
    $('#welcome').hidden = true;
  },
  onFrame(buffer, acknowledge) { pending = { frame: decodeFrame(buffer), acknowledge }; },
  onStatus(text) {
    $('#status').textContent = text;
    syncConnectionButtons();
  },
  onReset() {
    pending = null; viewer.clear(); objectKey = '';
    $('#objects').textContent = 'No geometry sources';
    $('#fit').disabled = $('#reset').disabled = true;
    $('#stats').textContent = 'Waiting for geometry';
    bytes = frames = 0; lastStats = performance.now();
  }
});
function syncConnectionButtons() {
  $('#start').disabled = connection.running;
  $('#stop').disabled = !connection.running && !window.wsCmdActive();
}
function start(event) {
  event?.preventDefault();
  try {
    connection.start(window.viewerEndpoint());
    window.wsInit();
    syncConnectionButtons();
  } catch (error) { $('#status').textContent = error.message; }
}
function stop() { connection.stop(); window.wsStop(); syncConnectionButtons(); }
window.addEventListener('wscmdstatechange', syncConnectionButtons);
$('#connection').addEventListener('submit', start);
$('#stop').addEventListener('click', stop);
$('#fit').addEventListener('click', () => viewer.fit());
$('#reset').addEventListener('click', () => viewer.resetCamera());
$('#grid').addEventListener('change', () => { viewer.grid.visible = $('#grid').checked; });
$('#point-scale').addEventListener('input', () => viewer.setPointScale(Number($('#point-scale').value)));
$('#start').disabled = false;

let animation;
function draw(now) {
  try {
    if (pending) {
      const { frame, acknowledge } = pending;
      pending = null;
      viewer.update(frame);
      bytes += frame.bytes; ++frames;
      latestCounts = frame.objects.reduce((n, o) => [n[0] + o.nP, n[1] + o.nL], [0, 0]);
      const key = frame.objects.map(o => o.id).join(',');
      if (key !== objectKey) {
        objectKey = key;
        $('#objects').replaceChildren();
        if (!frame.objects.length) $('#objects').textContent = 'No geometry sources';
        for (const o of frame.objects) {
          const label = document.createElement('label'), checkbox = document.createElement('input');
          checkbox.type = 'checkbox'; checkbox.checked = visibility.get(o.id) ?? true;
          checkbox.addEventListener('change', () => { visibility.set(o.id, checkbox.checked); viewer.setVisible(o.id, checkbox.checked); });
          label.append(checkbox, document.createTextNode(names.get(o.id) || `Object ${o.id}`));
          $('#objects').append(label);
          viewer.setVisible(o.id, checkbox.checked);
        }
      }
      viewer.render();
      acknowledge(); // Credit only after uploading and submitting this frame for rendering.
    } else viewer.render();
    if (frames && now - lastStats >= 500) {
      const seconds = (now - lastStats) / 1000;
      $('#stats').textContent = `${latestCounts[0].toLocaleString()} points · ${latestCounts[1].toLocaleString()} lines · ${(frames / seconds).toFixed(1)} fps · ${(bytes / seconds / 1048576).toFixed(1)} MiB/s`;
      bytes = frames = 0; lastStats = now;
    }
  } catch (error) {
    connection.fail(error);
    syncConnectionButtons();
  }
  animation = requestAnimationFrame(draw);
}
animation = requestAnimationFrame(draw);
window.addEventListener('pagehide', () => { connection.stop(); cancelAnimationFrame(animation); viewer.dispose(); });
if (location.hash === '#connect') { history.replaceState(null, '', location.pathname + location.search); start(); }
