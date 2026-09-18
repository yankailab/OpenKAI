import { CELL_BYTES, validateCellIDs } from './octreeCells.js';
export const PROTOCOL_VERSION = 4;
export const STREAM_TYPES = ['points', 'lines', 'cells'];
export const MAX_FRAME_BYTES = 64 * 1024 * 1024;
const littleEndian = new Uint8Array(new Uint16Array([1]).buffer)[0] === 1;

// One complete snapshot of one geometry type. Validate before returning any
// zero-copy views; other streams are independent of this frame's lifetime.
export function decodeFrame(buffer, expectedType) {
  if (!(buffer instanceof ArrayBuffer) || buffer.byteLength < 32 || buffer.byteLength > MAX_FRAME_BYTES)
    throw new Error('Invalid geometry frame length');
  const v = new DataView(buffer);
  if (v.getUint32(0, true) !== 0x34443357 || v.getUint32(4, true) !== PROTOCOL_VERSION)
    throw new Error('Unsupported geometry protocol');
  const type = STREAM_TYPES[v.getUint32(8, true) - 1];
  if (!type || (expectedType && type !== expectedType)) throw new Error('Wrong geometry stream type');
  const objectCount = v.getUint32(16, true);
  if (objectCount > 1024 || v.getUint32(20, true) !== buffer.byteLength) throw new Error('Invalid geometry header');
  let at = 32;
  const ids = new Set(), objects = [];
  const floats = n => {
    const data = littleEndian ? new Float32Array(buffer, at, n) :
      Float32Array.from({ length: n }, (_, i) => v.getFloat32(at + i * 4, true));
    at += n * 4; return data;
  };
  const bytes = n => { const data = new Uint8Array(buffer, at, n); at += n; return data; };
  for (let i = 0; i < objectCount; ++i) {
    if (at + 40 > buffer.byteLength) throw new Error('Truncated object header');
    const id = v.getUint32(at, true), count = v.getUint32(at + 4, true);
    const payload = type === 'cells' ? 40 + count * CELL_BYTES : count * (type === 'points' ? 16 : 32);
    if (ids.has(id) || at + 40 + payload > buffer.byteLength) throw new Error('Invalid object counts');
    ids.add(id);
    const pointSize = v.getFloat32(at + 8, true), opacity = v.getFloat32(at + 12, true);
    const bounds = Array.from({ length: 6 }, (_, j) => v.getFloat32(at + 16 + j * 4, true));
    if (!Number.isFinite(pointSize) || pointSize <= 0 || !Number.isFinite(opacity) || opacity < 0 || opacity > 1 ||
        !bounds.every(Number.isFinite) || bounds.some((x, j) => j < 3 && x > bounds[j + 3]))
      throw new Error('Invalid object style/bounds');
    at += 40;
    const object = { id, count, pointSize, opacity, bounds };
    if (type === 'cells') {
      const origin = Array.from(floats(3)), size = Array.from(floats(3));
      const maxLevel = v.getUint32(at, true);
      const timestamp = [v.getUint32(at + 8, true), v.getUint32(at + 12, true)];
      if (!origin.every(Number.isFinite) || !size.every(x => Number.isFinite(x) && x > 0) || maxLevel > 40 || v.getUint32(at + 4, true))
        throw new Error('Invalid grid header');
      at += 16;
      const cells = bytes(count * CELL_BYTES);
      validateCellIDs(cells, maxLevel);
      object.grid = { origin, size, maxLevel, timestamp, cells };
    } else {
      const vertices = count * (type === 'lines' ? 2 : 1);
      object.positions = floats(vertices * 3);
      object.colors = bytes(vertices * 4);
    }
    objects.push(object);
  }
  if (at !== buffer.byteLength) throw new Error('Unexpected trailing geometry bytes');
  return { type, sequence: v.getUint32(12, true), objects, bytes: buffer.byteLength };
}
