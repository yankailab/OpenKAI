import * as THREE from '../vendor/three.module.min.js';
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
    this.pointCount = 0;
    this.session = null;
    this.assembling = null;
    this.trajectorySession = null;
    this.trajectoryTimestamp = null;
    this.trajectoryLast = null;
    this.trajectoryNext = this.trajectoryCount = 0;
    this.trajectoryCapacity = 8192;
    this.trajectoryPositions = new Float32Array(this.trajectoryCapacity * 6);
    const trajectoryGeometry = new THREE.BufferGeometry();
    trajectoryGeometry.setAttribute('position', new THREE.BufferAttribute(this.trajectoryPositions, 3).setUsage(THREE.DynamicDrawUsage));
    trajectoryGeometry.setDrawRange(0, 0);
    this.trajectory = new THREE.LineSegments(trajectoryGeometry,
      new THREE.LineBasicMaterial({ color: 0xffc56b, transparent: true, opacity: 0.8, depthTest: false, depthWrite: false }));
    this.trajectory.name = 'Camera trajectory';
    this.trajectory.frustumCulled = false;
    this.trajectory.renderOrder = 1;
    this.scene.add(this.trajectory);
    // Shared uniforms recolor every point on the GPU as the viewing eye moves.
    this.pointColorUniforms = {
      eyeColorEnabled: { value: 1 },
      eyeColorRange: { value: new THREE.Vector2(0, 1) },
      eyeColorPalette: { value: [0xff603d, 0xffda59, 0x57e39b, 0x00c6ef, 0x3055ff].map(hex => new THREE.Color(hex)) }
    };
    this.hasDistanceRange = false;
    this.bounds = new THREE.Box3();
    this.didAutoFit = false;
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
    if (status?.session !== undefined && String(status.session) !== this.trajectorySession) {
      this.clearTrajectory(); this.trajectorySession = String(status.session);
    }
    this.sensorValid = Boolean(status?.poseValid && finite(status.position, 3) && finite(status.orientation, 4));
    if (!this.sensorValid) { this.sensor.visible = false; this.lastSensorPosition = null; this.trajectoryLast = null; return; }
    this.appendTrajectory(status);
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
    this.trajectoryLast = null; this.lastSensorPosition = null;
    this.sensorBody.material.color.setHex(0x8799ad); this.frustum.material.color.setHex(0x8799ad);
  }
  configure(config) {
    this.clear();
    this.config = config;
    this.scene.background.fromArray(config.background);
    this.grid.visible = config.showGrid;
    this.grid.quaternion.setFromUnitVectors(new THREE.Vector3(0, 1, 0), new THREE.Vector3(...config.camera.up).normalize());
    this.autoBound = config.autoBound;
    this.didAutoFit = false;
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
    const object = { id, count: 0, localBounds: new THREE.Box3(), distanceSamples: new Float32Array(0) };
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
    object.points.matrixAutoUpdate = false;
    this.scene.add(object.points);
    this.objects.set(id, object);
    return object;
  }
  update(event) {
    if (event.type === 'reset') {
      this.clear(); this.session = event.session;
      if (this.trajectorySession !== event.session) {
        this.clearTrajectory(); this.trajectorySession = event.session;
      }
      return;
    }
    if (event.session !== this.session) throw new Error('Submap belongs to a different SLAM session');
    if (event.type === 'pose') {
      const object = this.objects.get(event.id);
      if (object) { object.points.matrix.fromArray(event.pose); object.points.matrixWorldNeedsUpdate = true; this.updateBounds(); }
      else if (this.assembling?.id === event.id) this.assembling.pose = event.pose;
      else throw new Error('Pose update refers to an unknown submap');
      return;
    }
    if (event.type === 'submap') {
      if (this.assembling || this.objects.has(event.id)) throw new Error('Duplicate or unfinished submap');
      this.assembling = { ...event, received: 0, positions: new Float32Array(event.pointCount * 3) };
      if (!event.pointCount) this.finishSubmap();
      return;
    }
    const submap = this.assembling;
    if (event.type !== 'chunk' || !submap || event.id !== submap.id || event.timestampNs !== submap.timestampNs ||
        event.totalPoints !== submap.pointCount || event.offsetPoints !== submap.received)
      throw new Error('Unexpected or out-of-order submap chunk');
    submap.positions.set(event.positions, event.offsetPoints * 3);
    submap.received += event.countPoints;
    if (submap.received === submap.pointCount) this.finishSubmap();
  }
  finishSubmap() {
    const data = this.assembling; this.assembling = null;
    const object = this.createObject(data.id), mesh = object.points;
    object.count = data.pointCount;
    // Immutable local geometry is uploaded once. Graph optimization updates only its matrix.
    mesh.geometry.setAttribute('position', new THREE.BufferAttribute(data.positions, 3));
    const colors = new Uint8Array(data.pointCount * 3);
    const color = new THREE.Color().setHSL((Number(BigInt(data.id) % 29n) * 0.618034) % 1, 0.55, 0.6);
    for (let i = 0; i < colors.length; i += 3) {
      colors[i] = Math.round(color.r * 255); colors[i + 1] = Math.round(color.g * 255); colors[i + 2] = Math.round(color.b * 255);
    }
    mesh.geometry.setAttribute('color', new THREE.BufferAttribute(colors, 3, true));
    mesh.geometry.computeBoundingBox(); mesh.geometry.computeBoundingSphere();
    object.localBounds.copy(mesh.geometry.boundingBox);
    mesh.material.size = this.config?.pointSize || 2;
    mesh.matrix.fromArray(data.pose); mesh.matrixWorldNeedsUpdate = true;
    const stride = Math.max(1, Math.ceil(data.pointCount / 256));
    object.distanceSamples = new Float32Array(Math.ceil(data.pointCount / stride) * 3);
    for (let i = 0, at = 0; i < data.pointCount; i += stride, at += 3)
      object.distanceSamples.set(data.positions.subarray(i * 3, i * 3 + 3), at);
    this.pointCount += data.pointCount;
    this.updateBounds();
    if (this.autoBound && !this.didAutoFit && this.pointCount) { this.fit(); this.didAutoFit = true; }
  }
  updateBounds() {
    this.bounds.makeEmpty();
    for (const object of this.objects.values()) if (object.count)
      this.bounds.union(object.localBounds.clone().applyMatrix4(object.points.matrix));
  }
  appendTrajectory(status) {
    if (!status.poseFresh || status.poseTimestampNs === undefined) { this.trajectoryLast = null; return; }
    const timestamp = BigInt(status.poseTimestampNs);
    if (this.trajectoryTimestamp !== null && timestamp <= this.trajectoryTimestamp) return;
    this.trajectoryTimestamp = timestamp;
    if (this.trajectoryLast) {
      const at = this.trajectoryNext * 6;
      this.trajectoryPositions.set(this.trajectoryLast, at);
      this.trajectoryPositions.set(status.position, at + 3);
      const attribute = this.trajectory.geometry.getAttribute('position');
      attribute.addUpdateRange(at, 6); attribute.needsUpdate = true;
      this.trajectoryNext = (this.trajectoryNext + 1) % this.trajectoryCapacity;
      this.trajectoryCount = Math.min(this.trajectoryCount + 1, this.trajectoryCapacity);
      this.trajectory.geometry.setDrawRange(0, this.trajectoryCount * 2);
    }
    this.trajectoryLast = status.position.slice();
  }
  clearTrajectory() {
    this.trajectoryTimestamp = this.trajectoryLast = null;
    this.trajectoryCount = this.trajectoryNext = 0;
    this.trajectory.geometry.setDrawRange(0, 0);
    this.trajectory.geometry.getAttribute('position').clearUpdateRanges();
    this.lastSensorPosition = null;
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
    // Keep color-scale work bounded even when a long session contains many submaps.
    const objectStride = Math.max(1, Math.ceil(this.objects.size / 32));
    const sample = new THREE.Vector3();
    let objectIndex = 0;
    for (const object of this.objects.values()) {
      if (objectIndex++ % objectStride) continue;
      const samples = object.distanceSamples;
      for (let i = 0; i < samples.length; i += 3) {
        sample.fromArray(samples, i).applyMatrix4(object.points.matrix);
        const distance = sample.distanceTo(eye);
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
  removeObject(object) {
    this.scene.remove(object.points); object.points.geometry.dispose(); object.points.material.dispose();
    this.objects.delete(object.id);
  }
  clear() {
    for (const object of this.objects.values()) this.removeObject(object);
    this.assembling = null; this.session = null; this.pointCount = 0; this.didAutoFit = false;
    this.bounds.makeEmpty(); this.hasDistanceRange = false;
  }
  dispose() {
    this.clear(); this.resizeObserver.disconnect(); this.controls.dispose();
    this.axes.traverse(object => {
      object.geometry?.dispose();
      object.material?.map?.dispose();
      object.material?.dispose();
    });
    this.sensor.traverse(object => { object.geometry?.dispose(); object.material?.dispose(); });
    this.trajectory.geometry.dispose(); this.trajectory.material.dispose();
    this.grid.geometry.dispose(); this.grid.material.dispose(); this.renderer.dispose();
  }
}
