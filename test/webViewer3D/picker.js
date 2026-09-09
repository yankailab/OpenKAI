import * as THREE from '/vendor/three.module.min.js';
import { OrbitControls } from '/vendor/OrbitControls.js';
import { Viewer3D } from '/js/viewer3D.js';
import { GridBoxes } from '/js/gridBoxes.js';
import { cellBox, validateCellIDs } from '/js/octreeCells.js';
import { cellIDKey, childID, remapSelection, rootBounds, selectionGrid } from '/js/gridSelection.js';

function check(condition, message) { if (!condition) throw Error(message); }
const root = new Uint8Array(16);
function idAt(...path) { return path.reduce((id, child) => childID(id, child), root); }
function selected(...ids) { return new Map(ids.map(id => [cellIDKey(id), id])); }
function volume(header, ids) {
  const grid = selectionGrid(header, ids);
  return [...ids.values()].reduce((sum, _, i) => { const { size } = cellBox(grid, i); return sum + size[0] * size[1] * size[2]; }, 0);
}
function inside(header, ids, min, max) {
  const grid = selectionGrid(header, ids);
  for (let i = 0; i < ids.size; ++i) {
    const box = cellBox(grid, i);
    check(box.center.every((x, axis) => x - box.size[axis] / 2 >= min[axis] - 1e-12 && x + box.size[axis] / 2 <= max[axis] + 1e-12), 'Remapped volume moved');
  }
}
export async function runPickerTests() {
  const header = { origin: [0, 0, 0], size: [2, 2, 2], maxLevel: 4 };
  let id = root;
  for (let depth = 0; depth <= 40; ++depth) {
    validateCellIDs(selectionGrid({ ...header, maxLevel: 40 }, selected(id)).cells, 40);
    check((id[0] & 63) === depth && cellIDKey(id).length === 32, 'Inexact ID encoding');
    if (depth < 40) id = childID(id, (depth * 5 + 3) % 8);
  }
  const old = selected(idAt(7));
  const translated = { ...header, origin: [.25, .25, .25] };
  let mapped = remapSelection(header, translated, old);
  check(mapped.size > 1 && volume(translated, mapped) === 1, 'Translation did not preserve selected volume');
  inside(translated, mapped, [0, 0, 0], [1, 1, 1]);
  const expanded = { ...header, size: [4, 4, 4] };
  mapped = remapSelection(header, expanded, old);
  check(mapped.size === 1 && volume(expanded, mapped) === 1, 'Root expansion changed volume');
  inside(expanded, mapped, [0, 0, 0], [1, 1, 1]);
  const shrunk = { ...header, origin: [.5, .5, .5], size: [.5, .5, .5] };
  mapped = remapSelection(header, shrunk, old);
  check(mapped.size === 1 && [...mapped.keys()][0] === cellIDKey(root), 'Root clipping failed');
  check(remapSelection(header, { ...header, origin: [10, 0, 0] }, old).size === 0, 'Outside-root selection survived');
  const coarse = { ...header, size: [100, 100, 100], maxLevel: 0 };
  check(remapSelection(header, coarse, old).size === 1, 'Coarser root erased an in-bounds selection');
  const limited = remapSelection(header, translated, old, 1);
  check(limited.size === 1 && volume(translated, limited) >= 1, 'Work budget erased selected volume');
  const overlaps = remapSelection(header, translated, selected(idAt(7), idAt(7, 7)));
  check(volume(translated, overlaps) === 1, 'Nested selections counted twice');
  const reverse = remapSelection(header, translated, selected(idAt(7, 7), idAt(7)));
  check(volume(translated, reverse) === 1, 'Selection insertion order changed remapping');

  const container = document.createElement('div');
  container.style.cssText = 'position:fixed;left:0;top:0;width:300px;height:300px';
  document.body.append(container);
  const viewer = new Viewer3D(container), picker = viewer.picker;
  picker.configure([{ id: 8, name: 'octGrid' }, { id: 9, name: 'otherGrid' }]);
  const boxes = new GridBoxes();
  const grid = selectionGrid(header, selected(root, idAt(7), idAt(7, 7)));
  for (let at = 0; at < grid.cells.length; at += 19) { grid.cells[at + 16] = 0; grid.cells[at + 17] = 255; }
  boxes.update(grid, rootBounds(header), 1);
  picker.updateObject({ id: 8, visible: true, boxes }, grid);
  const ray = new THREE.Ray(new THREE.Vector3(.75, .75, 5), new THREE.Vector3(0, 0, -1));
  let hit = picker.pickRay(ray);
  check(hit?.depth === 2, 'Did not pick deepest overlapped occupied cell');
  check(picker.pickRay(ray, 0, 3) === null, 'Picked a cell beyond the far plane');
  check(picker.pickRay(ray, 4.25, 4.3)?.depth === 2, 'Cell containing the clipped ray interval was not picked');
  picker.toggle(hit.source, hit.id);
  check(picker.count === 1 && hit.source.overlay.grid.cells[16] === 255 && hit.source.overlay.grid.cells[17] === 0, 'Selection not red');
  // Input buffers can be replaced without changing the retained ID.
  const saved = picker.commands()[0].cellIDs[0];
  grid.cells.fill(0);
  check(picker.commands()[0].cellIDs[0] === saved, 'Selection aliases frame storage');
  const empty = selectionGrid(header, selected());
  boxes.update(empty, rootBounds(header), 1);
  picker.updateObject({ id: 8, visible: true, boxes }, empty);
  check(picker.count === 1 && picker.pickRay(ray)?.depth === 2, 'Expired cell was lost');
  picker.removeObject(8);
  check(picker.count === 1 && picker.pickRay(ray), 'Absent source removed selection');
  picker.setVisible(8, false);
  check(picker.pickRay(ray) === null, 'Hidden selection was pickable');
  picker.setVisible(8, true);
  hit = picker.pickRay(ray); picker.toggle(hit.source, hit.id);
  check(picker.count === 0, 'Second click did not deselect expired cell');
  picker.toggle(hit.source, idAt(7));
  const newGrid = selectionGrid(translated, selected());
  boxes.update(newGrid, rootBounds(translated), 1);
  picker.updateObject({ id: 8, visible: true, boxes }, newGrid);
  check(picker.count > 1 && volume(translated, hit.source.selected) === 1, 'Header update did not remap persistent selection');
  const command = picker.commands()[0];
  check(command.cmd === 'gridCellSelection' && command.module === 'octGrid' && command.vPorigin.join() === '0.25,0.25,0.25' &&
    command.vRootCellSize.join() === '2,2,2' && command.cellIDs.every(x => /^[0-9a-f]{32}$/.test(x)), 'Wrong command payload');
  check(command.vPorigin.every(x => typeof x === 'string') && !('color' in command), 'Non-text header or color sent');
  picker.updateObject({ id: 9, visible: true, boxes }, empty);
  picker.toggle(picker.sourceFor(9), root);
  check(picker.commands().length === 2, 'Different grids lost their own headers');
  picker.clear();
  check(picker.count === 0 && picker.commands().length === 0 && hit.source.overlay.geometry.instanceCount === 0, 'Clear left selections');
  boxes.geometry.dispose(); boxes.material.dispose(); viewer.dispose(); container.remove();
  return 'PASS: picker depth priority, exact IDs, persistence, toggling, volume remapping, multiple grids and clearing';
}

// Capture the live viewer for input/UI tests without exposing application globals.
export async function preparePickerUI() {
  const render = Viewer3D.prototype.render;
  const viewer = await new Promise(resolve => {
    Viewer3D.prototype.render = function () { Viewer3D.prototype.render = render; resolve(this); return render.call(this); };
  });
  viewer.update = () => {}; // Hold a deterministic frame while stream ACKs continue.
  viewer.clear(); viewer.picker.clear();
  const header = { origin: [0, 0, 0], size: [2, 2, 2], maxLevel: 4 };
  const object = viewer.createObject(7);
  const grid = selectionGrid(header, selected(root, idAt(7), idAt(7, 7)));
  for (let at = 0; at < grid.cells.length; at += 19) { grid.cells[at + 16] = 0; grid.cells[at + 17] = 255; }
  object.boxes.update(grid, rootBounds(header), 1);
  viewer.picker.updateObject(object, grid);
  viewer.autoBound = false;
  viewer.axes.visible = false; viewer.grid.visible = false;
  viewer.camera = new THREE.OrthographicCamera(-1.5, 1.5, 1.5, -1.5, .01, 100);
  viewer.camera.position.set(0, 0, 5); viewer.camera.up.set(0, 1, 0); viewer.camera.lookAt(0, 0, 0);
  viewer.controls.dispose();
  viewer.controls = new OrbitControls(viewer.camera, viewer.renderer.domElement);
  viewer.controls.enableDamping = false;
  viewer.controls.update();
  const rect = viewer.renderer.domElement.getBoundingClientRect();
  window.pickerTestViewer = viewer;
  return { x: rect.left + rect.width * .75, y: rect.top + rect.height * .25, id: cellIDKey(idAt(7, 7)) };
}
