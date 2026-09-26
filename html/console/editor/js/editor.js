/* Standalone, dependency-free OpenKAI config editor. Classic scripts support file://. */
(function () {
  'use strict';
  const $ = id => document.getElementById(id);
  const clone = value => JSON.parse(JSON.stringify(value));
  const own = (object, key) => Object.prototype.hasOwnProperty.call(object, key);
  const pathLabel = path => path.map(String).join('.');
  const pathKey = path => JSON.stringify(path);
  const read = (object, path) => path.reduce((value, key) => value != null && own(value, key) ? value[key] : undefined, object);
  const initialConfig = () => ({ APP: { class: 'ModuleMgr', appName: 'MyOpenKAI', bLog: true, bStdErr: true } });
  const STORAGE_KEY = 'openkai-config-editor-v1';
  const state = { schema: null, model: null, positions: {}, selected: null, scale: 1, pan: { x: 35, y: 35 }, history: [], cursor: -1, pending: null, drag: null, pointer: { x: 0, y: 0 } };
  let saving = null;

  function element(tag, className, text) {
    const result = document.createElement(tag);
    if (className) result.className = className;
    if (text !== undefined) result.textContent = text;
    return result;
  }
  function button(text, action, className = 'small') {
    const result = element('button', className, text);
    result.type = 'button';
    result.addEventListener('click', action);
    return result;
  }
  function status(message, error = false) {
    $('status').textContent = message;
    $('status').classList.toggle('error', error);
  }
  function snapshot() {
    return { document: state.model.toJSON(), positions: clone(state.positions), selected: state.selected };
  }
  function saveDraft() {
    clearTimeout(saving);
    saving = setTimeout(() => {
      try {
        localStorage.setItem(STORAGE_KEY, JSON.stringify({ ...snapshot(), filename: $('filename').value, pan: state.pan, scale: state.scale }));
      } catch (_) { /* File-origin storage may be unavailable; editing/export still works. */ }
    }, 150);
  }
  function record() {
    state.history.splice(state.cursor + 1);
    state.history.push(snapshot());
    if (state.history.length > 60) state.history.shift();
    state.cursor = state.history.length - 1;
    updateHistory();
    saveDraft();
  }
  function updateHistory() {
    $('undo').disabled = state.cursor <= 0;
    $('redo').disabled = state.cursor >= state.history.length - 1;
  }
  function undo(direction) {
    const cursor = state.cursor + direction;
    if (cursor < 0 || cursor >= state.history.length) return;
    state.cursor = cursor;
    const entry = state.history[cursor];
    state.model.import(entry.document);
    state.positions = clone(entry.positions);
    state.selected = entry.selected;
    cancelConnection();
    render();
    updateHistory();
    saveDraft();
    status(direction < 0 ? 'Undid change.' : 'Redid change.');
  }
  function mutate(action, message) {
    try {
      action();
      record();
      render();
      if (message) status(message);
      return true;
    } catch (error) {
      status(error.message, true);
      return false;
    }
  }
  function worldPoint(clientX, clientY) {
    const rect = $('canvas').getBoundingClientRect();
    return { x: (clientX - rect.left - state.pan.x) / state.scale, y: (clientY - rect.top - state.pan.y) / state.scale };
  }
  function setTransform() {
    $('world').style.transform = `translate(${state.pan.x}px, ${state.pan.y}px) scale(${state.scale})`;
    $('zoom-level').value = `${Math.round(state.scale * 100)}%`;
  }
  function zoom(factor, point) {
    const rect = $('canvas').getBoundingClientRect();
    const center = point || { x: rect.left + rect.width / 2, y: rect.top + rect.height / 2 };
    const before = worldPoint(center.x, center.y);
    state.scale = Math.max(.2, Math.min(2, state.scale * factor));
    state.pan.x = center.x - rect.left - before.x * state.scale;
    state.pan.y = center.y - rect.top - before.y * state.scale;
    setTransform();
    saveDraft();
  }
  function fit() {
    const nodes = [...$('nodes').children];
    if (!nodes.length) return;
    const minX = Math.min(...nodes.map(node => state.positions[node.dataset.id].x));
    const minY = Math.min(...nodes.map(node => state.positions[node.dataset.id].y));
    const maxX = Math.max(...nodes.map(node => state.positions[node.dataset.id].x + node.offsetWidth));
    const maxY = Math.max(...nodes.map(node => state.positions[node.dataset.id].y + node.offsetHeight));
    state.scale = Math.max(.2, Math.min(1.15, ($('canvas').clientWidth - 70) / Math.max(1, maxX - minX), ($('canvas').clientHeight - 70) / Math.max(1, maxY - minY)));
    state.pan = { x: ($('canvas').clientWidth - (maxX - minX) * state.scale) / 2 - minX * state.scale, y: ($('canvas').clientHeight - (maxY - minY) * state.scale) / 2 - minY * state.scale };
    setTransform();
    saveDraft();
  }
  function arrange(shouldRecord = true) {
    const nodes = state.model.nodes();
    const edges = state.model.dependencies().filter(edge => edge.targetId);
    const depth = new Map();
    function level(id, visiting = new Set()) {
      if (depth.has(id)) return depth.get(id);
      if (visiting.has(id)) return 0;
      const next = new Set(visiting).add(id);
      const providers = edges.filter(edge => edge.sourceId === id).map(edge => edge.targetId);
      const value = Math.min(6, providers.length ? 1 + Math.max(...providers.map(target => level(target, next))) : 0);
      depth.set(id, value);
      return value;
    }
    const columns = new Map();
    for (const node of nodes) {
      const column = level(node.id);
      const count = columns.get(column) || 0;
      // Providers are on the right, matching the dependency arrows.
      state.positions[node.id] = { x: -column * 305, y: count };
      const depCount = Math.min(6, node.definition?.dependencies?.length || 0);
      columns.set(column, count + 145 + depCount * 26);
    }
    renderGraph();
    fit();
    if (shouldRecord) record();
  }
  function freePosition() {
    const rect = $('canvas').getBoundingClientRect();
    const start = worldPoint(rect.left + 30, rect.top + 30);
    const width = Math.max(250, (rect.width - 60) / state.scale);
    const occupied = state.model.nodes().map(node => ({ position: state.positions[node.id], height: nodeElement(node.id)?.offsetHeight || 200 })).filter(item => item.position);
    for (let row = 0; row < 6; row++) {
      for (let column = 0; column < Math.max(1, Math.floor(width / 275)); column++) {
        const candidate = { x: start.x + column * 275, y: start.y + row * 280 };
        if (!occupied.some(item => Math.abs(item.position.x - candidate.x) < 245 && candidate.y < item.position.y + item.height + 25 && candidate.y + 230 > item.position.y)) return candidate;
      }
    }
    return { x: start.x + occupied.length * 25, y: start.y + occupied.length * 25 };
  }
  function addClass(name, point) {
    const position = point || freePosition();
    mutate(() => {
      const node = state.model.addNode(name);
      state.selected = node.id;
      state.positions[node.id] = position;
    }, `Added ${name}. Select its properties to configure it.`);
  }
  function renderLibrary() {
    const query = $('class-search').value.toLowerCase().trim();
    const helpers = $('show-helpers').checked;
    const classes = state.schema.classes.filter(item => (helpers || item.creatable || item.name === 'ModuleMgr') && `${item.name} ${item.category}`.toLowerCase().includes(query));
    $('class-count').textContent = classes.length;
    $('class-library').replaceChildren();
    const groups = new Map();
    for (const item of classes) {
      const category = item.category || 'Other';
      if (!groups.has(category)) groups.set(category, []);
      groups.get(category).push(item);
    }
    for (const [category, items] of [...groups].sort(([a], [b]) => a.localeCompare(b))) {
      const group = element('details', 'category');
      group.open = Boolean(query) || ['Base', 'Module', 'Vision', 'Vision/Pipeline', 'UI'].includes(category);
      const summary = element('summary', '', category);
      summary.append(element('span', 'badge', items.length));
      group.append(summary);
      for (const definition of items.sort((a, b) => a.name.localeCompare(b.name))) {
        const creatable = definition.creatable || definition.name === 'ModuleMgr';
        const item = element('div', `class-item${creatable ? '' : ' reference'}`);
        item.draggable = creatable;
        item.dataset.class = definition.name;
        item.title = creatable ? `${definition.source || ''}\nDrag to add ${definition.name}` : `${definition.source || ''}\nBase/helper class; not registered in Module::createInstance.`;
        const info = element('div', 'class-info');
        info.append(element('div', 'class-name', definition.name), element('div', 'class-base', creatable ? (definition.baseClasses || []).join(' · ') || category : 'Base / helper · reference'));
        item.append(info);
        if (creatable) {
          const add = button('+', () => addClass(definition.name), 'class-add');
          add.setAttribute('aria-label', `Add ${definition.name}`);
          item.append(add);
          item.addEventListener('dragstart', event => {
            event.dataTransfer.setData('application/x-openkai-class', definition.name);
            event.dataTransfer.setData('text/plain', definition.name);
            event.dataTransfer.effectAllowed = 'copy';
          });
          item.addEventListener('dblclick', event => { if (!event.target.closest('button')) addClass(definition.name); });
        }
        group.append(item);
      }
      $('class-library').append(group);
    }
    if (!classes.length) $('class-library').append(element('p', 'hint', 'No matching classes.'));
  }
  function selectNode(id) {
    state.selected = id;
    renderGraph();
    renderInspector();
  }
  function canConnect(node, dependency) {
    return !node.nested && node.className !== 'ModuleMgr' && state.model.compatible(node.className, dependency.targetClass || 'BASE');
  }
  function beginConnection(node, dependency, event) {
    if (event) event.stopPropagation();
    const sameSelection = state.selected === node.id;
    state.pending = { sourceId: node.id, dependency };
    state.selected = node.id;
    $('connection-hint').hidden = false;
    $('connection-hint').textContent = `Connect ${node.name}.${pathLabel(dependency.path)} → ${dependency.targetClass || 'BASE'} provider · Esc cancels`;
    renderGraph();
    if (!sameSelection) renderInspector();
  }
  function concreteDependencyPath(definition) {
    return definition.path.map((part, index) => {
      if (part !== '*') return part;
      const container = (definition.containers || []).find(item => item.type === 'object' && pathKey(item.path) === pathKey(definition.path.slice(0, index)));
      if (!container) return part;
      const input = [...$('inspector').querySelectorAll('[data-map-path]')].find(item => item.dataset.dependency === pathKey(definition.path) && item.dataset.mapPath === pathKey(container.path));
      const key = input?.value.trim();
      if (!key) throw new Error(`Enter an object key for ${pathLabel(container.path)} in the dependency inspector first.`);
      return key;
    });
  }
  function cancelConnection() {
    state.pending = null;
    $('connection-hint').hidden = true;
  }
  function finishConnection(targetId) {
    if (!state.pending) return;
    const { sourceId, dependency } = state.pending;
    if (mutate(() => state.model.connect(sourceId, concreteDependencyPath(dependency), targetId), 'Dependency connected.')) {
      cancelConnection();
      renderGraph();
    }
  }
  function renderGraph() {
    $('nodes').replaceChildren();
    const nodes = state.model.nodes();
    const edges = state.model.dependencies();
    nodes.forEach((node, index) => {
      if (!state.positions[node.id]) state.positions[node.id] = { x: (index % 3) * 285, y: Math.floor(index / 3) * 245 };
      const position = state.positions[node.id];
      const card = element('article', 'graph-node');
      card.dataset.id = node.id;
      card.tabIndex = 0;
      card.setAttribute('aria-label', `${node.name}, ${node.className}`);
      card.classList.toggle('selected', node.id === state.selected);
      card.classList.toggle('disabled-node', node.data.bON === 0);
      card.classList.toggle('compatible', Boolean(state.pending && canConnect(node, state.pending.dependency)));
      card.style.left = `${position.x}px`;
      card.style.top = `${position.y}px`;
      const heading = element('div', 'node-header');
      const title = element('div', 'node-title');
      title.append(element('strong', '', node.name), element('small', '', node.className));
      heading.append(title);
      heading.addEventListener('pointerdown', event => {
        if (event.button !== 0 || state.pending) return;
        event.stopPropagation();
        state.drag = { type: 'node', id: node.id, x: event.clientX, y: event.clientY, origin: { ...position }, moved: false };
        state.selected = node.id;
        for (const existing of $('nodes').children) existing.classList.toggle('selected', existing.dataset.id === node.id);
        renderInspector();
      });
      card.append(heading, element('div', 'node-category', `${node.nested ? 'Embedded · ' : ''}${node.definition?.category || 'Unknown class'}`));
      if (!node.nested && node.className !== 'ModuleMgr') {
        const provider = button('', event => { event.stopPropagation(); if (state.pending) finishConnection(node.id); else selectNode(node.id); }, 'port provider-port');
        provider.title = 'Dependency provider: connect a class to this instance';
        provider.setAttribute('aria-label', `Connect to ${node.name}`);
        provider.addEventListener('pointerdown', event => event.stopPropagation());
        card.append(provider);
      }
      const dependencies = node.definition?.dependencies || [];
      if (!dependencies.length) card.append(element('div', 'node-summary', node.className === 'ModuleMgr' ? node.data.appName || 'Application settings' : `${Object.keys(node.data).filter(key => key !== 'class').length} configured properties`));
      for (const dependency of dependencies.slice(0, 6)) {
        const row = element('div', 'node-dependency');
        row.append(element('span', 'dep-label', pathLabel(dependency.path)));
        const port = button('', event => { event.stopPropagation(); }, 'port dependency-port');
        port.dataset.path = pathKey(dependency.path);
        port.title = `${pathLabel(dependency.path)} → ${dependency.targetClass || 'BASE'}; click or drag to a provider`;
        port.setAttribute('aria-label', `Connect ${pathLabel(dependency.path)} for ${node.name}`);
        port.addEventListener('pointerdown', event => {
          if (event.button !== 0) return;
          state.pointer = worldPoint(event.clientX, event.clientY);
          beginConnection(node, dependency, event);
        });
        port.addEventListener('keydown', event => { if (event.key === 'Enter' || event.key === ' ') { event.preventDefault(); beginConnection(node, dependency); } });
        row.append(port);
        card.append(row);
      }
      if (dependencies.length > 6) card.append(element('div', 'node-more', `+ ${dependencies.length - 6} connections in properties`));
      card.addEventListener('click', event => {
        if (event.target.closest('.port')) return;
        if (state.pending) finishConnection(node.id); else selectNode(node.id);
      });
      card.addEventListener('keydown', event => {
        if (event.target !== card) return;
        if (event.key === 'Enter') { if (state.pending) finishConnection(node.id); else selectNode(node.id); }
      });
      $('nodes').append(card);
    });
    $('welcome').hidden = nodes.some(node => node.className !== 'ModuleMgr');
    $('graph-count').value = `${nodes.length} instances · ${edges.filter(edge => edge.resolved).length} links`;
    setTransform();
    drawEdges();
  }
  function nodeElement(id) { return [...$('nodes').children].find(node => node.dataset.id === id); }
  function sourcePoint(sourceId, definition) {
    const card = nodeElement(sourceId);
    const position = state.positions[sourceId];
    if (!card || !position) return null;
    const port = [...card.querySelectorAll('.dependency-port')].find(item => item.dataset.path === pathKey(definition?.path || []));
    return { x: position.x + card.offsetWidth, y: position.y + (port ? port.parentElement.offsetTop + port.parentElement.offsetHeight / 2 : card.offsetHeight - 16) };
  }
  function drawEdges() {
    const layer = $('edge-paths');
    layer.replaceChildren();
    function draw(start, end, className, title) {
      if (!start || !end) return;
      const bend = Math.max(65, Math.abs(end.x - start.x) * .45);
      const path = document.createElementNS('http://www.w3.org/2000/svg', 'path');
      path.setAttribute('class', `edge ${className}`);
      path.setAttribute('d', `M ${start.x} ${start.y} C ${start.x + bend} ${start.y}, ${end.x - bend} ${end.y}, ${end.x} ${end.y}`);
      const label = document.createElementNS('http://www.w3.org/2000/svg', 'title');
      label.textContent = title;
      path.append(label);
      layer.append(path);
    }
    for (const edge of state.model.dependencies()) {
      if (!edge.targetId || !state.positions[edge.targetId]) continue;
      const target = state.positions[edge.targetId];
      draw(sourcePoint(edge.sourceId, edge.definition), { x: target.x - 2, y: target.y + 30 }, `${edge.compatible === false ? 'invalid' : ''} ${[edge.sourceId, edge.targetId].includes(state.selected) ? 'highlight' : ''}`, `${pathLabel(edge.path)} → ${edge.reference}`);
    }
    if (state.pending) draw(sourcePoint(state.pending.sourceId, state.pending.dependency), state.pointer, 'pending', 'New dependency');
  }

  function inferType(value) {
    return Array.isArray(value) ? 'array' : value === null ? 'json' : typeof value === 'object' ? 'object' : typeof value;
  }
  function field(node, path, definition = {}) {
    const value = read(node.data, path);
    const absent = value === undefined;
    const type = ['boolean', 'integer', 'number', 'array', 'object', 'string'].includes(definition.type) ? definition.type : absent ? 'json' : inferType(value);
    const wrapper = element('div', `field${absent ? ' absent' : ''}`);
    const label = element('label', '', pathLabel(path));
    const row = element('div', 'field-row');
    let input;
    if (type === 'boolean') {
      input = element('select');
      for (const [text, val] of [['Use runtime default', ''], ['true', 'true'], ['false', 'false']]) {
        const option = element('option', '', text); option.value = val; input.append(option);
      }
      input.value = absent ? '' : String(value);
    } else if (['array', 'object', 'json', 'any', 'unknown'].includes(type)) {
      input = element('textarea');
      input.spellcheck = false;
      input.value = absent ? '' : JSON.stringify(value, null, 2);
      input.placeholder = own(definition, 'default') ? JSON.stringify(definition.default) : type === 'array' ? '[]' : type === 'object' ? '{}' : 'JSON value';
      if (absent) input.rows = 2;
    } else {
      input = element('input');
      input.type = ['number', 'integer'].includes(type) ? 'number' : 'text';
      if (input.type === 'number') input.step = type === 'integer' ? '1' : 'any';
      input.value = absent ? '' : String(value);
      input.placeholder = own(definition, 'default') ? `Default: ${JSON.stringify(definition.default)}` : 'Use runtime default';
    }
    input.id = `parameter-${encodeURIComponent(node.id)}-${encodeURIComponent(pathKey(path))}`;
    input.dataset.path = pathKey(path);
    label.htmlFor = input.id;
    input.addEventListener('change', () => {
      try {
        let next;
        if (type === 'boolean') next = input.value === '' ? undefined : input.value === 'true';
        else if (['number', 'integer'].includes(type)) {
          next = input.value.trim() === '' ? undefined : Number(input.value);
          if (next !== undefined && (!Number.isFinite(next) || type === 'integer' && !Number.isInteger(next))) throw new Error('Enter a valid ' + type + '.');
        } else if (['array', 'object', 'json', 'any', 'unknown'].includes(type)) {
          next = input.value.trim() === '' ? undefined : JSON.parse(input.value);
          if (next !== undefined && type === 'array' && !Array.isArray(next)) throw new Error('Enter a JSON array.');
          if (next !== undefined && type === 'object' && (!next || Array.isArray(next) || typeof next !== 'object')) throw new Error('Enter a JSON object.');
        } else next = input.value;
        input.removeAttribute('aria-invalid');
        mutate(() => next === undefined ? state.model.deleteValue(node.id, path) : state.model.setValue(node.id, path, next), `Updated ${pathLabel(path)}.`);
      } catch (error) { input.setAttribute('aria-invalid', 'true'); status(`${pathLabel(path)}: ${error.message}`, true); }
    });
    row.append(input);
    if (!absent) {
      const omit = button('×', () => mutate(() => state.model.deleteValue(node.id, path), `Omitted ${pathLabel(path)}; runtime default will apply.`), 'unset');
      omit.title = 'Omit this parameter and use the runtime default';
      omit.setAttribute('aria-label', `Omit ${pathLabel(path)}`);
      row.append(omit);
    }
    wrapper.append(label, row);
    if (definition.description) wrapper.append(element('div', 'field-meta', definition.description));
    if (!absent && type === 'integer' && !Number.isInteger(value)) wrapper.append(element('div', 'field-meta error', `Existing value: ${JSON.stringify(value)}. Enter an integer to correct its type.`));
    if (absent) wrapper.append(element('div', 'field-meta', `Optional · ${type}${own(definition, 'default') ? ` · default ${JSON.stringify(definition.default)}` : ''}`));
    return wrapper;
  }
  function renderInspector() {
    const inspector = $('inspector');
    const node = state.selected && state.model.nodes().find(item => item.id === state.selected);
    inspector.replaceChildren();
    if (!node) {
      const empty = element('div', 'empty-inspector');
      empty.append(element('span', 'empty-icon', '⌘'), element('h2', '', 'Instance properties'), element('p', '', 'Select an instance to edit its parameters and connections.'));
      inspector.append(empty);
      return;
    }
    inspector.append(element('h2', '', node.className), element('p', 'inspector-subtitle', `${node.definition?.source || 'Class not in catalog'}${node.nested ? ' · Embedded object' : ''}`));
    const actions = element('div', 'instance-actions');
    actions.append(button('Duplicate', () => mutate(() => {
      const copy = clone(node.data);
      delete copy.name;
      const added = state.model.addNode(node.className);
      state.model.replaceNode(added.id, copy);
      state.positions[added.id] = { x: (state.positions[node.id]?.x || 0) + 35, y: (state.positions[node.id]?.y || 0) + 45 };
      state.selected = added.id;
    }, 'Duplicated instance.')));
    actions.lastChild.disabled = node.nested || (!node.definition?.creatable && node.className !== 'ModuleMgr');
    actions.append(button('Delete instance', () => mutate(() => { state.model.removeNode(node.id); delete state.positions[node.id]; state.selected = null; }, 'Removed instance. References to it remain visible as unresolved.'), 'small danger'));
    inspector.append(actions);
    const nameField = element('div', 'field');
    const nameLabel = element('label', '', 'Instance name');
    const nameInput = element('input');
    nameInput.id = 'instance-name';
    nameInput.value = node.key;
    nameInput.disabled = node.nested;
    nameLabel.htmlFor = nameInput.id;
    nameInput.addEventListener('change', () => mutate(() => {
      const old = node.id;
      const renamed = state.model.renameNode(node.id, nameInput.value.trim());
      state.positions[renamed.id] = state.positions[old];
      if (old !== renamed.id) delete state.positions[old];
      state.selected = renamed.id;
    }, 'Renamed instance and updated its resolved references.'));
    nameField.append(nameLabel, nameInput);
    if (node.name !== node.key) nameField.append(element('div', 'field-meta', `Runtime name: ${node.name} (explicit name property)`));
    inspector.append(nameField);
    const buildFlags = node.definition?.buildFlags || node.definition?.buildConditions || [];
    const buildText = Array.isArray(buildFlags) ? buildFlags.map(group => Array.isArray(group) ? group.join(' + ') : group).filter(Boolean).join(' or ') : buildFlags;
    if (buildText) inspector.append(element('p', 'hint', `Build: ${buildText}`));

    const connections = element('section', 'inspector-section');
    connections.append(element('h3', '', 'Dependencies'));
    const edges = state.model.dependencies(node.id);
    for (const definition of node.definition?.dependencies || []) {
      const group = element('div', 'dependency-group');
      group.append(element('h3', '', pathLabel(definition.path)), element('small', '', `${definition.targetClass || 'BASE'}${definition.multiple || definition.path.includes('*') ? ' · multiple' : ''}${definition.required ? ' · required' : ''}`));
      const matching = edges.filter(edge => pathKey(edge.definition?.path || []) === pathKey(definition.path));
      for (const edge of matching) {
        const row = element('div', `dependency-link${edge.resolved ? '' : ' unresolved'}`);
        row.append(element('span', '', `${edge.resolved ? '↗' : '⚠'} ${String(edge.reference)}${definition.path.includes('*') ? ` (${pathLabel(edge.path)})` : ''}`));
        row.append(button('×', () => mutate(() => state.model.disconnect(node.id, edge.path), 'Disconnected dependency.')));
        row.lastChild.setAttribute('aria-label', `Disconnect ${String(edge.reference)}`);
        group.append(row);
      }
      for (const container of definition.containers || []) {
        if (container.type !== 'object' || definition.path[container.path.length] !== '*') continue;
        const keyLabel = element('label', 'field-label', `${pathLabel(container.path)} object key`);
        const keyInput = element('input');
        keyInput.dataset.mapPath = pathKey(container.path);
        keyInput.dataset.dependency = pathKey(definition.path);
        keyInput.id = `map-key-${encodeURIComponent(pathKey(definition.path))}-${container.path.length}`;
        keyInput.placeholder = 'Entry name, e.g. left';
        keyInput.style.width = '100%';
        keyInput.style.fontSize = '11px';
        const keys = Object.keys(read(node.data, container.path) || {});
        keyInput.value = keys[0] || '';
        keyLabel.htmlFor = keyInput.id;
        const list = element('datalist'); list.id = `${keyInput.id}-options`; keyInput.setAttribute('list', list.id);
        for (const key of keys) { const option = element('option'); option.value = key; list.append(option); }
        group.append(keyLabel, keyInput, list);
      }
      const picker = element('div', 'dependency-picker');
      const select = element('select');
      select.setAttribute('aria-label', `Provider for ${pathLabel(definition.path)}`);
      select.dataset.dependency = pathKey(definition.path);
      const placeholder = element('option', '', 'Choose provider…'); placeholder.value = ''; select.append(placeholder);
      for (const target of state.model.nodes().filter(item => canConnect(item, definition))) {
        const option = element('option', '', `${target.name} · ${target.className}${target.data.bON === 0 ? ' (disabled)' : ''}`);
        option.value = target.id; select.append(option);
      }
      const connect = button('+', () => { if (select.value) mutate(() => state.model.connect(node.id, concreteDependencyPath(definition), select.value), 'Dependency connected.'); });
      connect.setAttribute('aria-label', `Add connection for ${pathLabel(definition.path)}`);
      picker.append(select, connect); group.append(picker); connections.append(group);
    }
    if (!(node.definition?.dependencies || []).length) connections.append(element('p', 'hint', 'No cataloged dependencies. Additional fields can be edited as JSON below.'));
    inspector.append(connections);

    const parameters = element('section', 'inspector-section');
    parameters.append(element('h3', '', 'Parameters'));
    const definitions = node.definition?.parameters || [];
    const present = [];
    function collect(value, prefix = []) {
      for (const [key, child] of Object.entries(value)) {
        if (!prefix.length && key === 'class') continue;
        const path = [...prefix, key];
        if (child && typeof child === 'object' && !Array.isArray(child) && Object.keys(child).length && typeof child.class !== 'string') collect(child, path);
        else present.push(path);
      }
    }
    collect(node.data);
    for (const path of present) {
      const definition = definitions.find(item => pathKey(item.path) === pathKey(path)) || {};
      parameters.append(field(node, path, definition));
    }
    const available = definitions.filter(item => !item.path.includes('*') && !['class', 'name'].includes(item.path[0]) && read(node.data, item.path) === undefined && !present.some(path => path.length < item.path.length && path.every((key, index) => key === item.path[index])));
    if (available.length) {
      const details = element('details');
      details.append(element('summary', '', `Available parameters (${available.length})`));
      const search = element('input'); search.type = 'search'; search.placeholder = 'Filter parameters…'; search.setAttribute('aria-label', 'Filter available parameters'); search.style.width = '100%'; search.style.marginBottom = '12px';
      const fields = element('div');
      function fill() {
        fields.replaceChildren();
        for (const definition of available.filter(item => pathLabel(item.path).toLowerCase().includes(search.value.toLowerCase()))) fields.append(field(node, definition.path, definition));
      }
      search.addEventListener('input', fill); fill();
      details.append(search, fields); parameters.append(details);
    }
    const repeated = definitions.filter(item => item.path.includes('*'));
    if (repeated.length) {
      const details = element('details');
      details.append(element('summary', '', 'Parameters inside collections'));
      let count = 0;
      for (const definition of repeated) {
        for (const entry of window.OpenKAIModel.expand(node.data, definition.path, true)) {
          details.append(field(node, entry.path, definition));
          count++;
        }
      }
      if (!count) details.append(element('p', 'hint', 'Add a dependency or a row in the collection JSON to edit its individual parameters here.'));
      parameters.append(details);
    }
    if (!present.length && !available.length) parameters.append(element('p', 'hint', 'Use instance JSON to add properties.'));
    inspector.append(parameters);
    const raw = element('details', 'inspector-section');
    raw.append(element('summary', '', 'Instance JSON / custom parameters'));
    const textarea = element('textarea', 'instance-json');
    textarea.id = 'instance-json'; textarea.value = JSON.stringify(node.data, null, 2); textarea.spellcheck = false; textarea.setAttribute('aria-label', 'Instance JSON');
    raw.append(textarea, button('Apply instance JSON', () => {
      try { const data = JSON.parse(textarea.value); mutate(() => state.model.replaceNode(node.id, data), 'Updated instance JSON.'); }
      catch (error) { status(error.message, true); }
    }));
    inspector.append(raw);
  }
  function renderDiagnostics() {
    const issues = state.model.validate();
    $('diagnostics-summary').textContent = issues.length ? `Config checks · ${issues.length} notice${issues.length === 1 ? '' : 's'}` : 'Config checks · No issues found';
    $('diagnostics-summary').classList.toggle('has-issues', issues.length > 0);
    $('diagnostics').replaceChildren();
    for (const issue of issues) {
      const item = button(issue.message, () => { if (issue.nodeId) selectNode(issue.nodeId); }, `diagnostic ${issue.severity}`);
      $('diagnostics').append(item);
    }
    if (!issues.length) $('diagnostics').append(element('p', 'hint', 'Connections and known parameter types look consistent. Hardware settings and build availability are checked by OpenKAI at launch.'));
  }
  function render() { renderGraph(); renderInspector(); renderDiagnostics(); }
  function loadDocument(document, filename) {
    const result = mutate(() => {
      state.model.import(document);
      state.positions = {};
      state.selected = null;
      cancelConnection();
    }, 'Loaded config. Unknown fields are preserved.');
    if (result) {
      if (filename) $('filename').value = filename;
      arrange(false);
      state.history[state.cursor] = snapshot();
      saveDraft();
    }
    return result;
  }
  function showJSON(paste = false) {
    $('json-title').textContent = paste ? 'Paste config JSON' : 'Edit config JSON';
    $('json-description').textContent = 'Apply JSON to rebuild the graph. Unknown fields are preserved. Copy and download export the config without canvas metadata.';
    $('json-text').value = paste ? '' : state.model.export();
    $('json-error').textContent = '';
    $('json-dialog').showModal();
    $('json-text').focus();
  }
  async function copy(text) {
    try {
      if (!navigator.clipboard?.writeText) throw new Error('Clipboard unavailable');
      await navigator.clipboard.writeText(text);
    } catch (_) {
      const temporary = element('textarea');
      temporary.value = text;
      temporary.style.cssText = 'position:fixed;left:-9999px;top:0';
      ($('json-dialog').open ? $('json-dialog') : document.body).append(temporary);
      temporary.select();
      const ok = document.execCommand('copy');
      temporary.remove();
      if (!ok) {
        if (!$('json-dialog').open) showJSON();
        $('json-text').value = text;
        $('json-text').select();
        status('Clipboard unavailable. JSON is selected; press Ctrl/Cmd+C to copy.', true);
        return;
      }
    }
    status('Config JSON copied.');
  }
  function download() {
    const blob = new Blob([state.model.export() + '\n'], { type: 'application/json' });
    const url = URL.createObjectURL(blob);
    const link = element('a');
    link.href = url;
    const filename = $('filename').value.trim() || 'OpenKAI.json';
    link.download = filename.endsWith('.json') ? filename : `${filename}.json`;
    document.body.append(link); link.click(); link.remove();
    setTimeout(() => URL.revokeObjectURL(url), 10000);
    status(`Downloaded ${link.download}.`);
  }
  function bindEvents() {
    $('class-search').addEventListener('input', renderLibrary);
    $('show-helpers').addEventListener('change', renderLibrary);
    $('new-config').onclick = () => loadDocument(initialConfig(), 'OpenKAI.json');
    $('load-example').onclick = () => loadDocument({
      APP: { class: 'ModuleMgr', appName: 'CameraCrop', bLog: true, bStdErr: true },
      cam: { class: '_Camera', bON: 1, thread: { FPS: 30 }, deviceID: 0, vSizeRGB: [640, 480] },
      crop: { class: '_Crop', thread: { FPS: 30 }, _VisionBase: 'cam', vRoi: [0, 0, 320, 240] },
      view: { class: '_WindowCV', thread: { FPS: 30 }, vBASE: ['cam', 'crop'] }
    }, 'CameraCrop.json');
    $('open-config').onclick = () => $('config-file').click();
    $('config-file').onchange = async event => {
      const file = event.target.files[0];
      if (file) { try { loadDocument(await file.text(), file.name); } catch (error) { status(error.message, true); } }
      event.target.value = '';
    };
    $('load-schema').onclick = () => $('schema-file').click();
    $('schema-file').onchange = async event => {
      const file = event.target.files[0];
      if (file) {
        try {
          const schema = JSON.parse(await file.text());
          validateSchema(schema);
          const model = new window.OpenKAIModel(schema, state.model.toJSON());
          state.schema = schema; state.model = model;
          $('schema-status').textContent = `Loaded ${file.name} · ${schema.classes.length} classes`;
          renderLibrary(); render(); status('Loaded class schema.');
        } catch (error) { status(`Schema: ${error.message}`, true); }
      }
      event.target.value = '';
    };
    $('paste-config').onclick = () => showJSON(true);
    $('show-json').onclick = () => showJSON();
    $('close-json').onclick = () => $('json-dialog').close();
    $('apply-json').onclick = () => {
      if (loadDocument($('json-text').value)) $('json-dialog').close();
      else $('json-error').textContent = $('status').textContent;
    };
    $('dialog-copy').onclick = () => copy($('json-text').value);
    $('copy-config').onclick = () => copy(state.model.export());
    $('download-config').onclick = download;
    $('filename').onchange = saveDraft;
    $('help').onclick = () => $('help-dialog').showModal();
    $('close-help').onclick = () => $('help-dialog').close();
    $('undo').onclick = () => undo(-1);
    $('redo').onclick = () => undo(1);
    $('auto-layout').onclick = () => arrange();
    $('fit-view').onclick = fit;
    $('zoom-in').onclick = () => zoom(1.2);
    $('zoom-out').onclick = () => zoom(1 / 1.2);
    $('canvas').addEventListener('wheel', event => { event.preventDefault(); zoom(Math.exp(-event.deltaY * .0015), { x: event.clientX, y: event.clientY }); }, { passive: false });
    $('canvas').addEventListener('dragover', event => { event.preventDefault(); event.dataTransfer.dropEffect = 'copy'; });
    $('canvas').addEventListener('drop', event => {
      event.preventDefault();
      const className = event.dataTransfer.getData('application/x-openkai-class') || event.dataTransfer.getData('text/plain');
      if (state.schema.classes.some(item => item.name === className)) addClass(className, worldPoint(event.clientX, event.clientY));
    });
    $('canvas').addEventListener('pointerdown', event => {
      if (event.button !== 0 || event.target.closest('.graph-node, button')) return;
      if (state.pending) { cancelConnection(); renderGraph(); return; }
      state.drag = { type: 'pan', x: event.clientX, y: event.clientY, origin: { ...state.pan }, moved: false };
      $('canvas').classList.add('panning');
    });
    window.addEventListener('pointermove', event => {
      state.pointer = worldPoint(event.clientX, event.clientY);
      if (state.pending) drawEdges();
      const drag = state.drag;
      if (!drag) return;
      const dx = event.clientX - drag.x; const dy = event.clientY - drag.y;
      if (Math.abs(dx) + Math.abs(dy) > 3) drag.moved = true;
      if (drag.type === 'pan') {
        state.pan = { x: drag.origin.x + dx, y: drag.origin.y + dy }; setTransform();
      } else {
        const position = { x: drag.origin.x + dx / state.scale, y: drag.origin.y + dy / state.scale };
        state.positions[drag.id] = position;
        const card = nodeElement(drag.id);
        if (card) { card.style.left = `${position.x}px`; card.style.top = `${position.y}px`; }
        drawEdges();
      }
    });
    window.addEventListener('pointerup', event => {
      if (state.pending) {
        const target = document.elementFromPoint(event.clientX, event.clientY);
        const card = target?.closest('.graph-node');
        if (card && !target.closest('.dependency-port') && card.dataset.id !== state.pending.sourceId) finishConnection(card.dataset.id);
      }
      if (state.drag?.moved) { if (state.drag.type === 'node') record(); else saveDraft(); }
      state.drag = null;
      $('canvas').classList.remove('panning');
    });
    window.addEventListener('keydown', event => {
      if (event.key === 'Escape') { cancelConnection(); renderGraph(); }
      if (event.target.closest('input, textarea, select') || document.querySelector('dialog[open]')) return;
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'z') { event.preventDefault(); undo(event.shiftKey ? 1 : -1); }
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 'y') { event.preventDefault(); undo(1); }
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === 's') { event.preventDefault(); download(); }
    });
    window.addEventListener('resize', drawEdges);
  }
  function validateSchema(schema) {
    if (!schema || schema.schemaVersion !== 1 || !Array.isArray(schema.classes)) throw new Error('Expected schemaVersion: 1 and a classes array.');
    const names = new Set();
    for (const item of schema.classes) {
      if (!item || typeof item.name !== 'string' || names.has(item.name)) throw new Error('Each class needs a unique name.');
      names.add(item.name);
      for (const property of [...(item.parameters || []), ...(item.dependencies || [])]) {
        if (!Array.isArray(property.path) || !property.path.length) throw new Error(`Invalid parameter path in ${item.name}.`);
      }
    }
  }
  async function boot() {
    try {
      let schema = window.OPENKAI_SCHEMA;
      let source = 'Bundled schema · offline ready';
      if (location.protocol !== 'file:') {
        try {
          const response = await fetch('OpenKAI.json', { cache: 'no-cache' });
          if (!response.ok) throw new Error(String(response.status));
          schema = await response.json(); source = 'OpenKAI.json';
        } catch (_) { source = 'Bundled schema · JSON fetch unavailable'; }
      }
      validateSchema(schema);
      state.schema = schema;
      state.model = new window.OpenKAIModel(schema, initialConfig());
      let recovered = false;
      try {
        const draft = JSON.parse(localStorage.getItem(STORAGE_KEY));
        if (draft?.document) {
          state.model.import(draft.document); state.positions = draft.positions || {}; state.selected = draft.selected || null;
          if (Number.isFinite(draft.scale)) state.scale = Math.max(.2, Math.min(2, draft.scale));
          if (Number.isFinite(draft.pan?.x) && Number.isFinite(draft.pan?.y)) state.pan = draft.pan;
          if (draft.filename) $('filename').value = draft.filename;
          recovered = true;
        }
      } catch (_) { /* A corrupt or unavailable local draft must not block startup. */ }
      $('schema-status').textContent = `${source} · ${schema.classes.length} classes`;
      bindEvents(); renderLibrary(); render(); record();
      status(recovered ? 'Restored your local draft. Use New to start a fresh config.' : 'Ready. Add a class or open an existing config.');
      window.OpenKAIEditor = { get model() { return state.model; }, get schema() { return state.schema; }, addClass, loadDocument, selectNode, exportConfig: () => state.model.export() };
      document.body.dataset.ready = 'true';
    } catch (error) {
      status(`Could not start editor: ${error.message}`, true);
      $('schema-status').textContent = 'Schema unavailable. Keep js/schema-data.js next to the editor scripts.';
    }
  }
  boot();
}());
