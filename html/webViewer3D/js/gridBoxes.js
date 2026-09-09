import * as THREE from '../vendor/three.module.min.js';
import { cellBox, CELL_BYTES } from './octreeCells.js';

// One shared wire box and one instance per occupied cell. Instance index maps
// directly to the full cell ID and a Box3, ready for a future picking interface.
export class GridBoxes extends THREE.LineSegments {
  constructor() {
    const vertices = [];
    for (let i = 0; i < 8; ++i) for (const bit of [1, 2, 4]) if (!(i & bit))
      for (const corner of [i, i | bit])
        vertices.push((corner & 4 ? 0.5 : -0.5), (corner & 2 ? 0.5 : -0.5), (corner & 1 ? 0.5 : -0.5));
    const geometry = new THREE.InstancedBufferGeometry();
    geometry.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
    geometry.instanceCount = 0;
    super(geometry, new THREE.ShaderMaterial({
      uniforms: { opacity: { value: 1 } },
      vertexShader: `attribute vec3 cellCenter;
        attribute vec3 cellSize;
        attribute vec3 cellColor;
        varying vec3 boxColor;
        void main() {
          boxColor = cellColor;
          gl_Position = projectionMatrix * modelViewMatrix * vec4(cellCenter + position * cellSize, 1.0);
        }`,
      fragmentShader: `uniform float opacity;
        varying vec3 boxColor;
        void main() {
          gl_FragColor = vec4(boxColor, opacity);
          // Match the output conversion used by PointsMaterial.
          #include <tonemapping_fragment>
          #include <colorspace_fragment>
        }`
    }));
    this.grid = null;
    this.capacity = 0;
  }
  update(grid, bounds, opacity) {
    this.grid = grid;
    const count = grid ? grid.cells.length / CELL_BYTES : 0;
    let geometry = this.geometry;
    if (count > this.capacity) {
      this.capacity = 2 ** Math.ceil(Math.log2(Math.max(count, 256)));
      // Release old GPU attributes when the capacity grows.
      const position = geometry.getAttribute('position');
      geometry.dispose();
      geometry = this.geometry = new THREE.InstancedBufferGeometry();
      geometry.setAttribute('position', position);
      for (const name of ['cellCenter', 'cellSize', 'cellColor']) {
        const array = name === 'cellColor' ? new Uint8Array(this.capacity * 3) : new Float32Array(this.capacity * 3);
        geometry.setAttribute(name, new THREE.InstancedBufferAttribute(array, 3, name === 'cellColor').setUsage(THREE.DynamicDrawUsage));
      }
    }
    if (count) {
      for (let i = 0; i < count; ++i) {
        const box = cellBox(grid, i);
        geometry.getAttribute('cellCenter').array.set(box.center, i * 3);
        geometry.getAttribute('cellSize').array.set(box.size, i * 3);
        geometry.getAttribute('cellColor').array.set(box.color, i * 3);
      }
      for (const name of ['cellCenter', 'cellSize', 'cellColor']) {
        const attribute = geometry.getAttribute(name);
        attribute.clearUpdateRanges();
        attribute.addUpdateRange(0, count * 3);
        attribute.needsUpdate = true;
      }
      geometry.boundingBox = new THREE.Box3(new THREE.Vector3(...bounds.slice(0, 3)), new THREE.Vector3(...bounds.slice(3)));
      geometry.boundingSphere = geometry.boundingBox.getBoundingSphere(new THREE.Sphere());
    }
    geometry.instanceCount = count;
    geometry.setDrawRange(0, count ? 24 : 0);
    this.material.uniforms.opacity.value = opacity;
  }
  getCell(index) {
    if (!Number.isInteger(index) || index < 0 || index >= this.geometry.instanceCount) return null;
    const cell = cellBox(this.grid, index);
    return { id: cell.id, color: cell.color,
      box: new THREE.Box3().setFromCenterAndSize(new THREE.Vector3(...cell.center), new THREE.Vector3(...cell.size)) };
  }
}
