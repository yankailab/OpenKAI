/* Dedicated, receive-only MAVLink viewer protocol. No GeometryBase packets. */
export const PROTOCOL = 'openkai.mavlink';
export const VERSION = 1;
export const STREAM_PATH = '/stream/mavlink';
export const WORLD_IMAGERY_URL = 'https://services.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer';
export const finite = value => typeof value === 'number' && Number.isFinite(value);

export function parseMessage(data) {
  if (typeof data !== 'string' || data.length > 262144) throw new Error('Expected a MAVLink viewer JSON text message');
  const message = JSON.parse(data);
  if (!message || message.protocol !== PROTOCOL || message.version !== VERSION)
    throw new Error('Unsupported MAVLink viewer protocol/version');
  if (message.type === 'hello') {
    if (message.readOnly !== true || !message.config || typeof message.config !== 'object' || Array.isArray(message.config))
      throw new Error('Invalid MAVLink viewer hello');
  } else if (message.type === 'telemetry') {
    if (!Number.isSafeInteger(message.sequence) || message.sequence < 0 || !finite(message.timeMs) || typeof message.connected !== 'boolean')
      throw new Error('Invalid telemetry envelope');
  } else throw new Error('Unknown MAVLink viewer message type');
  return message;
}

export function fresh(sample, elapsedMs = 0, staleAfterMs = 3000) {
  return !!sample && sample.stale === false && finite(sample.ageMs) && sample.ageMs >= 0 &&
    sample.ageMs + Math.max(0, elapsedMs) <= staleAfterMs;
}

export function validPosition(position) {
  return !!position && position.valid === true && finite(position.latitudeDeg) && Math.abs(position.latitudeDeg) <= 90 &&
    finite(position.longitudeDeg) && Math.abs(position.longitudeDeg) <= 180 && finite(position.altitudeMslM);
}

export function socketUrl(endpoint) {
  const url = new URL(STREAM_PATH, endpoint);
  if (!['http:', 'https:'].includes(url.protocol)) throw new Error('Expected an HTTP server endpoint');
  url.protocol = url.protocol === 'https:' ? 'wss:' : 'ws:';
  return url.href;
}

export function assetUrl(value, endpoint) {
  if (typeof value !== 'string' || !value.trim()) return null;
  const url = new URL(value, endpoint);
  if (!['http:', 'https:'].includes(url.protocol) || url.username || url.password)
    throw new Error('Map and model assets must use HTTP or HTTPS');
  // URL escapes braces, which Cesium needs as URL-template tokens.
  return url.href.replace(/%7B/gi, '{').replace(/%7D/gi, '}');
}

export function normalizeConfig(input = {}) {
  const view = input.initialView || {};
  const buildings = input.buildingRendering || {};
  const bounded = (value, fallback, min, max) => finite(value) ? Math.min(max, Math.max(min, value)) : fallback;
  const online = input.onlineImagery && typeof input.onlineImagery === 'object' && !Array.isArray(input.onlineImagery) ? input.onlineImagery : {};
  const onlineProvider = online.provider ?? 'arcgis';
  const onlineMaximumLevel = Math.floor(bounded(online.maximumLevel, 19, 0, 23));
  return {
    ...input,
    staleAfterMs: finite(input.staleAfterMs) ? Math.min(3600000, Math.max(100, input.staleAfterMs)) : 3000,
    trailMaxPoints: finite(input.trailMaxPoints) ? Math.min(20000, Math.max(2, Math.floor(input.trailMaxPoints))) : 2000,
    initialView: {
      longitude: finite(view.longitude) && Math.abs(view.longitude) <= 180 ? view.longitude : 139.7671,
      latitude: finite(view.latitude) && Math.abs(view.latitude) <= 90 ? view.latitude : 35.6812,
      height: finite(view.height) ? Math.max(20, view.height) : 2500,
      heading: finite(view.heading) ? view.heading : 0,
      pitch: finite(view.pitch) ? Math.max(-90, Math.min(-5, view.pitch)) : -45,
    },
    drone: { url: '/models/drone/multirotor.glb', scale: 1, ...input.drone },
    altitude: { geoidSeparationM: null, ...input.altitude },
    buildings: Array.isArray(input.buildings) ? input.buildings.slice(0, 64) : [],
    mapSource: typeof input.mapSource === 'string' && input.mapSource.trim() ? input.mapSource.trim() : 'downloaded-satellite',
    onlineImagery: {
      ...online,
      enabled: online.enabled === true && ['arcgis', 'xyz'].includes(onlineProvider),
      provider: onlineProvider,
      url: typeof online.url === 'string' ? online.url.trim() : onlineProvider === 'arcgis' ? WORLD_IMAGERY_URL : '',
      minimumLevel: Math.floor(bounded(online.minimumLevel, 0, 0, onlineMaximumLevel)),
      maximumLevel: onlineMaximumLevel,
      credit: typeof online.credit === 'string' ? online.credit : '',
    },
    buildingRendering: {
      maximumScreenSpaceError: bounded(buildings.maximumScreenSpaceError, 2, 1, 64),
      dynamicScreenSpaceError: typeof buildings.dynamicScreenSpaceError === 'boolean' ? buildings.dynamicScreenSpaceError : false,
      foveatedScreenSpaceError: typeof buildings.foveatedScreenSpaceError === 'boolean' ? buildings.foveatedScreenSpaceError : false,
      cullRequestsWhileMoving: typeof buildings.cullRequestsWhileMoving === 'boolean' ? buildings.cullRequestsWhileMoving : false,
      cacheMegabytes: bounded(buildings.cacheMegabytes, 1024, 64, 4096),
      maximumCacheOverflowMegabytes: bounded(buildings.maximumCacheOverflowMegabytes, 1024, 0, 4096),
    },
  };
}
