import assert from 'node:assert/strict';
import test from 'node:test';
import { webcrypto } from 'node:crypto';
import { GridConfigControls } from '../../html/viewer/_SelectableOctGrid/js/gridConfigControls.js';
import { TransformControls } from '../../html/viewer/_SelectableOctGrid/js/transformControls.js';
import { GridPickerControls } from '../../html/viewer/_SelectableOctGrid/js/gridPickerControls.js';

const config = {
  vPorigin: ['1.25', '-2', '3'], vRootCellSize: ['8', '16', '32'],
  dTexpireCell: '18446744073709551615', dTexpirePCL: '9007199254740993',
  nMaxCells: '8333', nMaxLevel: '4', nPminBuild: '1',
};

function setup(t, modules = ['grid'], create = () => new GridConfigControls()) {
  const elements = new Map();
  const element = id => {
    if (!elements.has(id)) elements.set(id, {
      value: '', textContent: '', disabled: false, hidden: false, validityMessage: '', children: [],
      get valueAsNumber() { return this.value === '' ? NaN : Number(this.value); },
      setCustomValidity(message) { this.validityMessage = message; },
      replaceChildren(...children) { this.children = children; },
      reportValidity() { return [...elements.values()].every(input => !input.validityMessage); },
    });
    return elements.get(id);
  };
  const sent = [], timers = new Map();
  const window = new EventTarget();
  window.wsSocket = { readyState: 1 };
  window.wsSendCmd = command => { sent.push(command); return true; };
  const globals = {
    document: { querySelector: selector => element(selector.slice(1)), createElement: () => ({}) },
    window, WebSocket: { OPEN: 1 }, crypto: webcrypto,
    setTimeout: callback => { const id = {}; timers.set(id, callback); return id; },
    clearTimeout: id => timers.delete(id),
  };
  const original = Object.fromEntries(Object.keys(globals).map(key => [key, Object.getOwnPropertyDescriptor(globalThis, key)]));
  for (const [key, value] of Object.entries(globals)) Object.defineProperty(globalThis, key, { value, configurable: true });
  const controls = create(element);
  t.after(() => {
    controls.dispose();
    for (const [key, descriptor] of Object.entries(original)) {
      if (descriptor) Object.defineProperty(globalThis, key, descriptor);
      else delete globalThis[key];
    }
  });
  controls.setModules?.(modules);
  const reply = (fields = {}, command = sent.at(-1)) => controls.reply({ ...command, ...config, bSuccess: true, ...fields });
  const submit = () => element('grid-config').onsubmit({ preventDefault() {}, currentTarget: element('grid-config') });
  const state = () => window.dispatchEvent(new Event('wscmdstatechange'));
  return { controls, element, sent, reply, submit, window, state, timers };
}

test('Load reads every field, Apply updates runtime and Save preserves unapplied grid drafts', t => {
  const { controls, element, sent, reply, submit } = setup(t);
  assert.equal(sent[0].cmd, 'getGridConfig');
  assert.equal(sent[0].module, 'grid');
  assert.match(sent[0].requestId, /^grid-/);
  assert.equal(element('grid-config-apply').disabled, true);
  reply();
  assert.equal(element('grid-origin-x').value, '1.25');
  assert.equal(element('grid-size-z').value, '32');
  assert.equal(element('grid-dTexpireCell').value, '18446744073709551615');
  assert.equal(element('grid-dTexpirePCL').value, '9007199254740993');
  assert.equal(element('grid-config-apply').disabled, false);
  element('grid-origin-y').value = '-7.5';
  element('grid-nMaxLevel').value = '40';
  submit();
  const { cmd, module, requestId, ...submitted } = sent[1];
  assert.equal(cmd, 'setGridConfig');
  assert.equal(module, 'grid');
  assert.notEqual(requestId, sent[0].requestId);
  assert.deepEqual(submitted, { ...config, vPorigin: ['1.25', '-7.5', '3'], nMaxLevel: '40' });
  assert.equal(element('grid-config-load').disabled, true);
  reply(submitted);
  assert.equal(element('grid-config-status').textContent, 'Applied grid.');
  assert.equal(sent.length, 2, 'Apply must not send saveConfig');
  element('grid-nMaxCells').value = '999';
  element('grid-config-save').onclick();
  assert.deepEqual(Object.keys(sent[2]).sort(), ['cmd', 'module', 'requestId']);
  assert.equal(sent[2].cmd, 'saveConfig');
  controls.reply({ ...sent[2], bSuccess: true });
  assert.equal(element('grid-config-status').textContent, 'Saved grid to JSON.');
  assert.equal(element('grid-nMaxCells').value, '999', 'Save must preserve unsent edits');
  element('grid-config-load').onclick();
  reply({ nMaxCells: '10' });
  assert.equal(element('grid-nMaxCells').value, '10');
});

test('unrelated broadcasts, stale replies and other sources cannot overwrite the form', t => {
  const { controls, element, sent, reply } = setup(t, ['grid-a', 'grid-b']);
  assert.equal(element('grid-config-source-label').hidden, false);
  const first = sent[0];
  for (const fields of [{ requestId: 'another-tab' }, { module: 'grid-b' }, { cmd: 'setGridConfig' }]) {
    reply(fields);
    assert.equal(controls.loaded, false);
    assert.equal(controls.pending.requestId, first.requestId);
  }
  reply();
  element('grid-config-source').value = 'grid-b';
  element('grid-config-source').onchange();
  assert.equal(sent[1].module, 'grid-b');
  reply({ nMaxCells: '999' }, first);
  assert.equal(element('grid-nMaxCells').value, '8333');
  assert.equal(controls.loaded, false);
  reply({ nMaxCells: '25' });
  assert.equal(element('grid-nMaxCells').value, '25');
  element('grid-nMaxCells').value = '26';
  reply({ nMaxCells: '999' });
  assert.equal(element('grid-nMaxCells').value, '26', 'unsolicited replies preserve edits');
});

test('integer validation keeps uint64 precision and enforces all scalar bounds', t => {
  const { element, sent, reply, submit } = setup(t);
  reply();
  for (const [key, invalid] of [
    ['dTexpireCell', '18446744073709551616'], ['dTexpirePCL', '-1'], ['dTexpirePCL', '1e9'],
    ['nMaxCells', '2147483648'], ['nMaxLevel', '41'], ['nPminBuild', '2147483648'], ['nPminBuild', '1.5'],
  ]) {
    const input = element(`grid-${key}`), before = input.value;
    input.value = invalid;
    submit();
    assert.equal(sent.length, 1, `${key}=${invalid} must not be sent`);
    assert.match(input.validityMessage, /whole number/);
    input.value = before;
  }
  for (const key of ['dTexpireCell', 'dTexpirePCL', 'nMaxCells', 'nMaxLevel', 'nPminBuild']) element(`grid-${key}`).value = '0';
  submit();
  assert.equal(sent.length, 2);
  for (const key of ['dTexpireCell', 'dTexpirePCL', 'nMaxCells', 'nMaxLevel', 'nPminBuild']) assert.equal(sent[1][key], '0');
});

test('invalid float coordinates and nonpositive sizes are blocked before sending', t => {
  const { element, sent, reply, submit } = setup(t);
  reply();
  for (const [id, value] of [
    ['grid-origin-x', 'Infinity'], ['grid-origin-x', '+1'], ['grid-origin-x', '0x10'],
    ['grid-origin-x', '1e39'], ['grid-origin-x', '1e-50'], ['grid-origin-x', '1e-400'],
    ['grid-size-z', '0'], ['grid-size-z', '-1'],
  ]) {
    const input = element(id), before = input.value;
    input.value = value;
    submit();
    assert.equal(sent.length, 1, `${id}=${value} must not be sent`);
    assert.notEqual(input.validityMessage, '');
    input.value = before;
  }
  submit();
  assert.equal(sent.length, 2);
});

test('old or incomplete replies cannot partially load config', t => {
  const { controls, element, reply } = setup(t);
  reply({ dTexpirePCL: undefined });
  assert.equal(controls.loaded, false);
  assert.equal(element('grid-config-apply').disabled, true);
  assert.match(element('grid-config-status').textContent, /Invalid/);
  element('grid-config-load').onclick();
  reply({ nMaxLevel: 4 });
  assert.equal(controls.loaded, false, 'numeric scalar fields from older protocols are rejected');
  element('grid-config-load').onclick();
  reply();
  assert.equal(controls.loaded, true);
});

test('save errors preserve drafts and apply timeouts or disconnects require a fresh Load', t => {
  const { controls, element, sent, reply, submit, window, state, timers } = setup(t);
  reply();
  element('grid-nMaxCells').value = '999';
  element('grid-config-save').onclick();
  reply({ bSuccess: false, error: 'Cannot write JSON' });
  assert.equal(controls.loaded, true);
  assert.equal(element('grid-nMaxCells').value, '999');
  assert.match(element('grid-config-status').textContent, /Saving grid config failed.*Cannot write JSON/);
  submit();
  [...timers.values()][0]();
  assert.equal(controls.pending, null);
  assert.equal(controls.loaded, false);
  assert.match(element('grid-config-status').textContent, /not confirmed/);
  element('grid-config-load').onclick();
  const stale = sent.at(-1);
  window.wsSocket = null; state();
  assert.equal(element('grid-config-load').disabled, true);
  assert.equal(controls.loaded, false);
  window.wsSocket = { readyState: 1 }; state();
  assert.equal(sent.at(-1).cmd, 'getGridConfig');
  assert.notEqual(sent.at(-1).requestId, stale.requestId);
  reply({}, stale);
  assert.equal(controls.loaded, false);
  reply();
  assert.equal(controls.loaded, true);
});

test('transform Apply and Save are separate and Save replies do not overwrite drafts', t => {
  const { controls, element, sent } = setup(t, [], element => {
    element('transform-module').value = 'transform';
    return new TransformControls();
  });
  const transform = { vT: [1, 2, 3], vAngle: [0, Math.PI / 2, 0] };
  const reply = (fields = {}) => controls.reply({ ...sent.at(-1), bSuccess: true, config: transform, ...fields });
  assert.equal(sent[0].cmd, 'getConfig');
  reply();
  element('transform-x').value = '8';
  element('transform-config').onsubmit({ preventDefault() {}, currentTarget: element('transform-config') });
  assert.equal(sent[1].cmd, 'setConfig');
  assert.deepEqual(sent[1].config.vT, [8, 2, 3]);
  reply({ config: { ...transform, vT: [8, 2, 3] } });
  assert.equal(sent.length, 2, 'Apply must not save automatically');
  assert.equal(element('transform-status').textContent, 'Applied transform.');
  element('transform-x').value = '99';
  element('transform-save').onclick();
  assert.equal(sent[2].cmd, 'saveConfig');
  assert.equal('config' in sent[2], false);
  reply({ config: undefined });
  assert.equal(element('transform-x').value, '99');
  assert.equal(element('transform-status').textContent, 'Saved transform to JSON.');
});

function pickerWithSources(modules) {
  const selections = new Map(modules.map(module => [module, ['selected-cell']]));
  return {
    selections, editable: true,
    get count() { return [...selections.values()].reduce((sum, ids) => sum + ids.length, 0); },
    gridModules() { return [...selections.keys()]; },
    commands() { return [...selections].map(([module, cellIDs]) => ({ cmd: 'setCellSelection', module,
      vPorigin: ['0', '0', '0'], vRootCellSize: ['8', '8', '8'], nMaxLevel: 4, cellIDs: [...cellIDs] })); },
    clear() { for (const module of selections.keys()) selections.set(module, []); },
    replaceSelection(reply) {
      if (!Array.isArray(reply.cellIDs)) throw new Error('Invalid selection data');
      selections.set(reply.module, [...reply.cellIDs]);
    },
  };
}

test('picker Load replaces drafts, Apply waits for every source, and Save leaves drafts intact', t => {
  const picker = pickerWithSources(['a', 'b']);
  const { controls, element, sent } = setup(t, [], () => new GridPickerControls(picker));
  const reply = (command, cellIDs, fields = {}) => controls.reply({ ...command, bSuccess: true, cellIDs, ...fields });
  element('picker-load').onclick();
  assert.deepEqual(sent.map(command => command.cmd), ['getCellSelection', 'getCellSelection']);
  reply(sent[0], ['foreign'], { requestId: 'other-tab' });
  assert.deepEqual(picker.selections.get('a'), ['selected-cell']);
  reply(sent[1], []);
  assert.equal(element('picker-apply').disabled, true, 'all source acknowledgements are required');
  reply(sent[0], ['running-cell']);
  assert.deepEqual(picker.selections.get('a'), ['running-cell']);
  assert.deepEqual(picker.selections.get('b'), []);
  assert.equal(element('picker-status').textContent, 'Loaded selection for 2 grids.');
  element('picker-clear').onclick();
  element('picker-apply').onclick();
  assert.deepEqual(sent.slice(2).map(command => command.cellIDs), [[], []]);
  assert.deepEqual(sent.slice(2).map(command => command.nMaxLevel), [4, 4]);
  assert.equal(picker.editable, false);
  reply(sent[2], []);
  assert.match(element('picker-status').textContent, /Applying/);
  reply(sent[3], []);
  assert.equal(element('picker-status').textContent, 'Applied selection for 2 grids.');
  assert.equal(sent.length, 4, 'Apply must not automatically save');
  picker.selections.set('a', ['unsent-draft']);
  element('picker-save').onclick();
  for (const command of sent.slice(4)) {
    assert.equal(command.cmd, 'saveConfig');
    assert.equal('cellIDs' in command, false);
    reply(command, undefined);
  }
  assert.deepEqual(picker.selections.get('a'), ['unsent-draft']);
  assert.equal(element('picker-status').textContent, 'Saved 2 grids to JSON.');
});

test('picker reports rejected requests, missing acknowledgements and disconnects', t => {
  const picker = pickerWithSources(['a', 'b']);
  const { controls, element, sent, timers, window, state } = setup(t, [], () => new GridPickerControls(picker));
  element('picker-apply').onclick();
  controls.reply({ ...sent[0], bSuccess: false, error: 'Stale grid header' });
  controls.reply({ ...sent[1], bSuccess: true, cellIDs: [] });
  assert.match(element('picker-status').textContent, /Applied selection for 1 grid.*Failed: a: Stale grid header/);
  assert.deepEqual(picker.selections.get('a'), ['selected-cell']);
  element('picker-save').onclick();
  controls.reply({ ...sent[2], bSuccess: true });
  [...timers.values()][0]();
  assert.match(element('picker-status').textContent, /Saved 1 grid.*b: no reply/);
  element('picker-load').onclick();
  const stale = sent.at(-1);
  window.wsSocket = null; state();
  assert.equal(controls.pending, null);
  assert.equal(element('picker-apply').disabled, true);
  controls.reply({ ...stale, bSuccess: true, cellIDs: ['stale'] });
  assert.deepEqual(picker.selections.get('b'), []);
  assert.match(element('picker-status').textContent, /not confirmed/);
});
