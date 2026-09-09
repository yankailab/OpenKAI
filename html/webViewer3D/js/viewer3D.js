import * as THREE from '../vendor/three.module.min.js';
import { STREAM_TYPES } from './protocol.js';
import { GridBoxes } from './gridBoxes.js';
import { GridCellPicker } from './gridCellPicker.js';
import { OrbitControls } from '../vendor/OrbitControls.js';

export class Viewer3D {
  constructor(container) {
    this.container = container;
    this.renderer = new THREE.WebGLRenderer({ antialias: false, powerPreference: 'high-performance' });
    this.renderer.setPixelRatio(Math.min(devicePixelRatio, 2));
    container.append(this.renderer.domElement);
    this.scene = new THREE.Scene();
    this.scene.background = new THREE.Color(0.035, 0.045, 0.065);
    this.camera = new THREE.PerspectiveCamera(70, 1, 0.01, 1000000);
    this.camera.position.set(0, -8, 4);
    this.camera.up.set(0, 0, 1);
    this.controls = new OrbitControls(this.camera, this.renderer.domElement);
    this.controls.enableDamping = true;
    this.grid = new THREE.GridHelper(20, 20, 0x466879, 0x253744);
    this.scene.add(this.grid);
    this.axes = this.createOriginAxes();
    this.scene.add(this.axes);
    this.objects = new Map();
    this.picker = new GridCellPicker(this);
    this.pointScale = 1;
    this.gridMinLevel = 0;
    this.gridMaxLevel = 40;
    this.gridSolid = false;
    this.bounds = new THREE.Box3();
    this.boundTypes = new Set();
    this.resizeObserver = new ResizeObserver(() => this.resize());
    this.resizeObserver.observe(container);
    this.resize();
  }
  createOriginAxes() {
    const axes = new THREE.Group();
    axes.name = 'World origin axes';
    const origin = new THREE.Vector3(0, 0, 0);
    const label = (text, color, position) => {
      const canvas = document.createElement('canvas');
      canvas.width = 128; canvas.height = 64;
      const context = canvas.getContext('2d');
      context.font = 'bold 48px sans-serif';
      context.textAlign = 'center'; context.textBaseline = 'middle';
      context.lineWidth = 6; context.strokeStyle = '#090c11';
      context.strokeText(text, 64, 32);
      context.fillStyle = `#${color.toString(16).padStart(6, '0')}`;
      context.fillText(text, 64, 32);
      const texture = new THREE.CanvasTexture(canvas);
      texture.colorSpace = THREE.SRGBColorSpace;
      const sprite = new THREE.Sprite(new THREE.SpriteMaterial({
        map: texture, sizeAttenuation: false, depthWrite: false
      }));
      sprite.name = text;
      sprite.position.copy(position);
      sprite.scale.set(0.08, 0.04, 1);
      axes.add(sprite);
    };
    for (const [name, direction, color] of [
      ['X', new THREE.Vector3(1, 0, 0), 0xff5555],
      ['Y', new THREE.Vector3(0, 1, 0), 0x55dd77],
      ['Z', new THREE.Vector3(0, 0, 1), 0x5599ff]
    ]) {
      // Scene coordinates are metres; length includes the arrowhead.
      const arrow = new THREE.ArrowHelper(direction, origin, 1, color, 0.12, 0.06);
      arrow.name = `${name} axis (1 m)`;
      axes.add(arrow);
      label(name, color, direction.clone().multiplyScalar(1.12));
    }
    const marker = new THREE.Mesh(new THREE.SphereGeometry(0.025, 12, 8),
      new THREE.MeshBasicMaterial({ color: 0xffffff }));
    marker.name = 'Origin (0, 0, 0)';
    axes.add(marker);
    label('O', 0xffffff, new THREE.Vector3(-0.1, -0.1, -0.1));
    // Keep the coordinate reference readable over dense streamed geometry.
    axes.traverse(object => {
      object.renderOrder = 1;
      if (object.material) {
        object.material.depthTest = false;
        object.material.depthWrite = false;
        object.material.transparent = true;
      }
    });
    return axes;
  }
  configure(config) {
    this.clear();
    this.picker.configure(config.objects);
    this.config = config;
    this.scene.background.fromArray(config.background);
    this.grid.visible = config.showGrid;
    this.grid.quaternion.setFromUnitVectors(new THREE.Vector3(0, 1, 0), new THREE.Vector3(...config.camera.up).normalize());
    this.autoBound = config.autoBound;
    this.boundTypes.clear();
    this.resetCamera();
  }
  resize() {
    const width = Math.max(1, this.container.clientWidth), height = Math.max(1, this.container.clientHeight);
    this.renderer.setSize(width, height, false);
    if (this.camera.isPerspectiveCamera) this.camera.aspect = width / height;
    else {
      const halfHeight = (this.camera.top - this.camera.bottom) / 2;
      const center = (this.camera.right + this.camera.left) / 2;
      this.camera.left = center - halfHeight * width / height;
      this.camera.right = center + halfHeight * width / height;
    }
    this.camera.updateProjectionMatrix();
  }
  resetCamera() {
    if (!this.config) return;
    const c = this.config.camera;
    this.camera = c.type === 1 ? new THREE.OrthographicCamera(c.lr[0], c.lr[1], c.bt[1], c.bt[0], c.near, c.far) :
      new THREE.PerspectiveCamera(c.fov, 1, c.near, c.far);
    this.camera.position.fromArray(c.eye);
    this.camera.up.fromArray(c.up).normalize();
    if (!this.camera.up.lengthSq()) this.camera.up.set(0, 0, 1);
    this.controls.dispose();
    this.controls = new OrbitControls(this.camera, this.renderer.domElement);
    this.controls.enableDamping = true;
    this.controls.target.fromArray(c.target);
    if (this.camera.position.distanceToSquared(this.controls.target) < 1e-12) this.camera.position.z += 1;
    this.controls.update();
    this.resize();
  }
  createObject(id) {
    const points = new THREE.Points(new THREE.BufferGeometry(), new THREE.PointsMaterial({ vertexColors: true, sizeAttenuation: false }));
    const lines = new THREE.LineSegments(new THREE.BufferGeometry(), new THREE.LineBasicMaterial({ vertexColors: true }));
    const boxes = new GridBoxes();
    boxes.setLevelRange(this.gridMinLevel, this.gridMaxLevel);
    boxes.setSolid(this.gridSolid);
    const object = { id, points, lines, boxes, visible: true, pointSize: 2, streams: new Map() };
    this.objects.set(id, object);
    this.scene.add(points, lines, boxes);
    return object;
  }
  upload(mesh, positions, colors, bounds) {
    const count = positions.length / 3;
    let geometry = mesh.geometry;
    if (count && (!geometry.getAttribute('position') || geometry.getAttribute('position').count < count)) {
      const capacity = 2 ** Math.ceil(Math.log2(Math.max(count, 256)));
      geometry.dispose();
      geometry = mesh.geometry = new THREE.BufferGeometry();
      geometry.setAttribute('position', new THREE.BufferAttribute(new Float32Array(capacity * 3), 3).setUsage(THREE.DynamicDrawUsage));
      geometry.setAttribute('color', new THREE.BufferAttribute(new Uint8Array(capacity * 4), 4, true).setUsage(THREE.DynamicDrawUsage));
    }
    if (count) {
      for (const [name, data] of [['position', positions], ['color', colors]]) {
        const attribute = geometry.getAttribute(name);
        attribute.array.set(data);
        attribute.clearUpdateRanges();
        attribute.addUpdateRange(0, data.length);
        attribute.needsUpdate = true;
      }
      geometry.boundingBox = new THREE.Box3(new THREE.Vector3(...bounds.slice(0, 3)), new THREE.Vector3(...bounds.slice(3)));
      geometry.boundingSphere = geometry.boundingBox.getBoundingSphere(new THREE.Sphere());
    }
    geometry.setDrawRange(0, count);
  }
  update(frame) {
    const { type } = frame;
    if (!STREAM_TYPES.includes(type)) throw new Error('Unknown geometry stream');
    const active = new Set();
    for (const data of frame.objects) {
      active.add(data.id);
      const o = this.objects.get(data.id) || this.createObject(data.id);
      // Retain only bounds/counts here; point/line buffers have been uploaded
      // and should not keep their full received frame alive between updates.
      o.streams.set(type, { count: data.count, bounds: data.bounds });
      if (type === 'cells') {
        o.boxes.update(data.grid, data.bounds, data.opacity);
        o.boxes.visible = o.visible;
        this.picker.updateObject(o, data.grid);
      } else {
        const mesh = o[type];
        if (type === 'points') {
          o.pointSize = data.pointSize;
          mesh.material.size = data.pointSize * this.pointScale;
        }
        let transparent = data.opacity < 1;
        for (let at = 3; !transparent && at < data.colors.length; at += 4) transparent = data.colors[at] < 255;
        if (mesh.material.transparent !== transparent) { mesh.material.transparent = transparent; mesh.material.needsUpdate = true; }
        mesh.material.depthWrite = !transparent;
        mesh.material.opacity = data.opacity;
        mesh.visible = o.visible;
        this.upload(mesh, data.positions, data.colors, data.bounds);
      }
    }
    for (const o of this.objects.values()) if (o.streams.has(type) && !active.has(o.id)) this.removeStream(o, type);
    this.updateBounds();
    if (this.autoBound && !this.boundTypes.has(type) && frame.objects.some(o => o.count > 0)) {
      this.fit(); this.boundTypes.add(type);
    }
  }
  removeStream(o, type) {
    o.streams.delete(type);
    if (type === 'cells') {
      o.boxes.update(null, [0, 0, 0, 0, 0, 0], 1);
      this.picker.removeObject(o.id);
    } else o[type].geometry.setDrawRange(0, 0);
    if (!o.streams.size) this.removeObject(o);
  }
  clearStream(type) {
    for (const o of this.objects.values()) if (o.streams.has(type)) this.removeStream(o, type);
    this.updateBounds();
  }
  updateBounds() {
    this.bounds.makeEmpty();
    for (const o of this.objects.values()) if (o.visible) for (const data of o.streams.values()) if (data.count)
      this.bounds.union(new THREE.Box3(new THREE.Vector3(...data.bounds.slice(0, 3)), new THREE.Vector3(...data.bounds.slice(3))));
  }
  setVisible(id, visible) {
    const o = this.objects.get(id);
    if (o) { o.visible = visible; o.points.visible = visible; o.lines.visible = visible; o.boxes.visible = visible; }
    this.picker.setVisible(id, visible);
    this.updateBounds();
  }
  setPointScale(value) {
    this.pointScale = value;
    for (const o of this.objects.values()) o.points.material.size = o.pointSize * value;
  }
  setGridLevelRange(min, max) {
    this.gridMinLevel = Math.max(0, Math.min(40, Math.round(min)));
    this.gridMaxLevel = Math.max(this.gridMinLevel, Math.min(40, Math.round(max)));
    for (const o of this.objects.values()) o.boxes.setLevelRange(this.gridMinLevel, this.gridMaxLevel);
  }
  setGridSolid(solid) {
    this.gridSolid = solid;
    for (const o of this.objects.values()) o.boxes.setSolid(solid);
  }
  fit() {
    const bounds = this.bounds.clone();
    this.picker.expandBounds(bounds);
    if (bounds.isEmpty()) return;
    const sphere = bounds.getBoundingSphere(new THREE.Sphere());
    const radius = Math.max(sphere.radius, 0.01);
    const direction = this.camera.position.clone().sub(this.controls.target).normalize();
    if (!direction.lengthSq()) direction.set(0, -1, 0.5).normalize();
    let distance = radius * 3;
    if (this.camera.isPerspectiveCamera) {
      const halfFov = THREE.MathUtils.degToRad(this.camera.fov) / 2;
      const limitingFov = Math.min(halfFov, Math.atan(Math.tan(halfFov) * this.camera.aspect));
      distance = radius / Math.sin(limitingFov) * 1.15;
    } else {
      this.camera.zoom = Math.min(this.camera.right - this.camera.left, this.camera.top - this.camera.bottom) / (radius * 2.3);
    }
    this.controls.target.copy(sphere.center);
    this.camera.position.copy(sphere.center).addScaledVector(direction, distance);
    this.camera.near = Math.max(0.0001, (distance - radius) / 100);
    this.camera.far = Math.max(this.config.camera.far, distance + radius * 10);
    this.camera.updateProjectionMatrix();
    this.controls.update();
  }
  render() {
    this.controls.update();
    this.scene.updateMatrixWorld();
    this.camera.updateMatrixWorld();
    for (const o of this.objects.values()) o.boxes.sortCells(this.camera);
    this.renderer.render(this.scene, this.camera);
  }
  removeObject(o) {
    this.picker.removeObject(o.id);
    this.scene.remove(o.boxes); o.boxes.dispose();
    for (const mesh of [o.points, o.lines]) { this.scene.remove(mesh); mesh.geometry.dispose(); mesh.material.dispose(); }
    this.objects.delete(o.id);
  }
  clear() { for (const o of this.objects.values()) this.removeObject(o); this.bounds.makeEmpty(); }
  dispose() {
    this.clear(); this.resizeObserver.disconnect(); this.controls.dispose();
    this.picker.dispose();
    this.axes.traverse(object => {
      object.geometry?.dispose();
      object.material?.map?.dispose();
      object.material?.dispose();
    });
    this.grid.geometry.dispose(); this.grid.material.dispose(); this.renderer.dispose();
  }
}
