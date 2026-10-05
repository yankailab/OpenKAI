// Shared tests run in a real browser or with Node; no package installation needed.
export const testCases = [];
function test(name, run) { testCases.push({ name, run }); }
const assert = {
  ok(value, message = 'Expected truthy value') { if (!value) throw new Error(message); },
  equal(actual, expected) { if (actual !== expected) throw new Error(`${actual} !== ${expected}`); },
  deepEqual(actual, expected) { if (JSON.stringify(actual) !== JSON.stringify(expected)) throw new Error(`${JSON.stringify(actual)} != ${JSON.stringify(expected)}`); },
  throws(run) { let threw = false; try { run(); } catch { threw = true; } if (!threw) throw new Error('Expected an exception'); },
};
export function runTests() {
  return testCases.map(({ name, run }) => { run(); return { name, passed: true }; });
}

import { assetUrl, fresh, normalizeConfig, parseMessage, socketUrl, validPosition, WORLD_IMAGERY_URL } from '../js/protocol.js';
import { bodyToNed, modelToNed, modelToThree, resolveAltitude } from '../js/geo.js';
import { tileCoverage } from '../js/tileCoverage.js';

test('downloaded tile coverage preserves inclusive ranges and intentional holes', () => {
  const contains = tileCoverage({ format: 'openkai-tile-coverage/1', minimumLevel: 2, maximumLevel: 3,
    levels: { 2: { 1: [[0, 1], [3, 3]] }, 3: { 2: [[2, 5]] } } });
  for (const tile of [[1, 0, 2], [1, 1, 2], [1, 3, 2], [2, 2, 3], [2, 5, 3]]) assert.ok(contains(...tile));
  for (const tile of [[1, 2, 2], [0, 0, 2], [2, 6, 3], [2, 2, 4], [1, -1, 2], [1, 0.5, 2]])
    assert.equal(contains(...tile), false);
});

test('downloaded coverage rejects malformed levels and out-of-range or overlapping coordinates', () => {
  const valid = { format: 'openkai-tile-coverage/1', minimumLevel: 2, maximumLevel: 2, levels: { 2: { 1: [[0, 1]] } } };
  for (const change of [
    { format: 'other' }, { minimumLevel: -1 }, { maximumLevel: 24 }, { minimumLevel: 3 },
    { levels: {} }, { levels: { '02': {} } }, { levels: { 2: { 4: [[0, 1]] } } },
    { levels: { 2: { 1: [[-1, 1]] } } }, { levels: { 2: { 1: [[0, 4]] } } },
    { levels: { 2: { 1: [[0, 2], [2, 3]] } } }, { levels: { 2: { 1: [[2, 1]] } } },
    { levels: { 2: { 1: [[0.5, 1]] } } }, { levels: { 2: { 1: [[0, 1, 2]] } } },
  ]) assert.throws(() => tileCoverage({ ...valid, ...change }));
});

const envelope = { protocol: 'openkai.mavlink', version: 1 };
const attitude = (rollRad = 0, pitchRad = 0, yawRad = 0) => ({ valid: true, rollRad, pitchRad, yawRad });
const transform = (matrix, vector) => [0, 1, 2].map(row => vector.reduce((sum, value, col) => sum + matrix[row * 3 + col] * value, 0));
const near = (actual, expected) => actual.forEach((value, i) => assert.ok(Math.abs(value - expected[i]) < 1e-10, `${actual} != ${expected}`));
const position = { valid: true, latitudeDeg: 35.68, longitudeDeg: 139.76, altitudeMslM: 100, relativeAltitudeM: 7 };

test('only the dedicated versioned read-only JSON protocol is accepted', () => {
  assert.equal(parseMessage(JSON.stringify({ ...envelope, type: 'hello', readOnly: true, config: {} })).type, 'hello');
  assert.equal(parseMessage(JSON.stringify({ ...envelope, type: 'telemetry', sequence: 0, timeMs: 0, connected: false })).sequence, 0);
  for (const bad of [new ArrayBuffer(4), 'start', '{}', JSON.stringify({ ...envelope, version: 2 }), JSON.stringify({ ...envelope, type: 'hello', config: {} }), JSON.stringify({ ...envelope, type: 'command' })])
    assert.throws(() => parseMessage(bad));
});

test('freshness combines backend message age and local monotonic elapsed time', () => {
  assert.equal(fresh({ ageMs: 2900, stale: false }, 99), true);
  assert.equal(fresh({ ageMs: 2900, stale: false }, 101), false);
  assert.equal(fresh({ ageMs: 0, stale: true }), false);
  assert.equal(fresh({ ageMs: null, stale: false }), false);
  assert.equal(fresh(null), false);
});

test('coordinates distinguish valid zero coordinates from missing/invalid positions', () => {
  assert.equal(validPosition({ ...position, latitudeDeg: 0, longitudeDeg: 0, altitudeMslM: 0 }), true);
  assert.equal(validPosition({ ...position, latitudeDeg: 91 }), false);
  assert.equal(validPosition({ ...position, valid: false }), false);
  assert.equal(validPosition({ ...position, altitudeMslM: null }), false);
});

test('zero attitude points forward north, top up and right east', () => {
  const matrix = modelToNed(attitude());
  near(transform(matrix, [1, 0, 0]), [1, 0, 0]);
  near(transform(matrix, [0, 1, 0]), [0, 0, -1]);
  near(transform(matrix, [0, 0, 1]), [0, 1, 0]);
  near(transform(modelToThree(attitude()), [1, 0, 0]), [0, 0, -1]);
});

test('MAVLink yaw is clockwise north toward east; pitch raises nose; roll lowers right', () => {
  near(transform(bodyToNed(attitude(0, 0, Math.PI / 2)), [1, 0, 0]), [0, 1, 0]);
  near(transform(bodyToNed(attitude(0, Math.PI / 2, 0)), [1, 0, 0]), [0, 0, -1]);
  near(transform(bodyToNed(attitude(Math.PI / 2, 0, 0)), [0, 1, 0]), [0, 0, 1]);
});

test('body-to-NED quaternions match composed Euler rotations including nonzero roll and pitch', () => {
  const r = .3, p = -.2, y = 1.1;
  const cr = Math.cos(r/2), sr = Math.sin(r/2), cp = Math.cos(p/2), sp = Math.sin(p/2), cy = Math.cos(y/2), sy = Math.sin(y/2);
  const quaternionWxyz = [cr*cp*cy+sr*sp*sy, sr*cp*cy-cr*sp*sy, cr*sp*cy+sr*cp*sy, cr*cp*sy-sr*sp*cy];
  near(bodyToNed({ valid: true, quaternionWxyz }), bodyToNed(attitude(r, p, y)));
  near(bodyToNed({ valid: true, quaternionWxyz: quaternionWxyz.map(v => -2*v) }), bodyToNed(attitude(r, p, y)));
  assert.equal(bodyToNed({ valid: true, quaternionWxyz: [0, 0, 0, 0] }), null);
  assert.equal(bodyToNed({ ...attitude(), valid: false }), null);
});

test('ellipsoid altitude uses geoid separation and never relative altitude', () => {
  const config = normalizeConfig({ altitude: { geoidSeparationM: 39.2 } });
  assert.equal(resolveAltitude(position, null, config).heightM, 139.2);
  const gps = { ...position, altitudeMslM: 99, altitudeEllipsoidM: 139, ageMs: 0, stale: false };
  const auto = normalizeConfig();
  assert.equal(resolveAltitude(position, gps, auto).heightM, 140);
  assert.equal(resolveAltitude(position, { ...gps, stale: true }, auto).approximate, true);
  assert.equal(resolveAltitude(position, { ...gps, latitudeDeg: 0 }, auto).approximate, true);
  assert.equal(resolveAltitude(position, null, auto).heightM, 100);
  assert.equal(resolveAltitude(position, gps, auto, 4000).approximate, true);
  assert.equal(resolveAltitude(position, gps, normalizeConfig({ altitude: { geoidSeparationM: 0 } })).heightM, 100);
});

test('endpoints preserve secure transport, IPv6 and URL-template tokens', () => {
  assert.equal(socketUrl('https://[::1]:8443/viewer/'), 'wss://[::1]:8443/stream/mavlink');
  assert.equal(assetUrl('/models/{z}/{x}/{y}.png', 'http://localhost:8080/'), 'http://localhost:8080/models/{z}/{x}/{y}.png');
  assert.throws(() => assetUrl('javascript:alert(1)', 'http://localhost/'));
  assert.throws(() => assetUrl('https://user:password@example.com/a', 'http://localhost/'));
  assert.equal(normalizeConfig({ staleAfterMs: 3600000 }).staleAfterMs, 3600000);
  assert.equal(normalizeConfig({ trailMaxPoints: 100000 }).trailMaxPoints, 20000);
});

test('online maps require explicit enablement and a supported provider', () => {
  for (const onlineImagery of [undefined, null, [], true, { enabled: 'true' }, { enabled: true, provider: 'unknown' }])
    assert.equal(normalizeConfig({ onlineImagery }).onlineImagery.enabled, false);
  const enabled = normalizeConfig({ onlineImagery: { enabled: true } }).onlineImagery;
  assert.equal(enabled.enabled, true);
  assert.equal(enabled.provider, 'arcgis');
  assert.equal(enabled.url, WORLD_IMAGERY_URL);
  const custom = normalizeConfig({ onlineImagery: { enabled: true, provider: 'xyz', url: ' https://example.test/{z}/{x}/{y}.png ', credit: 'Example' } }).onlineImagery;
  assert.equal(custom.url, 'https://example.test/{z}/{x}/{y}.png');
  assert.equal(custom.credit, 'Example');
  assert.equal(normalizeConfig({ onlineImagery: { enabled: true, provider: 'xyz' } }).onlineImagery.url, '');
});

test('online imagery levels remain bounded, ordered integers', () => {
  const oversized = normalizeConfig({ onlineImagery: { minimumLevel: 100, maximumLevel: 99 } }).onlineImagery;
  assert.equal(oversized.maximumLevel, 23);
  assert.equal(oversized.minimumLevel, 23);
  const invalid = normalizeConfig({ onlineImagery: { minimumLevel: -1, maximumLevel: NaN, credit: {} } }).onlineImagery;
  assert.equal(invalid.minimumLevel, 0);
  assert.equal(invalid.maximumLevel, 19);
  assert.equal(invalid.credit, '');
  const fractional = normalizeConfig({ onlineImagery: { minimumLevel: 8.5, maximumLevel: 4.5 } }).onlineImagery;
  assert.equal(fractional.minimumLevel, 4);
  assert.equal(fractional.maximumLevel, 4);
});

test('building detail defaults keep distant and peripheral tiles detailed while following', () => {
  assert.deepEqual(normalizeConfig().buildingRendering, {
    maximumScreenSpaceError: 2, dynamicScreenSpaceError: false, foveatedScreenSpaceError: false,
    cullRequestsWhileMoving: false, cacheMegabytes: 1024, maximumCacheOverflowMegabytes: 1024,
  });
  const settings = {
    maximumScreenSpaceError: 8, dynamicScreenSpaceError: true, foveatedScreenSpaceError: true,
    cullRequestsWhileMoving: true, cacheMegabytes: 256, maximumCacheOverflowMegabytes: 0,
  };
  assert.deepEqual(normalizeConfig({ buildingRendering: settings }).buildingRendering, settings);
});

test('building detail configuration bounds GPU budgets and rejects invalid numeric or boolean values', () => {
  assert.deepEqual(normalizeConfig({ buildingRendering: {
    maximumScreenSpaceError: -1, cacheMegabytes: 100000, maximumCacheOverflowMegabytes: -1,
    dynamicScreenSpaceError: 'false', foveatedScreenSpaceError: null, cullRequestsWhileMoving: 1,
  } }).buildingRendering, {
    maximumScreenSpaceError: 1, dynamicScreenSpaceError: false, foveatedScreenSpaceError: false,
    cullRequestsWhileMoving: false, cacheMegabytes: 4096, maximumCacheOverflowMegabytes: 0,
  });
  const invalid = normalizeConfig({ buildingRendering: { maximumScreenSpaceError: Infinity, cacheMegabytes: '256', maximumCacheOverflowMegabytes: NaN } });
  assert.equal(invalid.buildingRendering.maximumScreenSpaceError, 2);
  assert.equal(invalid.buildingRendering.cacheMegabytes, 1024);
  assert.equal(invalid.buildingRendering.maximumCacheOverflowMegabytes, 1024);
  assert.equal(normalizeConfig({ buildingRendering: { maximumScreenSpaceError: 100000, cacheMegabytes: 0, maximumCacheOverflowMegabytes: 100000 } }).buildingRendering.maximumScreenSpaceError, 64);
  assert.equal(normalizeConfig({ buildingRendering: { cacheMegabytes: 0 } }).buildingRendering.cacheMegabytes, 64);
  assert.equal(normalizeConfig({ buildingRendering: { maximumCacheOverflowMegabytes: 100000 } }).buildingRendering.maximumCacheOverflowMegabytes, 4096);
});

test('GPS dropouts retain the measured local geoid datum; moving away invalidates it', () => {
  const cache = {}, config = normalizeConfig();
  const gps = { ...position, altitudeMslM: 99, altitudeEllipsoidM: 139, ageMs: 0, stale: false };
  resolveAltitude(position, gps, config, 0, cache);
  const retained = resolveAltitude({ ...position, altitudeMslM: 110 }, null, config, 0, cache);
  assert.equal(retained.heightM, 150);
  assert.ok(retained.source.includes('cached'));
  assert.equal(resolveAltitude({ ...position, latitudeDeg: 36 }, null, config, 0, cache).approximate, true);
  assert.equal(resolveAltitude(position, null, normalizeConfig({ altitude: { geoidSeparationM: 30 } }), 0, cache).heightM, 130);
  assert.equal(resolveAltitude(position, null, config, 0, {}).approximate, true);
});

import { MavlinkConnection } from '../js/connection.js';

class FakeSocket {
  static sockets = [];
  constructor(url) { this.url = url; this.sent = []; FakeSocket.sockets.push(this); }
  send(value) { this.sent.push(value); }
  close(code) { this.closed = code || 1000; }
  message(value) { this.onmessage({ data: JSON.stringify(value) }); }
}
const base = { protocol: 'openkai.mavlink', version: 1 };
const hello = { ...base, type: 'hello', readOnly: true, config: {} };
const telemetry = sequence => ({ ...base, type: 'telemetry', sequence, timeMs: 1, connected: false });

test('stream sends no commands and rejects out-of-order telemetry', () => {
  const saved = globalThis.WebSocket;
  globalThis.WebSocket = FakeSocket;
  const received = [], states = [];
  const connection = new MavlinkConnection({ onState: state => states.push(state), onHello() {}, onReset() {}, onTelemetry: data => received.push(data.sequence) });
  try {
    connection.start('http://localhost:8080/');
    const ws = FakeSocket.sockets.at(-1);
    assert.equal(ws.url, 'ws://localhost:8080/stream/mavlink');
    ws.message(hello); ws.message(telemetry(1)); ws.message(telemetry(1)); ws.message(telemetry(0)); ws.message(telemetry(2));
    assert.deepEqual(received, [1, 2]);
    assert.deepEqual(ws.sent, []);
    connection.stop();
    ws.message(telemetry(3));
    ws.onerror();
    assert.deepEqual(received, [1, 2]);
    assert.equal(states.at(-1), 'stopped');
  } finally { connection.stop(); globalThis.WebSocket = saved; }
});

test('telemetry before hello and duplicate hellos close with protocol error', () => {
  const saved = globalThis.WebSocket;
  globalThis.WebSocket = FakeSocket;
  const connection = new MavlinkConnection({ onState() {}, onHello() {}, onReset() {}, onTelemetry() { throw new Error('Must not publish invalid telemetry'); } });
  try {
    connection.start('http://localhost/');
    const first = FakeSocket.sockets.at(-1); first.message(telemetry(0)); assert.equal(first.closed, 1002);
    connection.start('http://localhost/');
    const second = FakeSocket.sockets.at(-1); second.message(hello); second.message(hello); assert.equal(second.closed, 1002);
  } finally { connection.stop(); globalThis.WebSocket = saved; }
});
