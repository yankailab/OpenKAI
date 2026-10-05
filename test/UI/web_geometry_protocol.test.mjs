import assert from 'node:assert/strict';
import test from 'node:test';

const viewers = ['_SelectableOctGrid', '_GeometryBase', '_Scepter', '_Orbbec', '_RealSense'];
const protocols = await Promise.all(viewers.map(async name => ({
  name, ...await import(`../../html/viewer/${name}/js/protocol.js`),
})));
const pointCount = 6_231_958; // nagano_sparse.ply exceeds the former 64 MiB frame limit.
const frameBytes = 72 + Math.ceil(pointCount * 15 / 4) * 4;

function pointHeader(buffer, count) {
  const view = new DataView(buffer);
  for (const [offset, value] of [
    [0, 0x36443357], [4, 6], [8, 1], [16, 1], [20, buffer.byteLength], [32, 7], [36, count],
  ]) view.setUint32(offset, value, true);
  view.setFloat32(40, 2, true);
  view.setFloat32(44, 1, true);
  for (const [axis, value] of [123.5, 234.5, 345.5].entries()) {
    view.setFloat32(60 + axis * 4, value, true);
  }
  return view;
}

test('all geometry viewers decode a complete large point cloud', () => {
  const buffer = new ArrayBuffer(frameBytes);
  const view = pointHeader(buffer, pointCount);
  const lastPosition = 72 + (pointCount - 1) * 12;
  const lastColor = 72 + pointCount * 12 + (pointCount - 1) * 3;
  for (const [axis, value] of [123.5, 234.5, 345.5].entries()) {
    view.setFloat32(lastPosition + axis * 4, value, true);
    view.setUint8(lastColor + axis, 230 + axis);
  }
  for (const { name, decodeFrame } of protocols) {
    const decoded = decodeFrame(buffer, 'points');
    assert.equal(decoded.bytes, 93_479_444, name);
    assert.equal(decoded.objects.length, 1, name);
    const object = decoded.objects[0];
    assert.equal(object.id, 7, name);
    assert.equal(object.count, pointCount, name);
    assert.equal(object.positions.length, pointCount * 3, name);
    assert.equal(object.colors.length, pointCount * 3, name);
    assert.deepEqual(Array.from(object.positions.subarray(-3)), [123.5, 234.5, 345.5], name);
    assert.deepEqual(Array.from(object.colors.subarray(-3)), [230, 231, 232], name);
  }

  view.setUint32(36, pointCount + 1, true);
  for (const { name, decodeFrame } of protocols) {
    assert.throws(() => decodeFrame(buffer, 'points'), /Invalid object counts/, name);
  }
});

test('large point-cloud declarations still require a complete payload', () => {
  const truncated = new ArrayBuffer(72);
  pointHeader(truncated, pointCount);
  for (const { name, decodeFrame } of protocols) {
    assert.throws(() => decodeFrame(truncated, 'points'), /Invalid object counts/, name);
  }
});

test('all geometry viewers retain the 256 MiB frame bound', () => {
  const limit = 256 * 1024 * 1024;
  const oversized = new ArrayBuffer(limit + 1);
  for (const { name, decodeFrame, MAX_FRAME_BYTES } of protocols) {
    assert.equal(MAX_FRAME_BYTES, limit, name);
    assert.throws(() => decodeFrame(oversized, 'points'), /Invalid geometry frame length/, name);
  }
});
