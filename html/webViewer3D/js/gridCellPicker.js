import * as THREE from '../vendor/three.module.min.js';
import { GridBoxes } from './gridBoxes.js';
import { CELL_BYTES } from './octreeCells.js';
import { cellIDKey, remapSelection, rootBounds, sameGridHeader, selectionGrid } from './gridSelection.js';

export class GridCellPicker {
  constructor(viewer) {
    this.viewer = viewer;
    this.sources = new Map();
    this.names = new Map();
    this.onChange = () => {};
    this.raycaster = new THREE.Raycaster();
    this.pointer = null;
    this.pointers = new Set();
    const canvas = viewer.renderer.domElement;
    this.handlers = {
      pointerdown: event => {
        this.pointers.add(event.pointerId);
        this.pointer = this.pointers.size === 1 && event.button === 0 ?
          { id: event.pointerId, x: event.clientX, y: event.clientY } : null;
      },
      pointermove: event => {
        if (this.pointer?.id === event.pointerId && Math.hypot(event.clientX - this.pointer.x, event.clientY - this.pointer.y) > 5)
          this.pointer = null;
      },
      pointerup: event => {
        const click = this.pointer?.id === event.pointerId && event.button === 0 &&
          Math.hypot(event.clientX - this.pointer.x, event.clientY - this.pointer.y) <= 5;
        this.pointer = null;
        this.pointers.delete(event.pointerId);
        if (click) this.pickAt(event.clientX, event.clientY);
      },
      pointercancel: event => { this.pointer = null; this.pointers.delete(event.pointerId); },
      lostpointercapture: event => { this.pointer = null; this.pointers.delete(event.pointerId); }
    };
    for (const [type, handler] of Object.entries(this.handlers)) canvas.addEventListener(type, handler, true);
  }
  configure(objects) { this.names = new Map(objects.map(o => [o.id, o.name])); }
  get count() { return [...this.sources.values()].reduce((sum, source) => sum + source.selected.size, 0); }
  sourceFor(id) { return this.sources.get(this.names.get(id) ?? String(id)); }
  updateObject(object, grid) {
    let source = this.sourceFor(object.id);
    if (!grid) { if (source) source.occupied = null; return; }
    if (!source) {
      const module = this.names.get(object.id) ?? String(object.id);
      const overlay = new GridBoxes();
      overlay.name = `Selected cells: ${module}`;
      overlay.renderOrder = 2;
      overlay.visible = false;
      overlay.material.depthTest = false;
      overlay.material.depthWrite = false;
      overlay.material.transparent = true;
      source = { module, selected: new Map(), overlay, header: null };
      this.sources.set(module, source);
      this.viewer.scene.add(overlay);
    }
    const header = { origin: [...grid.origin], size: [...grid.size], maxLevel: grid.maxLevel };
    const changed = source.header && !sameGridHeader(source.header, header);
    if (changed) source.selected = remapSelection(source.header, header, source.selected);
    source.header = header;
    source.occupied = object.boxes;
    source.visible = object.visible;
    source.overlay.visible = object.visible && source.selected.size > 0;
    if (changed) { this.refresh(source); this.onChange(); }
  }
  removeObject(id) {
    const source = this.sourceFor(id);
    if (source) source.occupied = null;
  }
  setVisible(id, visible) {
    const source = this.sourceFor(id);
    if (source) { source.visible = visible; source.overlay.visible = visible && source.selected.size > 0; }
  }
  refresh(source) {
    source.overlay.visible = source.visible && source.selected.size > 0;
    source.overlay.update(selectionGrid(source.header, source.selected), rootBounds(source.header), 1);
  }
  pickAt(clientX, clientY) {
    const { viewer } = this, rect = viewer.renderer.domElement.getBoundingClientRect();
    if (!rect.width || !rect.height || clientX < rect.left || clientX > rect.right || clientY < rect.top || clientY > rect.bottom) return false;
    viewer.camera.updateMatrixWorld();
    this.raycaster.setFromCamera(new THREE.Vector2((clientX - rect.left) / rect.width * 2 - 1,
      1 - (clientY - rect.top) / rect.height * 2), viewer.camera);
    const ray = this.raycaster.ray, forward = viewer.camera.getWorldDirection(new THREE.Vector3());
    const depthAtOrigin = forward.dot(ray.origin.clone().sub(viewer.camera.position));
    const depthPerUnit = forward.dot(ray.direction);
    const hit = this.pickRay(ray, Math.max(0, (viewer.camera.near - depthAtOrigin) / depthPerUnit),
      (viewer.camera.far - depthAtOrigin) / depthPerUnit);
    if (!hit) return false;
    this.toggle(hit.source, hit.id);
    return true;
  }
  pickRay(ray, near = 0, far = Infinity) {
    let best = null;
    const origin = ray.origin.toArray(), direction = ray.direction.toArray();
    for (const source of this.sources.values()) {
      if (!source.visible) continue;
      // Test retained selections too, so expired cells can still be deselected.
      for (const mesh of [source.overlay, source.occupied]) {
        if (!mesh?.visible || !mesh.grid || !mesh.geometry.instanceCount || mesh.material.uniforms.opacity.value <= 0) continue;
        const centers = mesh.geometry.getAttribute('cellCenter').array;
        const sizes = mesh.geometry.getAttribute('cellSize').array;
        for (let i = 0; i < mesh.geometry.instanceCount; ++i) {
          const at = mesh.cellIndices[i] * CELL_BYTES;
          if (mesh.grid.cells[at + 19] === 0) continue;
          const depth = mesh.grid.cells[at] & 63;
          if (best && depth < best.depth) continue;
          let distance = near, end = far;
          for (let axis = 0; axis < 3; ++axis) {
            const min = centers[i * 3 + axis] - sizes[i * 3 + axis] / 2;
            const max = centers[i * 3 + axis] + sizes[i * 3 + axis] / 2;
            if (direction[axis] === 0) {
              if (origin[axis] < min || origin[axis] > max) { end = -Infinity; break; }
            } else {
              const a = (min - origin[axis]) / direction[axis], b = (max - origin[axis]) / direction[axis];
              distance = Math.max(distance, Math.min(a, b));
              end = Math.min(end, Math.max(a, b));
            }
          }
          if (distance > end) continue;
          if (!best || depth > best.depth || distance < best.distance) {
            best = { source, depth, distance, id: mesh.grid.cells.slice(at, at + 16) };
          }
        }
      }
    }
    return best;
  }
  toggle(source, id) {
    const key = cellIDKey(id);
    if (source.selected.has(key)) source.selected.delete(key);
    else source.selected.set(key, id.slice());
    this.refresh(source);
    this.onChange();
  }
  clear() {
    for (const source of this.sources.values()) {
      source.selected.clear();
      this.refresh(source);
    }
    this.onChange();
  }
  expandBounds(bounds) {
    const min = new THREE.Vector3(), max = new THREE.Vector3();
    for (const source of this.sources.values()) {
      if (!source.visible || !source.selected.size) continue;
      const geometry = source.overlay.geometry;
      const centers = geometry.getAttribute('cellCenter').array, sizes = geometry.getAttribute('cellSize').array;
      for (let i = 0; i < geometry.instanceCount; ++i) {
        min.set(centers[i * 3] - sizes[i * 3] / 2, centers[i * 3 + 1] - sizes[i * 3 + 1] / 2, centers[i * 3 + 2] - sizes[i * 3 + 2] / 2);
        max.set(centers[i * 3] + sizes[i * 3] / 2, centers[i * 3 + 1] + sizes[i * 3 + 1] / 2, centers[i * 3 + 2] + sizes[i * 3 + 2] / 2);
        bounds.expandByPoint(min); bounds.expandByPoint(max);
      }
    }
  }
  commands() {
    return [...this.sources.values()].filter(source => source.selected.size).map(source => ({
      cmd: 'octGridCellSelect', module: source.module,
      vPorigin: source.header.origin.map(String), vRootCellSize: source.header.size.map(String),
      cellIDs: [...source.selected.keys()]
    }));
  }
  dispose() {
    for (const [type, handler] of Object.entries(this.handlers)) this.viewer.renderer.domElement.removeEventListener(type, handler, true);
    for (const source of this.sources.values()) {
      this.viewer.scene.remove(source.overlay);
      source.overlay.dispose();
    }
    this.sources.clear();
  }
}
