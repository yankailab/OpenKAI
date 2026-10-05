import * as THREE from 'three';
import { GLTFLoader } from '../vendor/GLTFLoader.js';
import { modelToThree } from './geo.js';

export class AttitudeViewer {
  constructor(container, onStatus) {
    this.container = container;
    this.onStatus = onStatus;
    this.generation = 0;
    this.scene = new THREE.Scene();
    this.camera = new THREE.PerspectiveCamera(35, 1, .01, 100);
    this.camera.position.set(2.9, 2, 3.2);
    this.viewDirection = this.camera.position.clone().normalize();
    this.modelRadius = 1;
    this.camera.lookAt(0, 0, 0);
    this.renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
    this.renderer.outputColorSpace = THREE.SRGBColorSpace;
    container.append(this.renderer.domElement);
    this.scene.add(new THREE.HemisphereLight(0xcdeeff, 0x27313b, 3));
    const light = new THREE.DirectionalLight(0xffffff, 3); light.position.set(3, 5, 2); this.scene.add(light);
    this.grid = new THREE.GridHelper(3, 6, 0x426e79, 0x203642); this.scene.add(this.grid);
    this.northArrow = new THREE.ArrowHelper(new THREE.Vector3(0, 0, -1), new THREE.Vector3(), 1.35, 0x64dcb5, .16, .1);
    this.scene.add(this.northArrow);
    this.resizeObserver = new ResizeObserver(() => this.resize());
    this.resizeObserver.observe(container);
    this.resize();
  }
  async load(url) {
    const generation = ++this.generation;
    this.lastRotation = null;
    if (this.model) { this.scene.remove(this.model); this.disposeModel(this.model); this.model = null; }
    this.render();
    try {
      const gltf = await new GLTFLoader().loadAsync(url);
      if (generation !== this.generation) { this.disposeModel(gltf.scene); return; }
      this.model = gltf.scene;
      const bounds = new THREE.Box3().setFromObject(this.model).getBoundingSphere(new THREE.Sphere());
      // Fit the entire assembly around its flight pivot, including payload and
      // every attitude, without changing the model's physical scale on the map.
      const radius = bounds.radius + bounds.center.length();
      if (Number.isFinite(radius) && radius > 0) this.modelRadius = radius;
      this.fitModel();
      this.model.matrixAutoUpdate = false;
      this.model.visible = false;
      this.scene.add(this.model);
      this.onStatus('attitude', 'Attitude preview · Three.js ready');
      this.render();
    } catch (error) { if (generation === this.generation) this.onStatus('attitude', `Attitude model unavailable: ${error.message}`, true); }
  }
  update(attitude, isFresh) {
    const rotation = modelToThree(attitude);
    let changed = false;
    if (this.model) {
      changed = this.model.visible !== !!rotation || (!!rotation && (!this.lastRotation || rotation.some((value, index) => value !== this.lastRotation[index])));
      this.model.visible = !!rotation;
      if (rotation && changed) this.model.matrix.set(rotation[0], rotation[1], rotation[2], 0, rotation[3], rotation[4], rotation[5], 0, rotation[6], rotation[7], rotation[8], 0, 0, 0, 0, 1);
      this.lastRotation = rotation;
    }
    this.container.dataset.stale = String(!isFresh);
    // Repeated telemetry with the same attitude needs no extra CAD render.
    if (changed) this.render();
  }
  resize() {
    const width = this.container.clientWidth, height = this.container.clientHeight;
    if (!width || !height) return;
    this.renderer.setSize(width, height);
    this.camera.aspect = width / height;
    this.fitModel();
    this.render();
  }
  fitModel() {
    const vertical = THREE.MathUtils.degToRad(this.camera.fov) / 2;
    const horizontal = Math.atan(Math.tan(vertical) * this.camera.aspect);
    const distance = this.modelRadius * 1.12 / Math.sin(Math.min(vertical, horizontal));
    this.camera.position.copy(this.viewDirection).multiplyScalar(distance);
    this.camera.near = Math.max(.001, this.modelRadius / 1000);
    this.camera.far = Math.max(100, distance * 10);
    this.camera.lookAt(0, 0, 0);
    this.camera.updateProjectionMatrix();
    this.grid.scale.setScalar(this.modelRadius);
    this.grid.position.y = -this.modelRadius;
    this.northArrow.scale.setScalar(this.modelRadius);
    this.northArrow.position.y = -.98 * this.modelRadius;
  }
  render() { this.renderer.render(this.scene, this.camera); }
  disposeModel(model) {
    model.traverse(node => {
      node.geometry?.dispose();
      const materials = Array.isArray(node.material) ? node.material : [node.material];
      for (const material of materials) {
        if (!material) continue;
        for (const value of Object.values(material)) if (value?.isTexture) value.dispose();
        material.dispose();
      }
    });
  }
}
