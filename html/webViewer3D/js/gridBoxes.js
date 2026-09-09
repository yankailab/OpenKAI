import * as THREE from '../vendor/three.module.min.js';
import { cellBox, CELL_BYTES } from './octreeCells.js';

// Shared wire/solid box shapes and one instance per occupied cell. Instance index maps
// to the original cell record, retaining exact IDs when levels are filtered.
export class GridBoxes extends THREE.Group {
  constructor() {
    super();
    const vertices = [];
    for (let i = 0; i < 8; ++i) for (const bit of [1, 2, 4]) if (!(i & bit))
      for (const corner of [i, i | bit])
        vertices.push((corner & 4 ? 0.5 : -0.5), (corner & 2 ? 0.5 : -0.5), (corner & 1 ? 0.5 : -0.5));
    const geometry = new THREE.InstancedBufferGeometry();
    geometry.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
    geometry.instanceCount = 0;
    this.material = new THREE.ShaderMaterial({
      uniforms: { opacity: { value: 1 } },
      vertexShader: `attribute vec3 cellCenter;
        attribute vec3 cellSize;
        attribute vec4 cellColor;
        varying vec4 boxColor;
        void main() {
          boxColor = cellColor;
          gl_Position = projectionMatrix * modelViewMatrix * vec4(cellCenter + position * cellSize, 1.0);
        }`,
      fragmentShader: `uniform float opacity;
        varying vec4 boxColor;
        void main() {
          gl_FragColor = vec4(boxColor.rgb, boxColor.a * opacity);
          if (gl_FragColor.a <= 0.0) discard;
          // Match the output conversion used by PointsMaterial.
          #include <tonemapping_fragment>
          #include <colorspace_fragment>
        }`
    });
    this.geometry = geometry;
    const box = new THREE.BoxGeometry(1, 1, 1);
    const solidGeometry = new THREE.InstancedBufferGeometry();
    solidGeometry.setAttribute('position', box.getAttribute('position'));
    solidGeometry.setIndex(box.index);
    solidGeometry.instanceCount = 0;
    box.dispose();
    this.wire = new THREE.LineSegments(geometry, this.material);
    this.solid = new THREE.Mesh(solidGeometry, this.material);
    this.solid.visible = false;
    this.add(this.wire, this.solid);
    this.grid = null;
    this.capacity = 0;
    this.cellIndices = new Uint32Array(0);
    this.minLevel = 0;
    this.maxLevel = 40;
    this.bounds = null;
    this.sortDirty = true;
    this.sortCapacity = 0;
    this.viewMatrix = new THREE.Matrix4();
    this.sortedMatrix = new THREE.Matrix4();
  }
  setSolid(solid) {
    this.wire.visible = !solid;
    this.solid.visible = solid;
  }
  setLevelRange(min, max) {
    if (this.minLevel === min && this.maxLevel === max) return;
    this.minLevel = min;
    this.maxLevel = max;
    if (this.grid) this.update(this.grid, this.bounds, this.material.uniforms.opacity.value);
  }
  update(grid, bounds, opacity) {
    this.grid = grid;
    this.bounds = bounds;
    const total = grid ? grid.cells.length / CELL_BYTES : 0;
    let count = 0;
    for (let i = 0; i < total; ++i) {
      const depth = grid.cells[i * CELL_BYTES] & 63;
      if (depth >= this.minLevel && depth <= this.maxLevel) ++count;
    }
    let transparent = opacity < 1 || !this.material.depthTest;
    let geometry = this.geometry;
    if (count > this.capacity) {
      this.capacity = 2 ** Math.ceil(Math.log2(Math.max(count, 256)));
      this.cellIndices = new Uint32Array(this.capacity);
      // Release old GPU attributes when the capacity grows.
      const position = geometry.getAttribute('position');
      const solidPosition = this.solid.geometry.getAttribute('position'), index = this.solid.geometry.index;
      geometry.dispose();
      this.solid.geometry.dispose();
      geometry = this.geometry = new THREE.InstancedBufferGeometry();
      this.wire.geometry = geometry;
      geometry.setAttribute('position', position);
      this.solid.geometry = new THREE.InstancedBufferGeometry();
      this.solid.geometry.setAttribute('position', solidPosition);
      this.solid.geometry.setIndex(index);
      for (const name of ['cellCenter', 'cellSize', 'cellColor']) {
        const itemSize = name === 'cellColor' ? 4 : 3;
        const array = name === 'cellColor' ? new Uint8Array(this.capacity * itemSize) : new Float32Array(this.capacity * itemSize);
        const attribute = new THREE.InstancedBufferAttribute(array, itemSize, name === 'cellColor').setUsage(THREE.DynamicDrawUsage);
        geometry.setAttribute(name, attribute);
        this.solid.geometry.setAttribute(name, attribute);
      }
    }
    if (count) {
      let instance = 0;
      for (let i = 0; i < total; ++i) {
        const depth = grid.cells[i * CELL_BYTES] & 63;
        if (depth < this.minLevel || depth > this.maxLevel) continue;
        const box = cellBox(grid, i);
        this.cellIndices[instance] = i;
        geometry.getAttribute('cellCenter').array.set(box.center, instance * 3);
        geometry.getAttribute('cellSize').array.set(box.size, instance * 3);
        geometry.getAttribute('cellColor').array.set(box.color, instance * 4);
        if (box.color[3] < 255) transparent = true;
        ++instance;
      }
      for (const name of ['cellCenter', 'cellSize', 'cellColor']) {
        const attribute = geometry.getAttribute(name);
        attribute.clearUpdateRanges();
        attribute.addUpdateRange(0, count * attribute.itemSize);
        attribute.needsUpdate = true;
      }
      geometry.boundingBox = new THREE.Box3(new THREE.Vector3(...bounds.slice(0, 3)), new THREE.Vector3(...bounds.slice(3)));
      geometry.boundingSphere = geometry.boundingBox.getBoundingSphere(new THREE.Sphere());
      this.solid.geometry.boundingBox = geometry.boundingBox;
      this.solid.geometry.boundingSphere = geometry.boundingSphere;
    }
    geometry.instanceCount = count;
    geometry.setDrawRange(0, count ? 24 : 0);
    this.solid.geometry.instanceCount = count;
    this.solid.geometry.setDrawRange(0, count ? 36 : 0);
    this.material.uniforms.opacity.value = opacity;
    if (this.material.transparent !== transparent) {
      this.material.transparent = transparent;
      this.material.needsUpdate = true;
    }
    this.material.depthWrite = !transparent;
    this.sortDirty = true;
  }
  // Sort whole cells back-to-front for alpha blending. Reuse scratch storage and
  // only sort when a snapshot or the camera/object transform has changed.
  // Intersecting/nested surfaces still have the usual object-sorting limitations.
  sortCells(camera) {
    const count = this.geometry.instanceCount;
    if (!this.visible || !this.solid.visible || !this.material.transparent || count < 2) return;
    this.viewMatrix.multiplyMatrices(camera.matrixWorldInverse, this.matrixWorld);
    if (!this.sortDirty && this.viewMatrix.equals(this.sortedMatrix)) return;
    this.sortedMatrix.copy(this.viewMatrix);
    this.sortDirty = false;
    if (count > this.sortCapacity) {
      this.sortCapacity = this.capacity;
      this.sortOrder = new Uint32Array(this.capacity);
      this.sortDepths = new Float64Array(this.capacity);
      this.sortIDs = new Uint32Array(this.capacity);
      this.sortScratch = new Float32Array(this.capacity * 4);
    }
    const order = this.sortOrder.subarray(0, count), centers = this.geometry.getAttribute('cellCenter').array;
    const e = this.viewMatrix.elements;
    for (let i = 0; i < count; ++i) {
      order[i] = i;
      this.sortDepths[i] = e[2] * centers[i * 3] + e[6] * centers[i * 3 + 1] + e[10] * centers[i * 3 + 2] + e[14];
    }
    order.sort((a, b) => this.sortDepths[a] - this.sortDepths[b]);
    this.sortIDs.set(this.cellIndices.subarray(0, count));
    for (let i = 0; i < count; ++i) this.cellIndices[i] = this.sortIDs[order[i]];
    for (const name of ['cellCenter', 'cellSize', 'cellColor']) {
      const attribute = this.geometry.getAttribute(name), size = attribute.itemSize;
      this.sortScratch.set(attribute.array.subarray(0, count * size));
      for (let i = 0; i < count; ++i) for (let j = 0; j < size; ++j)
        attribute.array[i * size + j] = this.sortScratch[order[i] * size + j];
      attribute.clearUpdateRanges();
      attribute.addUpdateRange(0, count * size);
      attribute.needsUpdate = true;
    }
  }
  dispose() {
    this.geometry.dispose();
    this.solid.geometry.dispose();
    this.material.dispose();
  }
  getCell(index) {
    if (!Number.isInteger(index) || index < 0 || index >= this.geometry.instanceCount) return null;
    const cell = cellBox(this.grid, this.cellIndices[index]);
    return { id: cell.id, color: cell.color,
      box: new THREE.Box3().setFromCenterAndSize(new THREE.Vector3(...cell.center), new THREE.Vector3(...cell.size)) };
  }
}
