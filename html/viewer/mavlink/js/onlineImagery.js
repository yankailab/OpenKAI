import { assetUrl, finite } from './protocol.js';
import { tileCoverage, transparentTile } from './tileCoverage.js';

const RETRY_DELAYS_MS = [5000, 15000, 30000, 60000];
const INITIALIZATION_TIMEOUT_MS = 10000;

// Owns only the selected raster layer. Outages never select another provider.
export class SelectedImagery {
  constructor(map, onState) {
    this.map = map;
    this.onState = onState;
    this.layer = null;
    this.generation = 0;
    this.state = 'disabled';
    this.retryCount = 0;
    this.onOnline = () => {
      if (this.settings?.online && this.state !== 'ready' && this.state !== 'loading') {
        this.retryCount = 0;
        this.start();
      }
    };
    this.onOffline = () => {
      if (!this.settings?.online) return;
      this.cancel();
      this.removeLayer();
      this.status('offline', 'offline · no other map source is displayed');
    };
    globalThis.addEventListener('online', this.onOnline);
    globalThis.addEventListener('offline', this.onOffline);
  }

  status(state, message) {
    this.state = state;
    this.onState(state, message);
  }

  configure(settings, endpoint) {
    this.stop();
    this.settings = settings;
    this.endpoint = endpoint;
    this.retryCount = 0;
    return this.start();
  }

  cancel() {
    ++this.generation;
    clearTimeout(this.retryTimer);
    clearTimeout(this.initializationTimer);
    this.retryTimer = null;
    this.initializationTimer = null;
  }

  removeLayer() {
    // Keep the provider's error listener: in-flight callbacks are harmless once
    // their generation is stale, and still need a listener to consume failures.
    if (this.layer && !this.map.viewer.isDestroyed()) {
      this.map.viewer.imageryLayers.remove(this.layer, true);
      this.map.viewer.scene.requestRender();
    }
    this.layer = null;
  }

  fail(generation) {
    if (generation !== this.generation) return;
    this.cancel();
    this.removeLayer();
    this.status('unavailable', this.settings.online
      ? 'unavailable · retrying this source'
      : 'downloaded tiles unavailable · check the configured files');
    if (!this.settings.online) return;
    const retryGeneration = this.generation;
    const delay = RETRY_DELAYS_MS[this.retryCount];
    this.retryCount = Math.min(this.retryCount + 1, RETRY_DELAYS_MS.length - 1);
    this.retryTimer = setTimeout(() => {
      this.retryTimer = null;
      if (retryGeneration === this.generation) this.start();
    }, delay);
  }

  async start() {
    this.cancel();
    this.removeLayer();
    const settings = this.settings;
    if (!settings) return;
    const generation = this.generation;
    const current = () => generation === this.generation && !this.map.viewer.isDestroyed();
    if (settings.online && globalThis.navigator?.onLine === false) {
      this.status('offline', 'offline · no other map source is displayed');
      return;
    }
    this.status('loading', settings.online ? 'connecting…' : 'loading…');
    const C = this.map.C;
    try {
      const url = assetUrl(settings.url, this.endpoint);
      if (!url) throw new Error('Imagery URL is missing');
      const rectangle = Array.isArray(settings.rectangle) && settings.rectangle.length === 4 && settings.rectangle.every(finite)
        ? C.Rectangle.fromDegrees(...settings.rectangle) : undefined;
      let credit;
      if (typeof settings.credit === 'string' && settings.credit) {
        const node = document.createElement(settings.creditUrl ? 'a' : 'span');
        node.textContent = settings.credit;
        if (settings.creditUrl) {
          node.href = assetUrl(settings.creditUrl, this.endpoint);
          node.target = '_blank';
          node.rel = 'noopener noreferrer';
        }
        credit = new C.Credit(node.outerHTML, true);
      }
      const options = {
        minimumLevel: settings.minimumLevel, maximumLevel: settings.maximumLevel,
        rectangle, enablePickFeatures: false, credit,
      };
      let creation;
      if (settings.provider === 'arcgis') creation = C.ArcGisMapServerImageryProvider.fromUrl(url, options);
      else if (settings.provider === 'tms') creation = C.TileMapServiceImageryProvider.fromUrl(url, options);
      else if (settings.provider === 'xyz') creation = Promise.resolve(new C.UrlTemplateImageryProvider({ ...options, url }));
      else throw new Error('Unsupported imagery provider');
      let coverage = Promise.resolve(null);
      if (settings.availableTilesUrl) {
        if (settings.online || settings.provider !== 'xyz') throw new Error('Tile coverage requires downloaded XYZ imagery');
        coverage = fetch(assetUrl(settings.availableTilesUrl, this.endpoint), { cache: 'no-cache' }).then(async response => {
          if (!response.ok) throw new Error('Downloaded tile coverage is unavailable');
          return tileCoverage(await response.json());
        });
      }
      const timeout = new Promise((_, reject) => {
        this.initializationTimer = setTimeout(() => reject(new Error('Imagery connection timed out')), INITIALIZATION_TIMEOUT_MS);
      });
      const [source, containsTile] = await Promise.race([Promise.all([creation, coverage]), timeout]);
      if (!current()) return;
      clearTimeout(this.initializationTimer);
      this.initializationTimer = null;
      // Cesium replaces ArcGIS's maximumLevel option with the server's last LOD.
      // Keep its attribution and discard policy while enforcing our zoom limit.
      const provider = Object.create(source);
      if (source.credit) source.credit.showOnScreen = true;
      provider.getTileCredits = (...args) => {
        const credits = source.getTileCredits?.(...args);
        for (const item of credits || []) item.showOnScreen = true;
        return credits;
      };
      if (Number.isInteger(settings.maximumLevel)) Object.defineProperty(provider, 'maximumLevel', {
        value: Number.isInteger(source.maximumLevel) ? Math.min(settings.maximumLevel, source.maximumLevel) : settings.maximumLevel,
      });
      provider.requestImage = (...args) => {
        if (!current()) return undefined;
        // Never request intentionally uncached coordinates: one sparse local
        // layer remains selected, and holes expose the neutral globe below it.
        if (containsTile && !containsTile(...args)) return transparentTile();
        const image = source.requestImage(...args);
        if (!image) return image;
        // Start this timeout only once an in-view tile has actually been
        // requested; a bounded source may legitimately be outside the camera.
        if (settings.online && this.state === 'loading' && !this.initializationTimer)
          this.initializationTimer = setTimeout(() => this.fail(generation), INITIALIZATION_TIMEOUT_MS);
        return Promise.resolve(image).then(result => {
          if (current() && this.state === 'loading') {
            clearTimeout(this.initializationTimer);
            this.initializationTimer = null;
            this.retryCount = 0;
            this.status('ready', settings.online ? 'online' : 'downloaded coverage only');
          }
          return result;
        });
      };
      provider.errorEvent.addEventListener(error => {
        error.retry = false;
        if (current()) this.fail(generation);
      });
      const layer = new C.ImageryLayer(provider, { rectangle });
      // Cesium's first layer normally stretches its boundary pixels globally,
      // even with rectangle set. Treat sources as non-base layers so uncovered
      // ground uses globe.baseColor, without adding a second source.
      layer.isBaseLayer = () => false;
      this.layer = layer;
      this.map.viewer.imageryLayers.add(layer);
      if (!settings.online) this.status('ready', 'downloaded coverage only');
      // No first-tile timeout: a bounded source may legitimately be out of view.
      this.map.viewer.scene.requestRender();
    } catch (_error) {
      if (current()) this.fail(generation);
    }
  }

  stop() {
    this.cancel();
    this.removeLayer();
    this.settings = null;
    this.state = 'disabled';
  }

  destroy() {
    this.stop();
    globalThis.removeEventListener('online', this.onOnline);
    globalThis.removeEventListener('offline', this.onOffline);
  }
}
