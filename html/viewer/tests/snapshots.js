import * as geometry from '../_GeometryBase/js/protocol.js';
import * as grid from '../_SelectableOctGrid/js/protocol.js';
import { Viewer3D as GeometryViewer } from '../_GeometryBase/js/viewer3D.js';
import { Viewer3D as GridViewer } from '../_SelectableOctGrid/js/viewer3D.js';

function assert(condition, message) {
  if (!condition) throw new Error(message);
}

function rejects(action, message) {
  let rejected = false;
  try { action(); } catch { rejected = true; }
  assert(rejected, message);
}

async function load(name) {
  const response = await fetch(`/fixtures/${name}.bin`);
  assert(response.ok, `Missing C++ fixture ${name}`);
  return response.arrayBuffer();
}

async function run() {
  const points = await load('points');
  const lines = await load('lines');
  const emptyPoints = await load('empty-points');
  const emptyLines = await load('empty-lines');
  for (const protocol of [geometry, grid]) {
    const decoded = protocol.decodeFrame(points, 'points');
    assert(decoded.objects[0].count === 2, 'Point cap/invalid filtering');
    assert(decoded.objects[0].positions[0] === 1, 'C++ point positions');
    assert(decoded.objects[0].colors[1] === 255, 'C++ fallback color');
    assert(protocol.decodeFrame(lines, 'lines').objects[0].count === 1, 'C++ line encoding');
    assert(protocol.decodeFrame(emptyPoints, 'points').objects[0].count === 0, 'Empty point snapshot');
    assert(protocol.decodeFrame(emptyLines, 'lines').objects[0].count === 0, 'Empty line snapshot');
    rejects(() => protocol.decodeFrame(lines, 'points'), 'Reject wrong stream');
    for (const version of [1, 2, 3, 4, 5, 7]) {
      const stale = points.slice(0);
      new DataView(stale).setUint32(4, version, true);
      rejects(() => protocol.decodeFrame(stale, 'points'), `Reject protocol ${version}`);
    }
  }

  // Exercise actual frontend uploads and clear behavior without requiring a GPU.
  for (const Viewer of [GeometryViewer, GridViewer]) {
    const viewer = Object.create(Viewer.prototype);
    viewer.objects = new Map();
    viewer.pointScale = 1;
    viewer.autoBound = false;
    viewer.updateBounds = () => {};
    viewer.createObject = id => {
      const mesh = () => ({ material: {}, geometry: { setDrawRange(start, count) { this.count = count; } } });
      const object = { id, kind: 'geometry', visible: true, streams: new Map(), points: mesh(), lines: mesh() };
      viewer.objects.set(id, object);
      return object;
    };
    viewer.upload = (mesh, positions) => mesh.geometry.setDrawRange(0, positions.length / 3);
    viewer.removeObject = object => viewer.objects.delete(object.id);
    viewer.update(geometry.decodeFrame(points));
    viewer.update(geometry.decodeFrame(lines));
    assert(viewer.objects.get(3).points.geometry.count === 2, 'Render populated points');
    viewer.update(geometry.decodeFrame(emptyPoints));
    assert(viewer.objects.get(3).points.geometry.count === 0, 'Clear points on empty publication');
    assert(viewer.objects.get(3).lines.geometry.count === 2, 'Point clear preserves lines');
    viewer.update(geometry.decodeFrame(emptyLines));
    assert(viewer.objects.get(3).lines.geometry.count === 0, 'Clear lines on empty publication');
  }
}

run().then(() => { document.querySelector('#result').textContent = 'PASS'; })
  .catch(error => { document.querySelector('#result').textContent = `FAIL: ${error.stack}`; });
