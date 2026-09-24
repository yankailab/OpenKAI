// Dedicated GLIM submap protocol. Geometry point/line frames are not accepted.
export const PROTOCOL_VERSION = 1;
export const PROTOCOL_NAME = 'openkai.glim';
export const MAX_SUBMAP_POINTS = 10000000;
export const MAX_CHUNK_POINTS = 65536;
const littleEndian = new Uint8Array(new Uint16Array([1]).buffer)[0] === 1;
const id = value => typeof value === 'string' && /^(0|[1-9]\d{0,19})$/.test(value) && BigInt(value) <= 0xffffffffffffffffn;
const pose = value => Array.isArray(value) && value.length === 16 && value.every(Number.isFinite) &&
  Math.abs(value[3]) < 1e-9 && Math.abs(value[7]) < 1e-9 && Math.abs(value[11]) < 1e-9 && Math.abs(value[15] - 1) < 1e-9;
export function decodeEvent(text) {
  if (text.length > 16384) throw new Error('GLIM metadata exceeds size limit');
  const event = JSON.parse(text);
  if (!['reset', 'submap', 'pose'].includes(event.type) || !id(event.session) || !id(event.revision))
    throw new Error('Invalid GLIM event');
  if (event.type !== 'reset' && (!id(event.id) || !pose(event.pose))) throw new Error('Invalid submap pose');
  if (event.type === 'submap' && (!id(event.timestampUs) || !Number.isInteger(event.pointCount) ||
      event.pointCount < 0 || event.pointCount > MAX_SUBMAP_POINTS)) throw new Error('Invalid submap size');
  return { ...event, bytes: text.length };
}
export function decodeChunk(buffer) {
  if (!(buffer instanceof ArrayBuffer) || buffer.byteLength < 56 || buffer.byteLength > 56 + MAX_CHUNK_POINTS * 12)
    throw new Error('Invalid GLIM chunk length');
  const v = new DataView(buffer);
  if (v.getUint32(0, true) !== 0x314d4c47 || v.getUint32(4, true) !== PROTOCOL_VERSION ||
      v.getUint32(8, true) !== 1 || v.getUint32(12, true) !== 56 || v.getUint32(52, true) !== 0)
    throw new Error('Unsupported GLIM chunk');
  const totalPoints = v.getUint32(40, true), offsetPoints = v.getUint32(44, true), countPoints = v.getUint32(48, true);
  if (totalPoints > MAX_SUBMAP_POINTS || !countPoints || countPoints > MAX_CHUNK_POINTS ||
      offsetPoints + countPoints > totalPoints || buffer.byteLength !== 56 + countPoints * 12)
    throw new Error('Invalid GLIM point counts');
  const positions = littleEndian ? new Float32Array(buffer, 56) :
    Float32Array.from({ length: countPoints * 3 }, (_, i) => v.getFloat32(56 + i * 4, true));
  if (!positions.every(Number.isFinite)) throw new Error('Non-finite submap point');
  return { type: 'chunk', session: v.getBigUint64(16, true).toString(), id: v.getBigUint64(24, true).toString(),
    timestampUs: v.getBigUint64(32, true).toString(), totalPoints, offsetPoints, countPoints, positions, bytes: buffer.byteLength };
}
