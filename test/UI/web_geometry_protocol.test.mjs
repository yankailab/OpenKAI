import assert from 'node:assert/strict';
import test from 'node:test';
import { readFile } from 'node:fs/promises';
import { runInNewContext } from 'node:vm';
import { TransformControls } from '../../html/viewer/_SelectableOctGrid/js/transformControls.js';

const sources = [
  ['_SelectableOctGrid', '../../html/viewer/_SelectableOctGrid/js/protocol.js'],
  ['shared geometry viewers', '../../html/common/js/geometryProtocol.js'],
];
const protocols = await Promise.all(sources.map(async ([name, path]) => ({
  name, ...await import(path),
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

const transformHTML = await readFile(new URL('../../html/viewer/_SelectableOctGrid/index.html', import.meta.url), 'utf8');
const commandHandler = await readFile(new URL('../../html/viewer/_SelectableOctGrid/js/wsCmdHandler.js', import.meta.url), 'utf8');

function transformUI(t) {
  const elements = new Map();
  for (const id of ['config', 'module', 'fields', 'load', 'save', 'status', 'x', 'y', 'z', 'roll', 'pitch', 'yaw']) {
    elements.set(`#transform-${id}`, {
      value: '0', disabled: false, textContent: '', validityMessage: '',
      get valueAsNumber() { return String(this.value).trim() ? Number(this.value) : NaN; },
      setCustomValidity(message) { this.validityMessage = message; },
    });
  }
  const $ = id => elements.get(`#transform-${id}`);
  $('module').value = transformHTML.match(/id="transform-module" value="([^"]+)"/)[1];
  $('config').reportValidity = () => [...elements.values()].every(element => !element.validityMessage);
  const listeners = new Map(), requests = [], timers = new Map();
  let timerID = 0;
  const window = {
    wsSocket: null,
    addEventListener(type, listener) { listeners.set(type, listener); },
    removeEventListener(type) { listeners.delete(type); },
    dispatchEvent(event) { listeners.get(event.type)?.(event); },
    wsSendCmd(command) {
      if (this.wsSocket?.readyState !== 1) return false;
      requests.push(command); return true;
    },
  };
  const replacements = {
    window, document: { querySelector: selector => elements.get(selector) }, WebSocket: { OPEN: 1 },
    setTimeout: callback => { const id = ++timerID; timers.set(id, callback); return id; },
    clearTimeout: id => timers.delete(id),
  };
  const original = new Map(Object.keys(replacements).map(key => [key, Object.getOwnPropertyDescriptor(globalThis, key)]));
  for (const [key, value] of Object.entries(replacements)) Object.defineProperty(globalThis, key, { configurable: true, writable: true, value });
  const controls = new TransformControls();
  t.after(() => {
    controls.dispose();
    for (const [key, descriptor] of original) {
      if (descriptor) Object.defineProperty(globalThis, key, descriptor);
      else delete globalThis[key];
    }
  });
  const context = {
    window, strEOJ: 'EOJ', wsCmdLog() {},
    CustomEvent: class { constructor(type, { detail }) { this.type = type; this.detail = detail; } },
  };
  runInNewContext(commandHandler, context);
  return {
    $, controls, requests,
    connect() { window.wsSocket = { readyState: 1 }; window.dispatchEvent({ type: 'wscmdstatechange' }); },
    disconnect() { window.wsSocket = null; window.dispatchEvent({ type: 'wscmdstatechange' }); },
    expire() { for (const callback of [...timers.values()]) callback(); },
    reply(config = { vT: [1, 2, 3], vAngle: [0, Math.PI / 2, -Math.PI] }, overrides = {}) {
      const reply = { ...requests.at(-1), bSuccess: true, config, ...overrides };
      const text = JSON.stringify(reply) + 'EOJ';
      // Exercise the existing fragmented command framing and page handler together.
      context.cmdHandler({ data: text.slice(0, 11) });
      context.cmdHandler({ data: text.slice(11) });
    },
    submit() { $('config').onsubmit({ preventDefault() {}, currentTarget: $('config') }); },
  };
}

test('transform controls load degrees and save only after a matched apply reply', t => {
  const ui = transformUI(t);
  assert.equal(ui.$('module').value, 'scepterTransform');
  assert.equal(ui.$('save').disabled, true);
  ui.connect();
  assert.equal(ui.requests[0].cmd, 'getConfig');
  for (const mismatch of [{ module: 'anotherTransform' }, { cmd: 'setConfig' }, { requestId: 'old-request' }]) {
    ui.reply(undefined, mismatch);
    assert.equal(ui.$('save').disabled, true);
    assert.equal(ui.requests.length, 1);
  }
  ui.reply();
  assert.equal(Number(ui.$('x').value), 1);
  assert.equal(Number(ui.$('pitch').value), 90);
  assert.equal(Number(ui.$('yaw').value), -180);
  assert.equal(ui.$('save').disabled, false);
  ui.$('x').value = '1.25';
  ui.$('roll').value = '45';
  ui.submit();
  assert.deepEqual(ui.requests.at(-1).config, { vT: [1.25, 2, 3], vAngle: [Math.PI / 4, Math.PI / 2, -Math.PI] });
  assert.equal(ui.requests.at(-1).cmd, 'setConfig');
  assert.equal(ui.$('save').disabled, true);
  const applied = ui.requests.at(-1).config;
  ui.reply(applied);
  assert.equal(ui.requests.at(-1).cmd, 'saveConfig');
  assert.equal(ui.requests.at(-1).config, undefined);
  assert.equal(ui.requests.length, 3);
  ui.reply(applied);
  assert.match(ui.$('status').textContent, /Saved scepterTransform to JSON/);
  assert.equal(ui.$('save').disabled, false);
});

test('transform controls reject invalid values and never save a rejected apply', t => {
  const ui = transformUI(t);
  ui.connect(); ui.reply();
  ui.$('x').value = '';
  ui.submit();
  assert.equal(ui.requests.length, 1);
  assert.match(ui.$('x').validityMessage, /finite/);
  ui.$('x').value = '4';
  ui.submit();
  ui.reply(undefined, { bSuccess: false, error: 'Invalid transform' });
  assert.equal(ui.requests.length, 2);
  assert.equal(ui.requests.at(-1).cmd, 'setConfig');
  assert.match(ui.$('status').textContent, /Invalid transform/);
  assert.equal(ui.$('x').value, '4');
});

test('transform controls distinguish applied parameters from failed persistence', t => {
  const ui = transformUI(t);
  ui.connect(); ui.reply(); ui.submit(); ui.reply();
  assert.equal(ui.requests.at(-1).cmd, 'saveConfig');
  ui.reply(undefined, { bSuccess: false, error: 'JSON file is read-only' });
  assert.match(ui.$('status').textContent, /Transform applied; save failed: JSON file is read-only/);
  assert.equal(ui.$('save').disabled, false);
  ui.submit(); ui.reply(); ui.expire();
  assert.match(ui.$('status').textContent, /Transform applied; saving was not confirmed/);
  assert.equal(ui.$('save').disabled, true);
});

test('transform controls recover from missing modules, target changes, and reconnects', t => {
  const ui = transformUI(t);
  ui.connect();
  const oldRequest = { ...ui.requests.at(-1) };
  ui.expire();
  assert.match(ui.$('status').textContent, /No reply from scepterTransform/);
  assert.equal(ui.$('module').disabled, false);
  ui.$('module').value = 'otherTransform';
  ui.$('module').oninput(); ui.$('module').onchange();
  assert.equal(ui.requests.at(-1).module, 'otherTransform');
  ui.reply(undefined, oldRequest);
  assert.equal(ui.$('save').disabled, true);
  ui.reply();
  assert.equal(ui.$('save').disabled, false);
  ui.submit(); ui.reply(); ui.disconnect();
  assert.match(ui.$('status').textContent, /Transform applied; saving was not confirmed/);
  assert.equal(ui.$('save').disabled, true);
  ui.connect();
  assert.equal(ui.requests.at(-1).cmd, 'getConfig');
  ui.reply();
  assert.equal(ui.$('save').disabled, false);
});

test('transform controls reject malformed backend configurations', t => {
  const ui = transformUI(t);
  ui.connect();
  ui.reply({ vT: [1, 2], vAngle: [0, 0, 0] });
  assert.match(ui.$('status').textContent, /Invalid transform reply/);
  assert.equal(ui.$('save').disabled, true);
  ui.$('load').onclick(); ui.reply();
  assert.equal(ui.$('save').disabled, false);
});
