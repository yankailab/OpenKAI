import * as THREE from '../vendor/three.module.min.js';
import { STREAM_TYPES } from './protocol.js';
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
    this.sensor = new THREE.Group();
    this.sensor.name = 'SLAM sensor pose';
    this.sensor.visible = false;
    this.sensor.add(new THREE.AxesHelper(0.35));
    this.sensorBody = new THREE.Mesh(new THREE.BoxGeometry(0.12, 0.08, 0.08),
      new THREE.MeshBasicMaterial({ color: 0xffc56b, depthTest: false, depthWrite: false }));
    this.sensor.add(this.sensorBody);
    this.frustum = new THREE.LineSegments(new THREE.BufferGeometry(),
      new THREE.LineBasicMaterial({ color: 0xffc56b, depthTest: false, depthWrite: false }));
    this.sensor.add(this.frustum);
    this.sensorBody.renderOrder = this.frustum.renderOrder = 2;
    this.scene.add(this.sensor);
    this.showSensor = true;
    this.followSensor = false;
    this.sensorValid = false;
    this.lastSensorPosition = null;
    this.objects = new Map();
    // Shared uniforms recolor every point on the GPU as the viewing eye moves.
    this.pointColorUniforms = {
      eyeColorEnabled: { value: 1 },
      eyeColorRange: { value: new THREE.Vector2(0, 1) },
      eyeColorPalette: { value: [0xff603d, 0xffda59, 0x57e39b, 0x00c6ef, 0x3055ff].map(hex => new THREE.Color(hex)) }
    };
    this.hasDistanceRange = false;
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
  configureSensor(horizontal, vertical, range, axis) {
    const halfX = Math.tan(THREE.MathUtils.degToRad(horizontal / 2)) * range;
    const halfY = Math.tan(THREE.MathUtils.degToRad(vertical / 2)) * range;
    const corners = [new THREE.Vector3(-halfX, -halfY, range), new THREE.Vector3(halfX, -halfY, range),
      new THREE.Vector3(halfX, halfY, range), new THREE.Vector3(-halfX, halfY, range)];
    const direction = { x: [1, 0, 0], y: [0, 1, 0], z: [0, 0, 1], '-x': [-1, 0, 0], '-y': [0, -1, 0], '-z': [0, 0, -1] }[axis];
    const rotation = new THREE.Quaternion().setFromUnitVectors(new THREE.Vector3(0, 0, 1), new THREE.Vector3(...direction));
    corners.forEach(c => c.applyQuaternion(rotation));
    const vertices = [];
    for (let i = 0; i < 4; ++i) vertices.push(0, 0, 0, ...corners[i].toArray(), ...corners[i].toArray(), ...corners[(i + 1) % 4].toArray());
    this.frustum.geometry.dispose();
    this.frustum.geometry = new THREE.BufferGeometry();
    this.frustum.geometry.setAttribute('position', new THREE.Float32BufferAttribute(vertices, 3));
  }
  setSensorPose(status) {
    const finite = (v, size) => Array.isArray(v) && v.length === size && v.every(Number.isFinite);
    this.sensorValid = Boolean(status?.poseValid && finite(status.position, 3) && finite(status.orientation, 4));
    if (!this.sensorValid) { this.sensor.visible = false; this.lastSensorPosition = null; return; }
    this.sensor.position.fromArray(status.position);
    this.sensor.quaternion.fromArray(status.orientation).normalize();
    if (this.followSensor && this.lastSensorPosition) {
      const delta = this.sensor.position.clone().sub(this.lastSensorPosition);
      this.camera.position.add(delta); this.controls.target.add(delta);
    }
    this.lastSensorPosition = this.sensor.position.clone();
    const color = status.poseFresh ? 0xffc56b : 0x8799ad;
    this.sensorBody.material.color.setHex(color); this.frustum.material.color.setHex(color);
  }
  markSensorStale() {
    this.sensorBody.material.color.setHex(0x8799ad); this.frustum.material.color.setHex(0x8799ad);
  }
  configure(config) {
    this.clear();
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
    const object = { id, streams: new Map(), distanceSamples: new Float32Array(0) };
    const material = new THREE.PointsMaterial({ vertexColors: true, sizeAttenuation: false });
    material.onBeforeCompile = shader => {
      Object.assign(shader.uniforms, this.pointColorUniforms);
      shader.vertexShader = 'varying float eyeDistance;\n' + shader.vertexShader.replace(
        '#include <project_vertex>', '#include <project_vertex>\neyeDistance = length(mvPosition.xyz);');
      shader.fragmentShader = `
        uniform float eyeColorEnabled;
        uniform vec2 eyeColorRange;
        uniform vec3 eyeColorPalette[5];
        varying float eyeDistance;
      ` + shader.fragmentShader.replace('#include <color_fragment>', `
        #include <color_fragment>
        if (eyeColorEnabled > 0.5) {
          float t = 4.0 * clamp((eyeDistance - eyeColorRange.x) / (eyeColorRange.y - eyeColorRange.x), 0.0, 1.0);
          vec3 ramp = mix(eyeColorPalette[0], eyeColorPalette[1], clamp(t, 0.0, 1.0));
          ramp = mix(ramp, eyeColorPalette[2], clamp(t - 1.0, 0.0, 1.0));
          ramp = mix(ramp, eyeColorPalette[3], clamp(t - 2.0, 0.0, 1.0));
          diffuseColor.rgb = mix(ramp, eyeColorPalette[4], clamp(t - 3.0, 0.0, 1.0));
        }
      `);
    };
    material.customProgramCacheKey = () => 'glim-eye-distance-v1';
    object.points = new THREE.Points(new THREE.BufferGeometry(), material);
    object.lines = new THREE.LineSegments(new THREE.BufferGeometry(), new THREE.LineBasicMaterial({ vertexColors: true }));
    this.scene.add(object.points, object.lines);
    this.objects.set(id, object);
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
      geometry.setAttribute('color', new THREE.BufferAttribute(new Uint8Array(capacity * 3), 3, true).setUsage(THREE.DynamicDrawUsage));
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
      const mesh = o[type];
      if (type === 'points') {
        mesh.material.size = data.pointSize;
        // Estimate the color range from a bounded sample, not every map point
        // on every animation frame. Streamed positions/colors remain untouched.
        const stride = Math.max(1, Math.ceil(data.count / 512));
        o.distanceSamples = new Float32Array(Math.ceil(data.count / stride) * 3);
        for (let i = 0, at = 0; i < data.count; i += stride, at += 3)
          o.distanceSamples.set(data.positions.subarray(i * 3, i * 3 + 3), at);
      }
      this.upload(mesh, data.positions, data.colors, data.bounds);
    }
    for (const o of this.objects.values()) if (o.streams.has(type) && !active.has(o.id)) this.removeStream(o, type);
    this.updateBounds();
    if (this.autoBound && !this.boundTypes.has(type) && frame.objects.some(o => o.count > 0)) {
      this.fit(); this.boundTypes.add(type);
    }
  }
  removeStream(o, type) {
    o.streams.delete(type);
    if (type === 'points') o.distanceSamples = new Float32Array(0);
    o[type].geometry.setDrawRange(0, 0);
    if (!o.streams.size) this.removeObject(o);
  }
  clearStream(type) {
    for (const o of this.objects.values()) if (o.streams.has(type)) this.removeStream(o, type);
    this.updateBounds();
  }
  updateBounds() {
    this.bounds.makeEmpty();
    for (const o of this.objects.values()) for (const data of o.streams.values()) if (data.count)
      this.bounds.union(new THREE.Box3(new THREE.Vector3(...data.bounds.slice(0, 3)), new THREE.Vector3(...data.bounds.slice(3))));
  }
  fit() {
    const bounds = this.bounds.clone();
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
  setPointColorMode(mode) {
    this.pointColorUniforms.eyeColorEnabled.value = mode === 'distance' ? 1 : 0;
  }
  updateDistanceRange() {
    let near = Infinity, far = -Infinity;
    const eye = this.camera.position;
    for (const o of this.objects.values()) {
      const samples = o.distanceSamples;
      for (let i = 0; i < samples.length; i += 3) {
        const distance = Math.hypot(samples[i] - eye.x, samples[i + 1] - eye.y, samples[i + 2] - eye.z);
        near = Math.min(near, distance); far = Math.max(far, distance);
      }
    }
    this.hasDistanceRange = Number.isFinite(near) && Number.isFinite(far);
    if (this.hasDistanceRange) {
      // Keep a finite gradient for a single point or an equidistant surface.
      const span = Math.max(far - near, 0.01);
      this.pointColorUniforms.eyeColorRange.value.set(Math.max(0, (near + far - span) / 2), (near + far + span) / 2);
    }
  }
  render() {
    this.sensor.visible = this.showSensor && this.sensorValid;
    this.controls.update();
    this.scene.updateMatrixWorld();
    this.camera.updateMatrixWorld();
    if (this.pointColorUniforms.eyeColorEnabled.value) this.updateDistanceRange();
    this.renderer.render(this.scene, this.camera);
  }
  removeObject(o) {
    for (const mesh of [o.points, o.lines]) { this.scene.remove(mesh); mesh.geometry.dispose(); mesh.material.dispose(); }
    this.objects.delete(o.id);
  }
  clear() { for (const o of this.objects.values()) this.removeObject(o); this.bounds.makeEmpty(); this.hasDistanceRange = false; }
  dispose() {
    this.clear(); this.resizeObserver.disconnect(); this.controls.dispose();
    this.axes.traverse(object => {
      object.geometry?.dispose();
      object.material?.map?.dispose();
      object.material?.dispose();
    });
    this.sensor.traverse(object => { object.geometry?.dispose(); object.material?.dispose(); });
    this.grid.geometry.dispose(); this.grid.material.dispose(); this.renderer.dispose();
  }
}
