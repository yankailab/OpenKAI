import { CELL_BYTES, validateCellIDs } from './octreeCells.js';
export const MAX_FRAME_BYTES = 64 * 1024 * 1024;
const littleEndian = new Uint8Array(new Uint16Array([1]).buffer)[0] === 1;

// Validates the whole frame before returning views, so malformed frames never
// leave the renderer with a partially updated scene. Attributes are zero-copy.
export function decodeFrame(buffer) {
  if (!(buffer instanceof ArrayBuffer) || buffer.byteLength < 32 || buffer.byteLength > MAX_FRAME_BYTES)
    throw new Error('Invalid geometry frame length');
  const v = new DataView(buffer);
  const version = v.getUint32(4, true);
  if (v.getUint32(0, true) !== 0x31443357 || (version !== 1 && version !== 2))
    throw new Error('Unsupported geometry protocol');
  const count = v.getUint32(12, true);
  if (count > 1024 || v.getUint32(16, true) !== buffer.byteLength)
    throw new Error('Invalid geometry header');
  let at = 32;
  const ids = new Set(), objects = [];
  const floats = n => {
    const data = littleEndian ? new Float32Array(buffer, at, n) :
      Float32Array.from({ length: n }, (_, i) => v.getFloat32(at + i * 4, true));
    at += n * 4;
    return data;
  };
  const bytes = n => { const data = new Uint8Array(buffer, at, n); at += n; return data; };
  for (let i = 0; i < count; ++i) {
    if (at + 64 > buffer.byteLength) throw new Error('Truncated object header');
    const id = v.getUint32(at, true), nP = v.getUint32(at + 4, true), nL = v.getUint32(at + 8, true);
    const nC = version === 2 ? v.getUint32(at + 12, true) : 0;
    const flags = version === 2 ? v.getUint32(at + 48, true) : 0;
    const gridBytes = flags & 1 ? 40 + nC * CELL_BYTES : 0;
    const end = Math.ceil((at + 64 + nP * 16 + nL * 32 + gridBytes) / 4) * 4;
    if ((flags & ~1) || (nC && !flags) || ids.has(id) || end > buffer.byteLength)
      throw new Error('Invalid object counts');
    ids.add(id);
    const pointSize = v.getFloat32(at + 16, true), opacity = v.getFloat32(at + 20, true);
    const bounds = Array.from({ length: 6 }, (_, j) => v.getFloat32(at + 24 + j * 4, true));
    if (!Number.isFinite(pointSize) || pointSize <= 0 || !Number.isFinite(opacity) || opacity < 0 || opacity > 1 ||
        !bounds.every(Number.isFinite) || bounds.some((x, j) => j < 3 && x > bounds[j + 3]))
      throw new Error('Invalid object style/bounds');
    at += 64;
    const object = { id, nP, nL, nC, pointSize, opacity, bounds,
      points: floats(nP * 3), pointColors: bytes(nP * 4), lines: floats(nL * 6), lineColors: bytes(nL * 8), grid: null };
    if (flags & 1) {
      const origin = Array.from(floats(3)), size = Array.from(floats(3));
      const maxLevel = v.getUint32(at, true);
      const timestamp = [v.getUint32(at + 8, true), v.getUint32(at + 12, true)];
      if (!origin.every(Number.isFinite) || !size.every(x => Number.isFinite(x) && x > 0) || maxLevel > 40)
        throw new Error('Invalid grid header');
      at += 16;
      const cells = bytes(nC * CELL_BYTES);
      validateCellIDs(cells, maxLevel);
      object.grid = { origin, size, maxLevel, timestamp, cells };
    }
    while (at < end) if (v.getUint8(at++) !== 0) throw new Error('Invalid grid padding');
    objects.push(object);
  }
  if (at !== buffer.byteLength) throw new Error('Unexpected trailing geometry bytes');
  return { sequence: v.getUint32(8, true), objects, bytes: buffer.byteLength };
}
