#!/usr/bin/env python3
"""Resume a full Tokyo 23-ward PLATEAU building, imagery, map and terrain cache.

Optional setup dependencies: requests, shapely. Uses original provider pixels
and existing cached files; publishes metadata only after each dataset completes.
"""
import argparse
from concurrent.futures import ThreadPoolExecutor
import json
import math
from pathlib import Path
import re
import shutil
import struct
import threading
import time
from urllib.parse import urljoin, urlparse
import zipfile
import zlib

import requests
from shapely.geometry import box, shape
from shapely.ops import unary_union
from shapely.prepared import prep

from download_assets import (digest, install_terrain, tile_coordinate,
                             verify_embedded_model, verify_png, IMAGERY_URL)

CATALOG = 'https://api.plateauview.mlit.go.jp/datacatalog/3dtiles/13-bldg-lod2-latest/tileset.json'
BOUNDARIES = 'https://nlftp.mlit.go.jp/ksj/gml/data/N03/N03-2025/N03-20250101_13_GML.zip'
BBOX = [139.5627, 35.5281, 139.9190, 35.8178]
CODES = {str(code) for code in range(13101, 13124)}
GSI_URL = 'https://cyberjapandata.gsi.go.jp/xyz/std/{z}/{x}/{y}.png'


def atomic_json(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_suffix(path.suffix + '.tmp')
    temporary.write_text(json.dumps(value, ensure_ascii=False, separators=(',', ':')) + '\n')
    temporary.replace(path)


class Downloader:
    def __init__(self, root, max_gib, reserve_gib):
        self.root, self.limit, self.reserve = root, max_gib * 2**30, reserve_gib * 2**30
        self.local, self.lock = threading.local(), threading.Lock()
        self.total = 0
        self.started = time.monotonic()

    def get(self, url, destination, expected_hash=None, allow_missing=False,
            expected_bytes=None, expected_crc32=None):
        destination = Path(destination)
        if urlparse(url).scheme != 'https':
            raise ValueError('Only HTTPS sources are supported')
        def verify(path):
            if expected_bytes is not None and path.stat().st_size != expected_bytes:
                raise ValueError(f'Source size mismatch: {path}')
            if expected_hash and digest(path) != expected_hash:
                raise ValueError(f'Checksum mismatch: {path}')
            if expected_crc32 is not None:
                crc = 0
                with path.open('rb') as stream:
                    for chunk in iter(lambda: stream.read(1024 * 1024), b''):
                        crc = zlib.crc32(chunk, crc)
                if f'{crc & 0xffffffff:08x}' != expected_crc32.lower():
                    raise ValueError(f'Source ZIP checksum mismatch: {path}')
        if destination.is_file():
            verify(destination)
            return destination
        destination.parent.mkdir(parents=True, exist_ok=True)
        if not hasattr(self.local, 'session'):
            self.local.session = requests.Session()
            self.local.session.headers['User-Agent'] = 'OpenKAI-MAVLink-offline-cache/1.0'
        temporary = destination.with_suffix(destination.suffix + '.part')
        for attempt in range(5):
            if shutil.disk_usage(self.root).free < self.reserve:
                raise RuntimeError('Disk reserve reached; free space and resume the same command')
            with self.lock:
                if self.total >= self.limit:
                    raise RuntimeError('Download budget reached; increase --max-gib and resume')
            try:
                with self.local.session.get(url, stream=True, timeout=(15, 90)) as response:
                    if urlparse(response.url).scheme != 'https':
                        raise ValueError('Source redirected outside HTTPS')
                    if response.status_code == 404 and allow_missing:
                        return None
                    response.raise_for_status()
                    with temporary.open('wb') as stream:
                        for chunk in response.iter_content(256 * 1024):
                            with self.lock:
                                if shutil.disk_usage(self.root).free < self.reserve + len(chunk):
                                    raise RuntimeError('Disk reserve reached; free space and resume the same command')
                                self.total += len(chunk)
                                if self.total > self.limit:
                                    raise RuntimeError('Download budget reached; increase --max-gib and resume')
                                stream.write(chunk)
                verify(temporary)
                temporary.replace(destination)
                return destination
            except requests.RequestException:
                if attempt == 4:
                    raise
                time.sleep(min(2**attempt, 16))
            finally:
                temporary.unlink(missing_ok=True)

    def progress(self, label, count, total):
        elapsed = time.monotonic() - self.started
        print(f'{label}: {count}/{total}; received {self.total / 2**30:.2f} GiB; {elapsed / 60:.1f} min', flush=True)


def run_jobs(download, jobs, work, workers, label, interval=250):
    # Bounded batches avoid submitting hundreds of thousands of futures at once.
    done, results = 0, []
    with ThreadPoolExecutor(max_workers=workers) as pool:
        for start in range(0, len(jobs), workers * 64):
            for result in pool.map(work, jobs[start:start + workers * 64]):
                results.append(result)
                done += 1
                if done % interval == 0:
                    download.progress(label, done, len(jobs))
    download.progress(label, done, len(jobs))
    return results


def boundaries(download, root):
    folder = root / 'plateau' / 'tokyo-23wards'
    target = folder / 'ward-boundaries.geojson'
    if not target.exists():
        archive = download.get(BOUNDARIES, folder / 'source-boundaries.zip')
        with zipfile.ZipFile(archive) as source:
            names = [name for name in source.namelist() if name.lower().endswith('.geojson')]
            if len(names) != 1:
                raise ValueError('Expected one official N03 GeoJSON')
            document = json.loads(source.read(names[0]))
        features = [f for f in document['features'] if f['properties'].get('N03_007') in CODES]
        if {f['properties']['N03_007'] for f in features} != CODES:
            raise ValueError('Boundary source does not contain all 23 wards')
        atomic_json(target, {'type': 'FeatureCollection', 'source': BOUNDARIES, 'features': features})
    document = json.loads(target.read_text())
    return unary_union([shape(f['geometry']) for f in document['features']])


def prepare_buildings(download, root):
    folder = root / 'plateau' / 'tokyo-23wards'
    source = download.get(CATALOG, folder / 'source-catalog-tileset.json')
    catalog = json.loads(source.read_text())
    wards, jobs, publications, seen = [], {}, [], set()
    for entry in catalog['root']['children']:
        url = entry.get('content', {}).get('uri', '')
        match = re.search(r'/(131\d\d)_([a-z-]+)_(?:pref|city)_(\d{4})_', url)
        if not match or match[1] not in CODES:
            continue
        code, name, year = match.groups()
        if code in seen:
            raise ValueError(f'Duplicate ward in catalog: {code}')
        seen.add(code)
        target = folder / code
        original = download.get(url, target / 'source-tileset.json')
        document = json.loads(original.read_text())
        counts = [0]

        def rewrite(tile):
            if tile.get('implicitTiling'):
                raise ValueError('Implicit source tiles need a different mirror')
            for content in ([tile['content']] if 'content' in tile else []) + tile.get('contents', []):
                remote = urljoin(url, content.get('uri', content.get('url', '')))
                suffix = Path(urlparse(remote).path).suffix
                if suffix not in ('.b3dm', '.glb'):
                    raise ValueError(f'Unexpected building resource: {remote}')
                import hashlib
                filename = hashlib.sha256(remote.encode()).hexdigest()[:20] + suffix
                existing = root / 'plateau' / 'tokyo-central' / 'data' / filename
                if code == '13101' and existing.is_file():
                    destination = existing
                    relative = '../../tokyo-central/data/' + filename
                else:
                    destination = target / 'data' / filename
                    relative = 'data/' + filename
                jobs[remote] = destination
                counts[0] += 1
                content.pop('url', None)
                content['uri'] = relative
            for child in tile.get('children', []):
                rewrite(child)
        rewrite(document['root'])
        dataset = f'https://www.geospatial.jp/ckan/dataset/plateau-{code}-{name}-{year}'
        document.setdefault('asset', {}).setdefault('extras', {}).update({
            'source': url, 'dataset': dataset, 'modification': 'All source building content mirrored; local URIs only',
            'attribution': f'東京都 / 国土交通省 Project PLATEAU, {name} {year}'})
        publications.append((target / 'tileset.json', document))
        wards.append({'code': code, 'name': name, 'year': int(year), 'source': url, 'dataset': dataset,
                      'tiles': counts[0], 'region': document['root']['boundingVolume']['region'],
                      'assetVersion': document.get('asset', {}).get('version', '1.0'),
                      'geometricError': document.get('geometricError', 500),
                      'url': f'/models/plateau/tokyo-23wards/{code}/tileset.json'})
    if seen != CODES:
        raise ValueError(f'Catalog missing wards: {sorted(CODES - seen)}')
    return sorted(wards, key=lambda w: w['code']), list(jobs.items()), publications


def install_buildings(download, root, plan, workers, inventory=None):
    wards, jobs, publications = plan
    sizes, checksums = {}, {}
    if inventory is not None:
        for ward in inventory['wards']:
            sizes.update(ward['contentSizes'])
            checksums.update(ward['contentZipCRC32'])
        planned = {url for url, _ in jobs}
        if planned != set(sizes) or planned != set(checksums):
            raise ValueError('Source inventory does not exactly match planned building URLs')
    def fetch(job):
        existed = job[1].is_file()
        path = download.get(*job, expected_bytes=sizes.get(job[0]), expected_crc32=checksums.get(job[0]))
        try:
            verify_embedded_model(path)
        except (ValueError, struct.error):
            if not existed:
                path.unlink(missing_ok=True)
            raise
    run_jobs(download, jobs, fetch, workers, 'Buildings', 100)
    for path, document in publications:
        atomic_json(path, document)
    regions = [w['region'] for w in wards]
    region = [min(r[i] for r in regions) if i in (0, 1, 4) else max(r[i] for r in regions) for i in range(6)]
    version = max((w['assetVersion'] for w in wards), key=lambda v: tuple(map(int, v.split('.'))))
    geometric_error = max(50000, *(w['geometricError'] for w in wards))
    document = {'asset': {'version': version, 'extras': {'source': CATALOG, 'wards': 23}},
                'geometricError': geometric_error, 'root': {'boundingVolume': {'region': region},
                'geometricError': geometric_error, 'refine': 'ADD', 'children': [
                    {'boundingVolume': {'region': w['region']}, 'geometricError': w['geometricError'],
                     'content': {'uri': w['code'] + '/tileset.json'}} for w in wards]}}
    atomic_json(root / 'plateau/tokyo-23wards/tileset.json', document)
    return {'url': '/models/plateau/tokyo-23wards/tileset.json', 'tiles': len(jobs), 'wards': wards,
            'rectangle': BBOX, 'source': CATALOG, 'licenseUrl': 'https://www.mlit.go.jp/plateau/site-policy/'}


def imagery_coordinates(geometry, minimum, maximum):
    prepared = prep(geometry)
    coordinates = []
    for zoom in range(minimum, maximum + 1):
        west, north = tile_coordinate(BBOX[0], BBOX[3], zoom)
        east, south = tile_coordinate(BBOX[2], BBOX[1], zoom)
        count = 2**zoom
        def latitude(y):
            return math.degrees(math.atan(math.sinh(math.pi * (1 - 2 * y / count))))
        for x in range(west, east + 1):
            for y in range(north, south + 1):
                rectangle = box(x / count * 360 - 180, latitude(y + 1),
                                (x + 1) / count * 360 - 180, latitude(y))
                if prepared.intersects(rectangle):
                    coordinates.append((zoom, x, y))
    return coordinates


def coverage(coordinates, minimum, maximum):
    rows = {str(z): {} for z in range(minimum, maximum + 1)}
    for z, x, y in sorted(coordinates):
        ranges = rows[str(z)].setdefault(str(x), [])
        if ranges and ranges[-1][1] == y - 1:
            ranges[-1][1] = y
        else:
            ranges.append([y, y])
    return {'format': 'openkai-tile-coverage/1', 'minimumLevel': minimum, 'maximumLevel': maximum, 'levels': rows}


def install_imagery(download, root, coordinates, maximum, workers, street_map=False):
    name = 'gsi' if street_map else 'plateau-ortho-2024'
    url = GSI_URL if street_map else IMAGERY_URL
    jobs = [c for c in coordinates if c[0] <= maximum]
    target = root / 'imagery' / name
    missing = []
    def fetch(coordinate):
        z, x, y = coordinate
        destination = target / str(z) / str(x) / f'{y}.png'
        existed = destination.is_file()
        path = download.get(url.format(z=z, x=x, y=y), destination, allow_missing=True)
        if path is None:
            return None
        try:
            verify_png(path)
        except ValueError:
            if not existed:
                path.unlink(missing_ok=True)
            raise
        return coordinate
    results = run_jobs(download, jobs, fetch, workers, name, 1000)
    good = [c for c in results if c is not None]
    missing = [coordinate for coordinate, result in zip(jobs, results) if result is None]
    atomic_json(target / 'coverage.json', coverage(good, 12, maximum))
    atomic_json(target / 'source-missing-tiles.json', {'source': url, 'tiles': missing})
    return {'url': f'/models/imagery/{name}/{{z}}/{{x}}/{{y}}.png', 'rectangle': BBOX,
            'availableTilesUrl': f'/models/imagery/{name}/coverage.json',
            'minimumLevel': 12, 'maximumLevel': maximum, 'tiles': len(good), 'sourceMissingTiles': len(missing),
            'source': url, 'selection': 'Tiles intersecting all 23 official N03 2025 ward polygons',
            'modification': 'Geographic selection only; original provider PNG bytes retained',
            'attribution': '地理院タイル（国土地理院）' if street_map else '東京都 / 国土交通省 Project PLATEAU / 国土地理院（航空写真）',
            'usageTerms': 'https://maps.gsi.go.jp/help/use.html' if street_map else 'https://www.mlit.go.jp/plateau/site-policy/'}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--models-root', type=Path, default=Path('/home/kai/dev/models/webMavlink'))
    parser.add_argument('--workers', type=int, default=8)
    parser.add_argument('--max-gib', type=float, default=80)
    parser.add_argument('--reserve-gib', type=float, default=5)
    parser.add_argument('--only', choices=['all', 'buildings', 'imagery', 'map', 'terrain'], default='all')
    parser.add_argument('--plan', action='store_true', help='Fetch only metadata and print tile counts')
    parser.add_argument('--inventory', type=Path,
                        help='Optional catalog with official ZIP sizes and CRC32 for each building content URL')
    args = parser.parse_args()
    if not 1 <= args.workers <= 32 or args.max_gib <= 0 or args.reserve_gib < 1:
        parser.error('Require 1..32 workers, positive max-gib and reserve-gib >= 1')
    root = args.models_root.resolve()
    root.mkdir(parents=True, exist_ok=True)
    download = Downloader(root, args.max_gib, args.reserve_gib)
    geometry = boundaries(download, root)
    plan = prepare_buildings(download, root)
    coordinates = imagery_coordinates(geometry, 12, 19)
    print(json.dumps({'wards': 23, 'buildingTiles': len(plan[1]), 'imageryTiles': len(coordinates),
                      'streetMapTiles': sum(c[0] <= 17 for c in coordinates), 'freeGiB': shutil.disk_usage(root).free / 2**30}, indent=2), flush=True)
    if args.plan:
        return
    manifest_path = root / 'asset-manifest.json'
    manifest = json.loads(manifest_path.read_text()) if manifest_path.exists() else {}
    # Dataset summaries are checkpointed independently, keeping completed work
    # usable and resumable if a later network transfer is interrupted.
    def publish(key, value):
        manifest[key] = value
        atomic_json(manifest_path, manifest)
    if args.only in ('all', 'buildings'):
        inventory = json.loads(args.inventory.read_text()) if args.inventory else None
        publish('buildings', install_buildings(download, root, plan, args.workers, inventory))
    if args.only in ('all', 'imagery'):
        publish('imagery', install_imagery(download, root, coordinates, 19, args.workers))
    if args.only in ('all', 'map'):
        publish('downloadedMap', install_imagery(download, root, coordinates, 17, args.workers, street_map=True))
    if args.only in ('all', 'terrain'):
        publish('terrain', install_terrain(download, root, BBOX, args.workers))
    previous = {item['path']: item for item in manifest.get('files', [])}
    files = []
    for path in sorted(root.rglob('*')):
        if not path.is_file() or path == manifest_path or path.suffix in ('.part', '.tmp'):
            continue
        relative = str(path.relative_to(root))
        item = previous.get(relative)
        if item is None or item['bytes'] != path.stat().st_size or path.suffix == '.json':
            item = {'path': relative, 'bytes': path.stat().st_size, 'sha256': digest(path)}
        files.append(item)
    manifest['files'] = files
    atomic_json(manifest_path, manifest)
    print(f'Complete: {manifest_path}; {sum(f["bytes"] for f in files) / 2**30:.2f} GiB cached', flush=True)


if __name__ == '__main__':
    try:
        main()
    except KeyboardInterrupt:
        print('\nInterrupted. Completed cache files are retained; run the same command to resume.', flush=True)
        raise SystemExit(130)
