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
    this.camera.lookAt(0, 0, 0);
    this.renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
    this.renderer.setPixelRatio(Math.min(window.devicePixelRatio || 1, 2));
    this.renderer.outputColorSpace = THREE.SRGBColorSpace;
    container.append(this.renderer.domElement);
    this.scene.add(new THREE.HemisphereLight(0xcdeeff, 0x27313b, 3));
    const light = new THREE.DirectionalLight(0xffffff, 3); light.position.set(3, 5, 2); this.scene.add(light);
    const grid = new THREE.GridHelper(3, 6, 0x426e79, 0x203642); grid.position.y = -.55; this.scene.add(grid);
    this.scene.add(new THREE.ArrowHelper(new THREE.Vector3(0, 0, -1), new THREE.Vector3(0, -.5, 0), 1.35, 0x64dcb5, .16, .1));
    this.resizeObserver = new ResizeObserver(() => this.resize());
    this.resizeObserver.observe(container);
    this.resize();
  }
  async load(url) {
    const generation = ++this.generation;
    if (this.model) { this.scene.remove(this.model); this.disposeModel(this.model); this.model = null; }
    try {
      const gltf = await new GLTFLoader().loadAsync(url);
      if (generation !== this.generation) { this.disposeModel(gltf.scene); return; }
      this.model = gltf.scene;
      this.model.matrixAutoUpdate = false;
      this.model.visible = false;
      this.scene.add(this.model);
      this.onStatus('attitude', 'Attitude preview · Three.js ready');
      this.render();
    } catch (error) { if (generation === this.generation) this.onStatus('attitude', `Attitude model unavailable: ${error.message}`, true); }
  }
  update(attitude, isFresh) {
    const rotation = modelToThree(attitude);
    if (this.model) {
      this.model.visible = !!rotation;
      if (rotation) this.model.matrix.set(rotation[0], rotation[1], rotation[2], 0, rotation[3], rotation[4], rotation[5], 0, rotation[6], rotation[7], rotation[8], 0, 0, 0, 0, 1);
    }
    this.container.dataset.stale = String(!isFresh);
    this.render();
  }
  resize() {
    const width = this.container.clientWidth, height = this.container.clientHeight;
    if (!width || !height) return;
    this.renderer.setSize(width, height);
    this.camera.aspect = width / height;
    this.camera.updateProjectionMatrix();
    this.render();
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
