import assert from 'node:assert/strict';
import test from 'node:test';
import * as THREE from '../../html/viewer/_SelectableOctGrid/vendor/three.module.min.js';
import { cellBox, CELL_BYTES, validateCellIDs } from '../../html/viewer/_SelectableOctGrid/js/octreeCells.js';
import { childID, cellIDKey, decodeSelection, remapSelection, rootBounds, selectionGrid } from '../../html/viewer/_SelectableOctGrid/js/gridSelection.js';
import { GridBoxes } from '../../html/viewer/_SelectableOctGrid/js/gridBoxes.js';
import { GridCellPicker } from '../../html/viewer/_SelectableOctGrid/js/gridCellPicker.js';
import { decodeFrame } from '../../html/viewer/_SelectableOctGrid/js/protocol.js';
import { decodeFrame as decodeGeometryFrame } from '../../html/common/js/geometryProtocol.js';

const header = { origin: [0, 0, 0], size: [8, 8, 8], maxLevel: 40 };
const ids = (...values) => new Map(values.map(id => [cellIDKey(id), id]));

// Build the 128-bit format independently from the viewer's byte operations.
function encodedPath(path) {
  let packed = BigInt(path.length);
  for (let level = 0; level < 40; ++level)
    packed = (packed << 3n) | BigInt(path[level] ?? 0);
  return Uint8Array.from({ length: 16 }, (_, byte) => Number((packed >> BigInt(byte * 8)) & 255n));
}

test('cell IDs put depth before all forty path indices, through byte and 64-bit boundaries', () => {
  let id = new Uint8Array(16);
  const path = [];
  for (let depth = 0; depth <= 40; ++depth) {
    assert.deepEqual(id, encodedPath(path));
    const grid = selectionGrid(header, ids(id));
    validateCellIDs(grid.cells, 40);
    const box = cellBox(grid, 0);
    const expected = [0, 1, 2].map(axis =>
      path.reduce((sum, child, level) => sum + ((child & (4 >> axis)) ? 1 : -1) * 2 ** (1 - level), 0));
    assert.deepEqual(box.center, expected);
    assert.deepEqual(box.size, [8, 8, 8].map(value => value / 2 ** depth));
    if (depth === 40) break;
    const child = (depth * 5 + 3) % 8;
    path.push(child);
    id = childID(id, child);
  }
  assert.throws(() => childID(id, 0), /Maximum grid depth/);
  assert.equal(cellIDKey(encodedPath([5, 2])), '0000000000000000000000000000a802');
  assert.equal(cellIDKey(encodedPath(Array(40).fill(7))), 'ffffffffffffffffffffffffffffff28');
});

test('cell ID validation checks reserved, depth and every unused path bit', () => {
  for (let depth = 0; depth <= 40; ++depth) {
    const id = encodedPath(Array(depth).fill(7));
    const { cells } = selectionGrid(header, ids(id));
    assert.doesNotThrow(() => validateCellIDs(cells, depth));
    if (depth > 0) assert.throws(() => validateCellIDs(cells, depth - 1), /Invalid grid cell ID/);
    for (let bit = 0; bit < 120 - depth * 3; ++bit) {
      const invalid = cells.slice();
      invalid[bit >> 3] |= 1 << (bit % 8);
      assert.throws(() => validateCellIDs(invalid, 40), /Noncanonical grid cell ID/);
    }
    for (const reserved of [64, 128]) {
      const invalid = cells.slice();
      invalid[15] |= reserved;
      assert.throws(() => validateCellIDs(invalid, 40), /Invalid grid cell ID/);
    }
  }
  const invalid = new Uint8Array(CELL_BYTES);
  invalid[15] = 41;
  assert.throws(() => validateCellIDs(invalid, 40), /Invalid grid cell ID/);
});

test('selection commands preserve byte-order hex and remap nested or deeper selections', () => {
  const command = { vPorigin: ['0', '0', '0'], vRootCellSize: ['8', '8', '8'], nMaxLevel: 40,
    cellIDs: ['0000000000000000000000000000a001', '0000000000000000000000000000a802'] };
  const selected = decodeSelection(command);
  assert.deepEqual([...selected.ids.values()], [encodedPath([5]), encodedPath([5, 2])]);
  const local = { origin: [2, -2, 2], size: [4, 4, 4], maxLevel: 40 };
  assert.deepEqual([...remapSelection(header, local, selected.ids).values()], [encodedPath([])]);
  assert.deepEqual([...remapSelection(header, { ...header, maxLevel: 1 }, ids(encodedPath([5, 2]))).values()],
    [encodedPath([5])]);
  assert.throws(() => decodeSelection({ ...command, cellIDs: ['01000000000000000000000000000000'] }),
    /Noncanonical grid cell ID/);
  const { nMaxLevel, ...missingDepth } = command;
  assert.throws(() => decodeSelection(missingDepth), /Invalid selection data/);
  assert.throws(() => decodeSelection({ ...command, nMaxLevel: null }), /Invalid selection data/);
});

test('grid level filtering, picking and loading use the depth byte at the end of the ID', t => {
  const grid = selectionGrid(header, ids(encodedPath([]), encodedPath([5]), encodedPath([5, 2])));
  const boxes = new GridBoxes();
  const picker = new GridCellPicker({
    renderer: { domElement: { addEventListener() {}, removeEventListener() {} } },
    scene: new THREE.Scene(),
  });
  t.after(() => { picker.dispose(); boxes.dispose(); });
  boxes.update(grid, rootBounds(header), 1);
  boxes.setLevelRange(1, 1);
  assert.equal(boxes.geometry.instanceCount, 1);
  assert.deepEqual(boxes.getCell(0).id, encodedPath([5]));
  boxes.setLevelRange(0, 40);
  picker.configure([{ id: 1, name: 'grid', selectableGrid: true }]);
  picker.updateObject({ id: 1, boxes, visible: true }, grid);
  const hit = picker.pickRay(new THREE.Ray(new THREE.Vector3(1, -1, 10), new THREE.Vector3(0, 0, -1)));
  assert.equal(hit.depth, 2);
  assert.deepEqual(hit.id, encodedPath([5, 2]));
  picker.updateObject({ id: 1, boxes, visible: true }, { ...grid, maxLevel: 1 });
  picker.mergeSelection({ module: 'grid', vPorigin: ['0', '0', '0'], vRootCellSize: ['8', '8', '8'],
    nMaxLevel: 40, cellIDs: [cellIDKey(encodedPath([5, 2]))] });
  assert.deepEqual([...picker.sourceFor(1).selected.values()], [encodedPath([5])]);
});

test('version 7 frames decode the new IDs and all viewers reject earlier protocols', () => {
  const buffer = new ArrayBuffer(112 + CELL_BYTES * 2), view = new DataView(buffer);
  for (const [offset, value] of [
    [0, 0x37443357], [4, 7], [8, 3], [16, 1], [20, buffer.byteLength], [32, 1], [36, 2], [96, 40],
  ]) view.setUint32(offset, value, true);
  view.setFloat32(40, 1, true);
  view.setFloat32(44, 1, true);
  for (let axis = 0; axis < 3; ++axis) view.setFloat32(84 + axis * 4, 8, true);
  const expected = selectionGrid(header, ids(encodedPath([5, 2]), encodedPath(Array(40).fill(7))));
  new Uint8Array(buffer, 112).set(expected.cells);
  const decoded = decodeFrame(buffer, 'cells');
  assert.deepEqual(decoded.objects[0].grid.cells, expected.cells);
  assert.deepEqual(cellBox(decoded.objects[0].grid, 0).center, [1, -1, 1]);
  const pointBuffer = buffer.slice(0, 72), pointView = new DataView(pointBuffer);
  pointView.setUint32(8, 1, true);
  pointView.setUint32(20, pointBuffer.byteLength, true);
  pointView.setUint32(36, 0, true);
  for (const [decode, frame, type] of [
    [decodeFrame, buffer, 'cells'], [decodeFrame, pointBuffer, 'points'], [decodeGeometryFrame, pointBuffer, 'points'],
  ]) {
    const frameView = new DataView(frame);
    assert.doesNotThrow(() => decode(frame, type));
    for (let version = 1; version < 7; ++version) {
      frameView.setUint32(4, version, true);
      assert.throws(() => decode(frame, type), /Unsupported geometry protocol/);
      frameView.setUint32(0, 0x30443357 + version * 2 ** 24, true);
      assert.throws(() => decode(frame, type), /Unsupported geometry protocol/);
      frameView.setUint32(4, 7, true);
      assert.throws(() => decode(frame, type), /Unsupported geometry protocol/);
      frameView.setUint32(0, 0x37443357, true);
    }
  }
});
