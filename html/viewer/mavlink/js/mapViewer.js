import { assetUrl, finite, fresh, normalizeConfig, validPosition } from './protocol.js';
import { modelToNed, resolveAltitude } from './geo.js';
import { MapSources } from './mapSources.js';
import { FlightCamera } from './flightCamera.js';

export class MapViewer {
  constructor(container, onStatus, onHeight) {
    const C = this.C = globalThis.Cesium;
    if (!C) throw new Error('Local CesiumJS library is missing');
    C.Ion.defaultAccessToken = '';
    this.onStatus = onStatus;
    this.onHeight = onHeight;
    this.config = normalizeConfig();
    this.generation = 0;
    this.buildings = [];
    this.credits = [];
    this.trailPositions = [];
    this.geoidCache = {};
    this.lastPlot = null;
    this.follow = false;
    this.fpv = false;
    this.lastAttitude = null;
    this.lastPoseFresh = false;
    this.showTrail = true;
    this.viewer = new C.Viewer(container, {
      baseLayer: false, baseLayerPicker: false, terrainProvider: new C.EllipsoidTerrainProvider(),
      geocoder: false, homeButton: false, sceneModePicker: false, navigationHelpButton: false,
      animation: false, timeline: false, fullscreenButton: false, selectionIndicator: false,
      infoBox: false, scene3DOnly: true, requestRenderMode: true, maximumRenderTimeChange: Infinity,
    });
    const scene = this.viewer.scene;
    const controls = scene.screenSpaceCameraController;
    // In Cesium's 3D view, rotate pans the globe and tilt orbits the picked point.
    controls.rotateEventTypes = [C.CameraEventType.RIGHT_DRAG];
    controls.translateEventTypes = [C.CameraEventType.RIGHT_DRAG];
    controls.tiltEventTypes = [C.CameraEventType.LEFT_DRAG, C.CameraEventType.PINCH];
    controls.zoomEventTypes = [C.CameraEventType.WHEEL, C.CameraEventType.PINCH];
    scene.globe.baseColor = C.Color.fromCssColorString('#172c3c');
    scene.globe.depthTestAgainstTerrain = true;
    scene.globe.enableLighting = false;
    this.marker = this.viewer.entities.add({
      show: false,
      point: { pixelSize: 9, color: C.Color.fromCssColorString('#64dcb5'), outlineWidth: 2, outlineColor: C.Color.BLACK, disableDepthTestDistance: Infinity },
      label: { text: 'Aircraft', font: '12px sans-serif', showBackground: true, pixelOffset: new C.Cartesian2(0, -24), disableDepthTestDistance: Infinity },
    });
    this.trailEntity = this.viewer.entities.add({ polyline: { positions: new C.CallbackProperty(() => this.trailPositions, false), width: 2, material: C.Color.fromCssColorString('#0047ab') } });
    scene.renderError.addEventListener((_scene, error) => onStatus('renderer', `Rendering error: ${error.message}`, true));
    this.flightCamera = new FlightCamera(this);
    this.mapSources = new MapSources(this);
    this.mapSources.configure(this.config, location.href);
    this.goHome();
  }

  async configure(raw, endpoint) {
    const C = this.C, generation = ++this.generation;
    const config = this.config = normalizeConfig(raw);
    this.geoidCache = {};
    this.lastPlot = null;
    const scene = this.viewer.scene;
    for (const tileset of this.buildings) scene.primitives.remove(tileset);
    this.buildings = [];
    if (this.model) scene.primitives.remove(this.model);
    this.model = null;
    for (const credit of this.credits) this.viewer.creditDisplay.removeStaticCredit(credit);
    this.credits = [];
    this.viewer.terrainProvider = new C.EllipsoidTerrainProvider();
    this.clearTrail();
    this.mapSources.configure(config, endpoint);
    const tasks = [];
    const current = () => generation === this.generation;
    const addCredit = text => {
      if (typeof text !== 'string' || !text) return;
      const node = document.createElement('span'); node.textContent = text;
      const credit = new C.Credit(node.innerHTML, true);
      this.viewer.creditDisplay.addStaticCredit(credit); this.credits.push(credit);
    };

    if (config.terrain?.url) tasks.push((async () => {
      try {
        const provider = await C.CesiumTerrainProvider.fromUrl(assetUrl(config.terrain.url, endpoint), { requestVertexNormals: true });
        if (!current()) return;
        this.viewer.terrainProvider = provider;
        provider.errorEvent.addEventListener(() => this.onStatus('terrain', 'Terrain tiles unavailable in this area', true));
        this.onStatus('terrain', 'Terrain · configured height tiles');
      } catch (error) { if (current()) this.onStatus('terrain', `Terrain unavailable: ${error.message}`, true); }
    })());
    else this.onStatus('terrain', 'Terrain · WGS84 ellipsoid (no elevation tiles)');

    if (!config.buildings.length) this.onStatus('buildings', 'PLATEAU buildings · not configured');
    for (const [index, settings] of config.buildings.entries()) tasks.push((async () => {
      const key = `buildings${index}`;
      this.onStatus(key, `PLATEAU buildings ${index + 1} · loading…`);
      try {
        const rendering = config.buildingRendering;
        // Keep detailed tiles visible farther away and across the viewport, including
        // while following the aircraft. The cache remains bounded per tileset.
        const tileset = await C.Cesium3DTileset.fromUrl(assetUrl(typeof settings === 'string' ? settings : settings.url, endpoint), {
          maximumScreenSpaceError: rendering.maximumScreenSpaceError,
          dynamicScreenSpaceError: rendering.dynamicScreenSpaceError,
          foveatedScreenSpaceError: rendering.foveatedScreenSpaceError,
          cullRequestsWhileMoving: rendering.cullRequestsWhileMoving,
          cacheBytes: rendering.cacheMegabytes * 1024 * 1024,
          maximumCacheOverflowBytes: rendering.maximumCacheOverflowMegabytes * 1024 * 1024,
          showCreditsOnScreen: true,
        });
        if (!current()) { tileset.destroy(); return; }
        tileset.show = true;
        scene.primitives.add(tileset); this.buildings.push(tileset); addCredit(settings.credit);
        tileset.tileFailed.addEventListener(() => this.onStatus(key, `PLATEAU buildings ${index + 1} · some tile content is missing`, true));
        this.onStatus(key, `PLATEAU buildings ${index + 1} · ready`);
      } catch (error) { if (current()) this.onStatus(key, `PLATEAU buildings ${index + 1} unavailable: ${error.message}`, true); }
    })());

    tasks.push((async () => {
      this.onStatus('model', 'Drone model · loading…');
      try {
        const model = await C.Model.fromGltfAsync({
          url: assetUrl(config.drone.url, endpoint), show: false,
          // Keep raw glTF coordinates so the explicit MODEL_TO_FRD matrix applies
          // identically here and in Three.js. Cesium's default axis fix is bypassed.
          upAxis: C.Axis.Z, forwardAxis: C.Axis.X,
          scale: finite(config.drone.scale) && config.drone.scale > 0 ? config.drone.scale : 1,
          minimumPixelSize: 80, maximumScale: 200,
        });
        if (!current()) { model.destroy(); return; }
        this.model = scene.primitives.add(model);
        model.errorEvent.addEventListener(error => this.onStatus('model', `Drone model error: ${error.message}`, true));
        this.onStatus('model', 'Drone model · ready (enlarged at distance)');
      } catch (error) { if (current()) this.onStatus('model', `Drone model unavailable: ${error.message}`, true); }
    })());
    if (!this.fpv) this.goHome();
    await Promise.allSettled(tasks);
    if (current()) scene.requestRender();
  }

  update(message, elapsedMs) {
    const C = this.C, config = this.config, position = message?.position;
    const positionFresh = validPosition(position) && fresh(position, elapsedMs, config.staleAfterMs);
    const attitudeFresh = fresh(message?.attitude, elapsedMs, config.staleAfterMs);
    if (!validPosition(position)) {
      if (this.model) this.model.show = false;
      this.marker.show = false;
      this.lastPosition = null;
      this.lastAttitude = null;
      this.lastPoseFresh = false;
      this.flightCamera.update(null, null, false);
      if (this.fpv) this.onStatus('camera', 'FPV · waiting for fresh position and attitude');
      this.viewer.scene.requestRender();
      return false;
    }
    const previous = this.lastPlot;
    const samePosition = previous && previous.latitudeDeg === position.latitudeDeg && previous.longitudeDeg === position.longitudeDeg && previous.altitudeMslM === position.altitudeMslM;
    const altitude = !positionFresh && samePosition ? previous.altitude : resolveAltitude(position, message.gps, config, elapsedMs, this.geoidCache);
    this.lastPlot = { latitudeDeg: position.latitudeDeg, longitudeDeg: position.longitudeDeg, altitudeMslM: position.altitudeMslM, altitude };
    this.onHeight(altitude.source, altitude.approximate);
    const cartesian = C.Cartesian3.fromDegrees(position.longitudeDeg, position.latitudeDeg, altitude.heightM);
    this.lastPosition = cartesian;
    this.marker.position = cartesian;
    this.marker.show = !this.fpv;
    this.marker.point.color = C.Color.fromCssColorString(positionFresh ? '#64dcb5' : '#edbb78');
    this.marker.label.text = positionFresh ? (attitudeFresh ? 'Aircraft' : 'Aircraft · attitude unavailable') : 'Last known position';
    const rotation = attitudeFresh ? modelToNed(message.attitude) : null;
    this.lastAttitude = message.attitude;
    this.lastPoseFresh = positionFresh && !!rotation;
    this.flightCamera.update(cartesian, message.attitude, this.lastPoseFresh);
    if (this.fpv) this.onStatus('camera', this.lastPoseFresh ? 'FPV · live aircraft position and attitude' : 'FPV · holding last view; position or attitude is stale');
    if (this.model) {
      this.model.show = positionFresh && !!rotation && !this.fpv;
      if (this.model.show) {
        const frame = C.Transforms.northEastDownToFixedFrame(cartesian);
        const local = C.Matrix4.fromRotationTranslation(C.Matrix3.fromRowMajorArray(rotation));
        this.model.modelMatrix = C.Matrix4.multiply(frame, local, new C.Matrix4());
      }
    }
    this.marker.point.show = !(this.model?.ready && this.model.show);
    if (positionFresh) {
      const previous = this.trailPositions.at(-1);
      if (previous && C.Cartesian3.distance(previous, cartesian) > 2000) this.clearTrail();
      if (!previous || C.Cartesian3.distance(previous, cartesian) > .1) {
        this.trailPositions.push(cartesian);
        if (this.trailPositions.length > config.trailMaxPoints) this.trailPositions.shift();
      }
      if (this.follow) this.locate();
    }
    this.viewer.scene.requestRender();
    return positionFresh;
  }

  reset() {
    this.geoidCache = {};
    this.lastPlot = null;
    this.lastPosition = null;
    this.lastAttitude = null;
    this.lastPoseFresh = false;
    this.flightCamera.reset();
    this.marker.show = false;
    if (this.model) this.model.show = false;
    this.clearTrail();
  }
  clearTrail() { this.trailPositions.length = 0; this.viewer.scene.requestRender(); }
  setTrail(show) { this.showTrail = show; this.trailEntity.show = show; this.viewer.scene.requestRender(); }
  setFollow(follow) {
    if (follow) this.setFpv(false);
    this.follow = follow;
    if (follow) this.locate();
    else this.viewer.camera.lookAtTransform(this.C.Matrix4.IDENTITY);
    if (!this.fpv) this.onStatus('camera', follow ? 'Follow · tracking aircraft position' : 'Camera · orbit and pan');
    this.viewer.scene.requestRender();
  }
  setFpv(enabled) {
    if (this.fpv === enabled) return;
    if (enabled) this.setFollow(false);
    this.fpv = enabled;
    this.flightCamera.setEnabled(enabled);
    if (enabled) {
      this.marker.show = false;
      if (this.model) this.model.show = false;
      this.flightCamera.update(this.lastPosition, this.lastAttitude, this.lastPoseFresh);
      this.onStatus('camera', this.lastPoseFresh ? 'FPV · live aircraft position and attitude' : 'FPV · waiting for fresh position and attitude');
    } else {
      if (this.lastPosition) this.locate();
      this.onStatus('camera', 'Camera · orbit and pan');
    }
    this.viewer.scene.requestRender();
  }
  locate() {
    if (!this.lastPosition) return;
    const C = this.C;
    this.viewer.camera.lookAt(this.lastPosition, new C.HeadingPitchRange(0, -Math.PI / 4, 800));
    if (!this.follow) this.viewer.camera.lookAtTransform(C.Matrix4.IDENTITY);
    this.viewer.scene.requestRender();
  }
  goHome() {
    const C = this.C, view = this.config.initialView;
    this.viewer.camera.lookAtTransform(C.Matrix4.IDENTITY);
    this.viewer.camera.setView({ destination: C.Cartesian3.fromDegrees(view.longitude, view.latitude, view.height), orientation: { heading: C.Math.toRadians(view.heading), pitch: C.Math.toRadians(view.pitch), roll: 0 } });
    this.viewer.scene.requestRender();
  }
}
