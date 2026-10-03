import { finite, fresh } from './protocol.js';

// Row-major rotation matrices. MAVLink attitude rotates body FRD into local NED.
// Raw glTF is +X forward, +Y up, +Z right; Cesium axis correction is disabled.
export const MODEL_TO_FRD = [1, 0, 0, 0, 0, 1, 0, -1, 0];
export const NED_TO_THREE = [0, 1, 0, 0, 0, -1, -1, 0, 0]; // East, up, south

export function multiply3(a, b) {
  return Array.from({ length: 9 }, (_, i) => {
    const row = Math.floor(i / 3), col = i % 3;
    return a[row * 3] * b[col] + a[row * 3 + 1] * b[col + 3] + a[row * 3 + 2] * b[col + 6];
  });
}

export function bodyToNed(attitude) {
  if (!attitude || attitude.valid !== true) return null;
  const q = attitude.quaternionWxyz;
  if (Array.isArray(q) && q.length === 4 && q.every(finite)) {
    const norm = Math.hypot(...q);
    if (norm > 1e-8 && norm < 10) {
      const [w, x, y, z] = q.map(v => v / norm);
      return [1 - 2 * (y*y + z*z), 2 * (x*y - z*w), 2 * (x*z + y*w),
        2 * (x*y + z*w), 1 - 2 * (x*x + z*z), 2 * (y*z - x*w),
        2 * (x*z - y*w), 2 * (y*z + x*w), 1 - 2 * (x*x + y*y)];
    }
  }
  const { rollRad: roll, pitchRad: pitch, yawRad: yaw } = attitude;
  if (![roll, pitch, yaw].every(finite)) return null;
  const cr = Math.cos(roll), sr = Math.sin(roll), cp = Math.cos(pitch), sp = Math.sin(pitch), cy = Math.cos(yaw), sy = Math.sin(yaw);
  return [cy*cp, cy*sp*sr-sy*cr, cy*sp*cr+sy*sr,
    sy*cp, sy*sp*sr+cy*cr, sy*sp*cr-cy*sr,
    -sp, cp*sr, cp*cr];
}

export function modelToNed(attitude) {
  const body = bodyToNed(attitude);
  return body ? multiply3(body, MODEL_TO_FRD) : null;
}

export function modelToThree(attitude) {
  const ned = modelToNed(attitude);
  return ned ? multiply3(NED_TO_THREE, ned) : null;
}

function nearby(sample, position) {
  return finite(sample?.latitudeDeg) && finite(sample?.longitudeDeg) &&
    Math.abs(sample.latitudeDeg - position.latitudeDeg) < .02 &&
    Math.abs(((sample.longitudeDeg - position.longitudeDeg + 540) % 360) - 180) < .02;
}

export function resolveAltitude(position, gps, config, elapsedMs = 0, geoidCache) {
  // GLOBAL_POSITION_INT.alt is MSL. Cesium Cartesian3.fromDegrees needs
  // ellipsoid height h = H + N. Never substitute relative/home altitude.
  const offset = config.altitude?.geoidSeparationM;
  if (finite(offset)) return { heightM: position.altitudeMslM + offset, approximate: false, source: `Configured geoid separation ${offset.toFixed(2)} m` };
  if (fresh(gps, elapsedMs, config.staleAfterMs) && gps.valid === true && finite(gps.altitudeEllipsoidM) && finite(gps.altitudeMslM) && nearby(gps, position)) {
    const separation = gps.altitudeEllipsoidM - gps.altitudeMslM;
    if (geoidCache) Object.assign(geoidCache, { latitudeDeg: gps.latitudeDeg, longitudeDeg: gps.longitudeDeg, separation });
    return { heightM: position.altitudeMslM + separation, approximate: false, source: 'Geoid separation from GPS ellipsoid and MSL heights' };
  }
  // Geoid separation varies with location, not aircraft motion/time. Keep the
  // measured local datum during GPS dropouts instead of moving the aircraft by
  // tens of meters. Invalidate it after moving ~2 km or reconnecting the stream.
  if (finite(geoidCache?.separation) && nearby(geoidCache, position)) {
    return { heightM: position.altitudeMslM + geoidCache.separation, approximate: false, source: 'Last GPS-derived local geoid separation (cached near its source position)' };
  }
  return { heightM: position.altitudeMslM, approximate: true, source: 'Approximate height: MSL used without geoid separation; set scene.altitude.geoidSeparationM for accurate building alignment' };
}
