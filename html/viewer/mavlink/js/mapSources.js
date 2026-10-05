import { finite, WORLD_IMAGERY_URL } from './protocol.js';
import { SelectedImagery } from './onlineImagery.js';

const WORLD_STREET_MAP_URL = 'https://services.arcgisonline.com/ArcGIS/rest/services/World_Street_Map/MapServer';
const TYPE_LABELS = { satellite: 'Satellite', map: 'Map', elevation: 'Elevation' };

function settings(defaults, overrides) {
  const input = overrides && typeof overrides === 'object' && !Array.isArray(overrides) ? overrides : {};
  const result = { ...defaults, ...input };
  result.provider = ['xyz', 'arcgis', 'tms'].includes(result.provider) ? result.provider : defaults.provider;
  result.url = typeof result.url === 'string' ? result.url.trim() : defaults.url;
  result.maximumLevel = Math.floor(finite(result.maximumLevel) ? Math.max(0, Math.min(23, result.maximumLevel)) : 19);
  result.minimumLevel = Math.floor(finite(result.minimumLevel) ? Math.max(0, Math.min(result.maximumLevel, result.minimumLevel)) : 0);
  if (!Array.isArray(result.rectangle) || result.rectangle.length !== 4 || !result.rectangle.every(finite) ||
      result.rectangle[0] < -180 || result.rectangle[2] > 180 || result.rectangle[1] < -90 || result.rectangle[3] > 90 ||
      result.rectangle[0] >= result.rectangle[2] || result.rectangle[1] >= result.rectangle[3]) delete result.rectangle;
  return result;
}

export function mapSourceCatalogue(config = {}) {
  const sources = [];
  const add = (id, sourceId, sourceLabel, type, options) => {
    if (options.enabled === false) return;
    sources.push({ ...options, id, sourceId, sourceLabel, type, typeLabel: TYPE_LABELS[type] });
  };
  if (config.imagery?.url) add('downloaded-satellite', 'downloaded', 'PLATEAU / downloaded', 'satellite', {
    ...settings({ provider: 'xyz', maximumLevel: 19 }, config.imagery), online: false,
  });
  if (config.downloadedMap?.url) add('downloaded-map', 'downloaded', 'PLATEAU / downloaded', 'map', {
    ...settings({ provider: 'xyz', maximumLevel: 17 }, config.downloadedMap), online: false,
  });
  if (config.terrain?.url) add('downloaded-elevation', 'downloaded', 'PLATEAU / downloaded', 'elevation', { provider: 'elevation', online: false });
  if (config.onlineImagery?.enabled) {
    const custom = config.mapSources || {};
    add('esri-satellite', 'esri', 'Esri', 'satellite', {
      ...settings({ provider: 'arcgis', url: WORLD_IMAGERY_URL, maximumLevel: 19 }, config.onlineImagery), online: true,
    });
    add('esri-map', 'esri', 'Esri', 'map', {
      ...settings({ provider: 'arcgis', url: WORLD_STREET_MAP_URL, maximumLevel: 19 }, custom.esriMap), online: true,
    });
    add('osm-map', 'osm', 'OpenStreetMap', 'map', {
      ...settings({ provider: 'xyz', url: 'https://tile.openstreetmap.org/{z}/{x}/{y}.png', maximumLevel: 19,
        credit: '© OpenStreetMap contributors', creditUrl: 'https://www.openstreetmap.org/copyright',
      }, custom.openStreetMap), online: true,
    });
    add('gsi-elevation', 'gsi', 'GSI Japan', 'elevation', {
      ...settings({ provider: 'xyz', url: 'https://cyberjapandata.gsi.go.jp/xyz/relief/{z}/{x}/{y}.png',
        minimumLevel: 5, maximumLevel: 15, rectangle: [122, 20, 154, 46],
        credit: '国土地理院 · 海域部は海上保安庁海洋情報部の資料を使用して作成',
        creditUrl: 'https://maps.gsi.go.jp/development/ichiran.html',
      }, custom.gsiElevation), online: true,
    });
  }
  add('natural-earth-map', 'natural-earth', 'Natural Earth / local', 'map', {
    provider: 'tms', url: new URL('../vendor/cesium/Assets/Textures/NaturalEarthII/', import.meta.url).href,
    credit: 'Natural Earth', creditUrl: 'https://www.naturalearthdata.com/', online: false,
  });
  return sources;
}

export class MapSources {
  constructor(map) {
    this.map = map;
    this.sources = [];
    this.selectedId = null;
    this.userSelection = false;
    this.state = 'disabled';
    this.imagery = new SelectedImagery(map, (state, message) => this.status(state, message));
  }

  get layer() { return this.imagery.layer; }
  get selected() { return this.sources.find(source => source.id === this.selectedId); }

  status(state, message) {
    this.state = state;
    const selected = this.selected;
    this.map.onStatus('imagery', `${selected?.sourceLabel || 'Map'} · ${selected?.typeLabel || ''} · ${message}`, state === 'unavailable');
  }

  configure(config, endpoint) {
    this.endpoint = endpoint;
    this.sources = mapSourceCatalogue(config);
    const previous = this.userSelection && this.sources.find(source => source.id === this.selectedId);
    const configured = this.sources.find(source => source.id === config.mapSource);
    return this.select((previous || configured || this.sources.find(source => source.id === 'downloaded-satellite') ||
      this.sources.find(source => source.id === 'natural-earth-map')).id, false);
  }

  clearMaterial() {
    if (!this.map.viewer.isDestroyed()) this.map.viewer.scene.globe.material = undefined;
    if (this.material && !this.material.isDestroyed()) this.material.destroy();
    this.material = null;
  }

  select(id, userSelection = true) {
    const source = this.sources.find(item => item.id === id);
    if (!source) return Promise.resolve(false);
    this.userSelection = userSelection || this.userSelection;
    this.imagery.stop();
    const scene = this.map.viewer.scene;
    this.clearMaterial();
    this.selectedId = id;
    this.map.onMapSourceChange?.(this.sources, id);
    if (source.provider === 'elevation') {
      // Height coloring uses the terrain already loaded for every map choice;
      // there is no raster imagery layer and no network request for this style.
      const ramp = document.createElement('canvas');
      ramp.width = 256; ramp.height = 1;
      const context = ramp.getContext('2d');
      const gradient = context.createLinearGradient(0, 0, 256, 0);
      ['#154d7a', '#3b9b87', '#96b66b', '#d9c98a', '#ab7451', '#eeeeec'].forEach((color, index) => gradient.addColorStop(index / 5, color));
      context.fillStyle = gradient; context.fillRect(0, 0, 256, 1);
      this.material = this.map.C.Material.fromType('ElevationRamp', { image: ramp, minimumHeight: -20, maximumHeight: 300 });
      scene.globe.material = this.material;
      this.status('ready', 'terrain height colors · −20 to 300 m (ellipsoid)');
      scene.requestRender();
      return Promise.resolve(true);
    }
    return this.imagery.configure(source, this.endpoint).then(() => true);
  }

  destroy() {
    this.imagery.destroy();
    this.clearMaterial();
  }
}
