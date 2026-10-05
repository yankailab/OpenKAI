import { assetUrl, finite, fresh, normalizeConfig, validPosition } from './protocol.js';
import { bodyToNed } from './geo.js';
import { MavlinkConnection } from './connection.js';
import { MapViewer } from './mapViewer.js';
import { AttitudeViewer } from './attitudeViewer.js';

const $ = selector => document.querySelector(selector);
const sceneMessages = new Map();
let heightApproximate = false;
function sceneStatus(key, text, error = false) {
  sceneMessages.set(key, { text, error });
  $('#scene-status').replaceChildren(...Array.from(sceneMessages.values(), item => {
    const li = document.createElement('li'); li.textContent = item.text; li.dataset.state = item.error ? 'error' : 'ready'; return li;
  }));
  if (error) $('#scene-details').open = true;
}

export const map = new MapViewer($('#viewport'), sceneStatus, (text, approximate) => {
  heightApproximate = approximate;
  $('#height-status').textContent = text;
  $('#height-status').dataset.stale = String(approximate);
});
function syncMapSelectors(sources, selectedId) {
  const selected = sources.find(source => source.id === selectedId);
  const groups = [...new Map(sources.map(source => [source.sourceId, source.sourceLabel])).entries()];
  $('#map-source').replaceChildren(...groups.map(([id, label]) => new Option(label, id)));
  $('#map-source').value = selected?.sourceId || '';
  const types = sources.filter(source => source.sourceId === selected?.sourceId);
  $('#map-type').replaceChildren(...types.map(source => new Option(source.typeLabel, source.type)));
  $('#map-type').value = selected?.type || '';
  $('#map-source').disabled = !groups.length;
  $('#map-type').disabled = types.length < 2;
}
map.onMapSourceChange = syncMapSelectors;
syncMapSelectors(map.mapSources.sources, map.mapSources.selectedId);

function syncCameraButtons() {
  $('#follow').setAttribute('aria-pressed', String(map.follow));
  $('#fpv').setAttribute('aria-pressed', String(map.fpv));
  $('#navigation-hint').textContent = map.fpv ? 'FPV · aircraft position and forward view · Click FPV to exit' : 'Left drag to orbit · Right drag to pan · Wheel to zoom';
}
export let attitudePreview;
try { attitudePreview = new AttitudeViewer($('#attitude-viewport'), sceneStatus); }
catch (error) { sceneStatus('attitude', `Attitude preview unavailable: ${error.message}`, true); }

let config = normalizeConfig(), latest = null, receivedAt = 0, streamState = 'stopped';
const systemStates = ['Uninitialized', 'Booting', 'Calibrating', 'Standby', 'Active', 'Critical', 'Emergency', 'Power off', 'Flight termination'];
const gpsStates = ['No GPS', 'No fix', '2D', '3D', 'DGPS', 'RTK float', 'RTK fixed', 'Static', 'PPP'];

function setField(id, text, stale = false) {
  const element = $(`#${id}`); element.textContent = text; element.dataset.stale = String(stale);
}
const number = (value, digits = 1, unit = '') => finite(value) ? `${value.toFixed(digits)}${unit}` : '—';
const degrees = value => finite(value) ? number(value * 180 / Math.PI, 1, '°') : '—';

function render() {
  const elapsed = latest ? Math.max(0, performance.now() - receivedAt) + (streamState === 'connected' ? 0 : config.staleAfterMs + 1) : 0;
  const isFresh = sample => fresh(sample, elapsed, config.staleAfterMs);
  const data = latest || {};
  const position = validPosition(data.position) ? data.position : null;
  const attitude = bodyToNed(data.attitude) ? data.attitude : null;
  const battery = data.battery || data.system;
  const heartbeatFresh = isFresh(data.heartbeat);
  const positionFresh = isFresh(position);
  const attitudeFresh = isFresh(attitude);
  const streaming = streamState === 'connected';
  const live = streaming && heartbeatFresh && data.connected;

  setField('roll', degrees(attitude?.rollRad), !!attitude && !attitudeFresh);
  setField('pitch', degrees(attitude?.pitchRad), !!attitude && !attitudeFresh);
  setField('yaw', finite(attitude?.yawRad) ? number((attitude.yawRad * 180 / Math.PI + 360) % 360, 1, '°') : '—', !!attitude && !attitudeFresh);
  setField('latitude', number(position?.latitudeDeg, 7, '°'), !!position && !positionFresh);
  setField('longitude', number(position?.longitudeDeg, 7, '°'), !!position && !positionFresh);
  setField('altitude', number(position?.altitudeMslM, 1, ' m'), !!position && !positionFresh);
  setField('relative-altitude', number(position?.relativeAltitudeM, 1, ' m'), !!position && !positionFresh);
  setField('speed', number(position?.groundSpeedMps, 1, ' m/s'), !!position && !positionFresh);
  setField('vertical-speed', number(finite(position?.velocityNedMps?.[2]) ? -position.velocityNedMps[2] : null, 1, ' m/s'), !!position && !positionFresh);
  setField('battery', number(battery?.remainingPct, 0, '%'), !!battery && !isFresh(battery));
  setField('power', `${number(battery?.voltageV, 1, ' V')} / ${number(battery?.currentA, 1, ' A')}`, !!battery && !isFresh(battery));
  setField('gps', data.gps ? `${gpsStates[data.gps.fixType] || `Fix ${data.gps.fixType ?? '?'}`} / ${number(data.gps.satellitesVisible, 0)}` : '—', !!data.gps && !isFresh(data.gps));
  setField('mode', data.heartbeat ? `${systemStates[data.heartbeat.systemStatus] || data.heartbeat.systemStatus || '—'} / ${data.heartbeat.customMode ?? '—'}` : '—', !!data.heartbeat && !heartbeatFresh);

  $('#vehicle-state').textContent = live ? (data.heartbeat.armed ? 'ARMED' : 'DISARMED') : latest ? 'LINK / DATA STALE' : 'NO TELEMETRY';
  $('#vehicle-state').dataset.state = live ? 'live' : latest ? 'stale' : 'waiting';
  $('#attitude-state').textContent = attitude ? (attitudeFresh ? 'Live attitude · green arrow points north' : 'Last attitude · stale') : 'Waiting for attitude';
  attitudePreview?.update(attitude, attitudeFresh);
  map.update(latest, elapsed);
  $('#center').disabled = !map.lastPosition;
  syncCameraButtons();
  $('#sample-state').textContent = latest
    ? `Position ${position ? number((position.ageMs + elapsed) / 1000, 1, 's ago') : 'unavailable'} · Attitude ${attitude ? number((attitude.ageMs + elapsed) / 1000, 1, 's ago') : 'unavailable'}`
    : 'Waiting for MAVLink messages';
  $('#sample-state').dataset.stale = String(!!latest && (!positionFresh || !attitudeFresh));
  const statusText = data.statusText;
  $('#vehicle-message').hidden = !statusText?.text;
  $('#vehicle-message').textContent = statusText?.text ? `Vehicle: ${String(statusText.text).slice(0, 4096)}` : '';
  $('#vehicle-message').dataset.stale = String(!!statusText && !isFresh(statusText));
  $('#welcome').hidden = !!position;
  if (streaming && !position) $('#welcome').innerHTML = 'Waiting for aircraft position<br><small>No valid global position has been received</small>';
  else if (!position) $('#welcome').innerHTML = 'Connect to your OpenKAI backend<br><small>Live position and attitude · Read-only telemetry</small>';
  $('#stats').textContent = position
    ? `${positionFresh ? 'Live position' : 'Last known position · stale'}${heightApproximate ? ' · Height approximate' : ''} · ${map.trailPositions.length} trail points · Read-only`
    : 'No aircraft position received';
  if (streaming) {
    $('#status').textContent = live ? 'MAVLink connected · read-only' : 'Stream connected · MAVLink heartbeat missing or stale';
    $('#status').dataset.state = live ? 'connected' : 'retrying';
  }
}

export const connection = new MavlinkConnection({
  onState(state, text) {
    streamState = state;
    $('#status').textContent = text; $('#status').dataset.state = state;
    $('#start').disabled = !['stopped', 'error'].includes(state);
    $('#stop').disabled = state === 'stopped';
    render();
  },
  onReset() { latest = null; map.reset(); render(); },
  onHello(raw, endpoint) {
    config = normalizeConfig(raw);
    for (const key of Array.from(sceneMessages.keys())) if (/^(buildings|imagery|terrain|model|attitude)/.test(key)) sceneMessages.delete(key);
    map.configure(config, endpoint).catch(error => sceneStatus('scene', `Scene configuration failed: ${error.message}`, true));
    try { attitudePreview?.load(assetUrl(config.drone.url, endpoint)); }
    catch (error) { sceneStatus('attitude', `Attitude model unavailable: ${error.message}`, true); }
  },
  onTelemetry(message) { latest = message; receivedAt = performance.now(); render(); },
});

$('#connection').addEventListener('submit', event => {
  event.preventDefault();
  try { connection.start(window.viewerEndpoint()); }
  catch (error) { $('#status').textContent = error.message; $('#status').dataset.state = 'error'; }
});
$('#stop').addEventListener('click', () => connection.stop());
$('#center').addEventListener('click', () => { map.setFpv(false); map.locate(); render(); });
$('#follow').addEventListener('click', () => { map.setFollow(!map.follow); render(); });
$('#fpv').addEventListener('click', () => { map.setFpv(!map.fpv); render(); });
$('#map-source').addEventListener('change', event => {
  const candidates = map.mapSources.sources.filter(source => source.sourceId === event.target.value);
  const selected = candidates.find(source => source.type === $('#map-type').value) || candidates[0];
  if (selected) map.mapSources.select(selected.id);
});
$('#map-type').addEventListener('change', event => {
  const selected = map.mapSources.sources.find(source => source.sourceId === $('#map-source').value && source.type === event.target.value);
  if (selected) map.mapSources.select(selected.id);
});
for (const [id, update] of [ ['trail', value => map.setTrail(value)] ]) {
  $(`#${id}`).addEventListener('click', event => {
    const pressed = event.currentTarget.getAttribute('aria-pressed') !== 'true';
    event.currentTarget.setAttribute('aria-pressed', String(pressed)); update(pressed);
  });
}
$('#clear-trail').addEventListener('click', () => map.clearTrail());
$('#tokyo').addEventListener('click', () => { map.setFpv(false); map.setFollow(false); map.goHome(); render(); });
document.addEventListener('visibilitychange', () => { if (!document.hidden) { map.viewer.resize(); render(); } });
window.addEventListener('beforeunload', () => connection.stop());
setInterval(render, 250); // Expire every message independently even if transport remains open.
$('#start').disabled = false;
$('#status').textContent = 'Ready · dedicated MAVLink stream';
render();
if (location.hash === '#connect') $('#connection').requestSubmit();
