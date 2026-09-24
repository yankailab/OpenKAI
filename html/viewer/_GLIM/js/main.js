import { SLAMControls } from './slamControls.js';
import { GLIMConnection } from './wsStreamBase.js';
import { Viewer3D } from './viewer3D.js';
const $ = selector => document.querySelector(selector);
const viewer = new Viewer3D($('#viewport'));
const slamControls = new SLAMControls(viewer);
let pending = null, bytes = 0, lastStats = performance.now();
const connection = new GLIMConnection({
  onHello(config) { viewer.configure(config); $('#welcome').hidden = true; },
  onEvent(event, acknowledge) { pending = { event, acknowledge }; },
  onStatus(text) {
    $('#status').textContent = text;
    $('#start').disabled = connection.running; $('#stop').disabled = !connection.running;
  },
  onReset() { pending = null; viewer.clear(); }
});
function start(event) {
  event?.preventDefault();
  try {
    const endpoint = window.viewerEndpoint(); window.wsInit();
    bytes = 0; lastStats = performance.now(); connection.start(endpoint);
  } catch (error) { $('#status').textContent = error.message; }
}
$('#connection').addEventListener('submit', start);
$('#stop').addEventListener('click', () => connection.stop());
$('#start').disabled = false;
let animation;
function draw(now) {
  let acknowledge;
  if (pending) {
    const message = pending; pending = null;
    try { viewer.update(message.event); bytes += message.event.bytes; acknowledge = message.acknowledge; }
    catch (error) { connection.fail(error); }
  }
  try {
    viewer.render(); slamControls.tick(now); acknowledge?.();
    if (now - lastStats >= 500) {
      const seconds = (now - lastStats) / 1000;
      $('#stats').textContent = `${viewer.pointCount.toLocaleString()} points · ${viewer.objects.size.toLocaleString()} submaps · ${(bytes / seconds / 1048576).toFixed(2)} MiB/s`;
      $('#mapEmpty').hidden = !connection.running || viewer.pointCount > 0;
      bytes = 0; lastStats = now;
    }
  } catch (error) { connection.fail(error); }
  animation = requestAnimationFrame(draw);
}
animation = requestAnimationFrame(draw);
window.addEventListener('pagehide', () => { connection.stop(); cancelAnimationFrame(animation); viewer.dispose(); slamControls.dispose(); });
if (location.hash === '#connect') { history.replaceState(null, '', location.pathname + location.search); start(); }
