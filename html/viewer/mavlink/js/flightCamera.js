import { bodyToNed } from './geo.js';

// The aircraft's camera looks along body +X (forward), with body -Z as up.
// MAVLink attitude is FRD -> NED; Cesium camera direction/up are Earth-fixed.
// Deriving both vectors also preserves roll without treating MAVLink yaw as a
// Cesium heading, whose local frame uses different axes.
export function fpvPose(C, position, attitude) {
  if (!position || ![position.x, position.y, position.z].every(Number.isFinite) ||
      C.Cartesian3.magnitudeSquared(position) < 1) return null;
  const rotation = bodyToNed(attitude);
  if (!rotation) return null;
  const frame = C.Transforms.northEastDownToFixedFrame(position);
  const direction = C.Matrix4.multiplyByPointAsVector(frame,
    new C.Cartesian3(rotation[0], rotation[3], rotation[6]), new C.Cartesian3());
  const up = C.Matrix4.multiplyByPointAsVector(frame,
    new C.Cartesian3(-rotation[2], -rotation[5], -rotation[8]), new C.Cartesian3());
  C.Cartesian3.normalize(direction, direction);
  C.Cartesian3.normalize(up, up);
  return { destination: C.Cartesian3.clone(position), orientation: { direction, up } };
}

export class FlightCamera {
  constructor(map) {
    this.map = map;
    this.enabled = false;
    this.lastPose = null;
    this.savedSettings = null;
  }

  setEnabled(enabled) {
    enabled = enabled === true;
    if (enabled === this.enabled) return;
    const { C, viewer } = this.map;
    const camera = viewer.camera;
    const controls = viewer.scene.screenSpaceCameraController;
    if (enabled) {
      this.savedSettings = {
        enableInputs: controls.enableInputs,
        enableCollisionDetection: controls.enableCollisionDetection,
        frustum: camera.frustum,
        near: camera.frustum.near,
        fov: camera.frustum.fov,
      };
      // Terrain collision correction otherwise lifts the camera away from a
      // low aircraft. Only telemetry moves the camera while FPV is selected.
      controls.enableInputs = false;
      controls.enableCollisionDetection = false;
      camera.cancelFlight();
      camera.lookAtTransform(C.Matrix4.IDENTITY);
      camera.frustum.near = 0.1;
      if (Number.isFinite(camera.frustum.fov)) camera.frustum.fov = C.Math.toRadians(70);
    } else {
      const saved = this.savedSettings;
      controls.enableInputs = saved.enableInputs;
      controls.enableCollisionDetection = saved.enableCollisionDetection;
      saved.frustum.near = saved.near;
      if (Number.isFinite(saved.fov)) saved.frustum.fov = saved.fov;
      this.savedSettings = null;
      this.lastPose = null;
    }
    this.enabled = enabled;
    viewer.scene.requestRender();
  }

  update(position, attitude, fresh) {
    if (!this.enabled || !fresh) return false;
    const pose = fpvPose(this.map.C, position, attitude);
    if (!pose) return false;
    this.map.viewer.camera.setView(pose);
    this.lastPose = pose;
    this.map.viewer.scene.requestRender();
    return true;
  }

  reset() {
    // A stream reset must not invent a new camera pose. Resume when a complete,
    // fresh aircraft position and attitude arrive on the replacement stream.
    this.lastPose = null;
  }
}
