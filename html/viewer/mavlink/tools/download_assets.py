#!/usr/bin/env python3
"""Prepare pinned local viewer libraries and a bounded, token-free Tokyo cache.

Requires Python 3.9+ and curl. Downloads only on explicit invocation. The
default photographic extent covers Chiyoda with a margin. Buildings use an
independent extent selecting all Chiyoda tiles, not individual clipped buildings.
"""

import argparse
from concurrent.futures import ThreadPoolExecutor
import copy
import gzip
import hashlib
import json
import math
from pathlib import Path
import shutil
import struct
import subprocess
import tarfile
import tempfile
import threading
import zlib
from urllib.parse import urljoin, urlparse

from create_drone import create_model

VIEWER = Path(__file__).resolve().parents[1]
BBOX = [139.725, 35.665, 139.790, 35.710]  # west, south, east, north, degrees
BUILDINGS_BBOX = [139.7301, 35.6690, 139.7828, 35.7052]
TERRAIN_BBOX = [139.75, 35.675, 139.775, 35.695]
CESIUM_URL = "https://registry.npmjs.org/cesium/-/cesium-1.138.0.tgz"
CESIUM_SHA256 = "dac1310ae9e23d420217fd9e7fdb72240cd479472dc9aa9dd6ef257842bb1b07"
THREE_URL = "https://registry.npmjs.org/three/-/three-0.185.0.tgz"
THREE_SHA256 = "5f52a93ffb7d5bd87735d414fad83709af45ddcb77f8c1198dda98971074433d"
PLATEAU_URL = (
    "https://assets.cms.plateau.reearth.io/assets/28/"
    "07d0a1-b6be-46ef-bd87-4f0683b5ef6e/"
    "13101_chiyoda-ku_pref_2025_citygml_1_op_bldg_3dtiles_13101_chiyoda-ku_lod2/tileset.json"
)
DATASET_URL = "https://www.geospatial.jp/ckan/dataset/plateau-13101-chiyoda-ku-2025"
CATALOG_URL = "https://www.geospatial.jp/ckan/api/3/action/package_show?id=plateau-13101-chiyoda-ku-2025"
IMAGERY_NAME = "plateau-ortho-2024"
IMAGERY_URL = "https://tile.plateauview.mlit.go.jp/tiles/" + IMAGERY_NAME + "/{z}/{x}/{y}.png"
IMAGERY_CATALOG = "https://tile.plateauview.mlit.go.jp/tiles/catalog.json"
IMAGERY_DOCUMENTATION = "https://docs.plateauview.mlit.go.jp/datasets/ortho/"
TERRAIN_URL = "https://tile.plateauview.mlit.go.jp/terrain/"


class Downloader:
    def __init__(self, max_bytes):
        self.max_bytes = max_bytes
        self.total = 0
        self.lock = threading.Lock()

    def get(self, url, destination, expected_hash=None):
        destination = Path(destination)
        destination.parent.mkdir(parents=True, exist_ok=True)
        if urlparse(url).scheme != "https":
            raise ValueError("Only HTTPS download sources are supported")
        with self.lock:
            if self.total >= self.max_bytes:
                raise ValueError("Offline download exceeds --max-mib; choose a smaller extent")
        if not destination.is_file():
            temporary = destination.with_suffix(destination.suffix + ".part")
            try:
                command = [
                    "curl", "--fail", "--silent", "--show-error", "--location",
                    "--proto", "=https", "--proto-redir", "=https",
                    "--connect-timeout", "15", "--max-time", "120",
                    "--max-filesize", str(min(self.max_bytes, 64 * 1024 * 1024)),
                    "--user-agent", "OpenKAI-MAVLink-offline-cache/1.0",
                    url]
                with subprocess.Popen(command, stdout=subprocess.PIPE) as process:
                    try:
                        with temporary.open("wb") as output:
                            while True:
                                chunk = process.stdout.read(64 * 1024)
                                if not chunk:
                                    break
                                with self.lock:
                                    if self.total + len(chunk) > self.max_bytes:
                                        raise ValueError("Offline download exceeds --max-mib; choose a smaller extent")
                                    self.total += len(chunk)
                                output.write(chunk)
                        if process.wait() != 0:
                            raise subprocess.CalledProcessError(process.returncode, command)
                    except BaseException:
                        process.kill()
                        process.wait()
                        raise
                temporary.replace(destination)
            finally:
                temporary.unlink(missing_ok=True)
        else:
            with self.lock:
                self.total += destination.stat().st_size
                if self.total > self.max_bytes:
                    raise ValueError("Offline download exceeds --max-mib; choose a smaller extent")
        if expected_hash and digest(destination) != expected_hash:
            raise ValueError(f"Checksum mismatch: {destination}; remove it and retry")
        return destination


def digest(path):
    sha = hashlib.sha256()
    with Path(path).open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            sha.update(chunk)
    return sha.hexdigest()


def install_vendor(download):
    vendor = VIEWER / "vendor"
    with tempfile.TemporaryDirectory(prefix="openkai-mavlink-vendor-") as temporary:
        archive = download.get(CESIUM_URL, Path(temporary) / "cesium.tgz", CESIUM_SHA256)
        with tarfile.open(archive) as source:
            for member in source.getmembers():
                if not member.isfile():
                    continue
                prefix = "package/Build/Cesium/"
                if member.name.startswith(prefix):
                    relative = Path(member.name[len(prefix):])
                elif member.name in ("package/LICENSE.md", "package/ThirdParty.json", "package/ThirdParty.extra.json"):
                    relative = Path(member.name).name
                else:
                    continue
                relative = Path(relative)
                if relative.is_absolute() or ".." in relative.parts:
                    raise ValueError("Unsafe archive path")
                target = vendor / "cesium" / relative
                target.parent.mkdir(parents=True, exist_ok=True)
                target.write_bytes(source.extractfile(member).read())
        archive = download.get(THREE_URL, Path(temporary) / "three.tgz", THREE_SHA256)
        files = {"GLTFLoader.js": "examples/jsm/loaders/GLTFLoader.js",
                 "BufferGeometryUtils.js": "examples/jsm/utils/BufferGeometryUtils.js",
                 "SkeletonUtils.js": "examples/jsm/utils/SkeletonUtils.js",
                 "three.module.min.js": "build/three.module.min.js",
                 "three.core.min.js": "build/three.core.min.js", "LICENSE.three": "LICENSE"}
        with tarfile.open(archive) as source:
            for target, relative in files.items():
                data = source.extractfile("package/" + relative).read()
                if target == "GLTFLoader.js":
                    data = data.replace(b"'../utils/BufferGeometryUtils.js'", b"'./BufferGeometryUtils.js'")
                    data = data.replace(b"'../utils/SkeletonUtils.js'", b"'./SkeletonUtils.js'")
                (vendor / target).write_bytes(data)
    print("Installed CesiumJS 1.138.0 and three.js r185", flush=True)


def verify_embedded_model(path):
    """Prevent a supposedly offline b3dm/GLB from retaining external textures."""
    data = Path(path).read_bytes()
    if data[:4] == b"b3dm":
        header = struct.unpack_from("<4s6I", data)
        if header[1] != 1 or header[2] != len(data):
            raise ValueError(f"Invalid b3dm: {path}")
        data = data[28 + sum(header[3:]):]
    if data[:4] != b"glTF":
        raise ValueError(f"Expected self-contained b3dm/GLB: {path}")
    magic, version, length = struct.unpack_from("<III", data)
    if version != 2 or length != len(data):
        raise ValueError(f"Invalid GLB: {path}")
    json_length, chunk_type = struct.unpack_from("<II", data, 12)
    if chunk_type != 0x4e4f534a:
        raise ValueError(f"Missing GLB JSON chunk: {path}")
    model = json.loads(data[20:20+json_length])
    for resource in model.get("buffers", []) + model.get("images", []):
        if resource.get("uri") and not resource["uri"].startswith("data:"):
            raise ValueError(f"External model resource requires manual mirroring: {path}")


def install_buildings(download, root, bbox, workers):
    target = root / "plateau" / "tokyo-central"
    target.mkdir(parents=True, exist_ok=True)
    source = download.get(PLATEAU_URL, target / "source-tileset.json")
    download.get(CATALOG_URL, target / "source-catalog.json")
    document = json.loads(source.read_text())
    rectangle = [math.radians(value) for value in bbox]
    jobs = {}

    def prune(tile):
        region = tile.get("boundingVolume", {}).get("region")
        if region is None or tile.get("implicitTiling"):
            raise ValueError("This cache builder requires explicit region-bounded PLATEAU tiles")
        west, south, east, north = rectangle
        if region[2] < west or region[0] > east or region[3] < south or region[1] > north:
            return None
        tile = copy.deepcopy(tile)
        children = [child for child in (prune(c) for c in tile.get("children", [])) if child]
        if children:
            tile["children"] = children
        else:
            tile.pop("children", None)
        for content in ([tile["content"]] if "content" in tile else []) + tile.get("contents", []):
            url = urljoin(PLATEAU_URL, content.get("uri", content.get("url", "")))
            extension = Path(urlparse(url).path).suffix
            if extension not in (".b3dm", ".glb"):
                raise ValueError("This pinned cache requires embedded b3dm/GLB content")
            local = "data/" + hashlib.sha256(url.encode()).hexdigest()[:20] + extension
            content.pop("url", None)
            content["uri"] = local
            jobs[url] = target / local
        return tile

    document["root"] = prune(document["root"])
    if document["root"] is None:
        raise ValueError("Requested extent does not intersect Chiyoda buildings")
    count = 0

    def fetch(job):
        path = download.get(*job)
        verify_embedded_model(path)

    with ThreadPoolExecutor(max_workers=workers) as executor:
        for _ in executor.map(fetch, jobs.items()):
            count += 1
            if count % 25 == 0:
                print(f"PLATEAU: {count}/{len(jobs)} tiles", flush=True)
    document.setdefault("asset", {})["extras"] = {
        "source": PLATEAU_URL, "dataset": DATASET_URL,
        "modification": "Selected tiles intersecting the configured geographic extent",
        "selectionRectangleDegrees": bbox,
        "attribution": "東京都 / 国土交通省 Project PLATEAU, Chiyoda 2025"}
    # Publish the entry point only after every referenced tile passed validation.
    (target / "tileset.json").write_text(json.dumps(document, ensure_ascii=False, separators=(",", ":")) + "\n")
    return {"url": "/models/plateau/tokyo-central/tileset.json", "tiles": len(jobs),
            "rectangle": bbox, "source": PLATEAU_URL, "dataset": DATASET_URL,
            "license": "PLATEAU Site Policy, section 3",
            "licenseUrl": "https://www.mlit.go.jp/plateau/site-policy/"}


def tile_coordinate(longitude, latitude, zoom):
    count = 2 ** zoom
    x = (longitude + 180) / 360 * count
    y = (1 - math.asinh(math.tan(math.radians(latitude))) / math.pi) / 2 * count
    return int(math.floor(x)), int(math.floor(y))


def verify_png(path):
    """Reject error bodies and incomplete/corrupt tiles before publishing the cache."""
    data = Path(path).read_bytes()
    if data[:8] != b"\x89PNG\r\n\x1a\n":
        raise ValueError(f"Expected PNG imagery: {path}")
    offset, has_header, has_data = 8, False, False
    while offset + 12 <= len(data):
        length, kind = struct.unpack_from(">I4s", data, offset)
        end = offset + 12 + length
        if end > len(data):
            raise ValueError(f"Truncated PNG imagery: {path}")
        payload = data[offset + 8:end - 4]
        checksum = struct.unpack_from(">I", data, end - 4)[0]
        if zlib.crc32(kind + payload) & 0xffffffff != checksum:
            raise ValueError(f"Invalid PNG checksum: {path}")
        if not has_header:
            if kind != b"IHDR" or length != 13 or struct.unpack_from(">II", payload) != (256, 256):
                raise ValueError(f"Expected 256x256 PNG imagery: {path}")
            has_header = True
        if kind == b"IDAT":
            has_data = True
        if kind == b"IEND":
            if length or end != len(data) or not has_data:
                raise ValueError(f"Invalid PNG ending: {path}")
            return
        offset = end
    raise ValueError(f"Incomplete PNG imagery: {path}")


def install_imagery(download, root, bbox, minimum, maximum, workers):
    jobs = []
    for zoom in range(minimum, maximum + 1):
        west, north = tile_coordinate(bbox[0], bbox[3], zoom)
        east, south = tile_coordinate(bbox[2], bbox[1], zoom)
        count = (east - west + 1) * (south - north + 1)
        if len(jobs) + count > 15000:
            raise ValueError("Requested more than 15000 photo tiles; choose a smaller bounded cache")
        for x in range(west, east + 1):
            for y in range(north, south + 1):
                jobs.append((IMAGERY_URL.format(z=zoom, x=x, y=y),
                             root / "imagery" / IMAGERY_NAME / str(zoom) / str(x) / f"{y}.png"))
    source_catalog = download.get(IMAGERY_CATALOG, root / "imagery" / IMAGERY_NAME / "source-catalog.json")
    if not any(entry.get("name") == IMAGERY_NAME for entry in json.loads(source_catalog.read_text()).get("tiles", [])):
        raise ValueError("The pinned photographic imagery source is absent from the official catalog")

    def fetch(job):
        path = download.get(*job)
        try:
            verify_png(path)
        except ValueError:
            path.unlink(missing_ok=True)
            raise

    with ThreadPoolExecutor(max_workers=workers) as executor:
        for index, _ in enumerate(executor.map(fetch, jobs), 1):
            if index % 250 == 0:
                print(f"PLATEAU aerial imagery: {index}/{len(jobs)} tiles", flush=True)
    # Keep downloaded source pixels unchanged; do not create artificial detail by
    # increasing the zoom above the compiled source's actual native tile limit.
    latitude = (bbox[1] + bbox[3]) / 2
    resolution = 2 * math.pi * 6378137 * math.cos(math.radians(latitude)) / (256 * 2**maximum)
    return {"url": "/models/imagery/" + IMAGERY_NAME + "/{z}/{x}/{y}.png", "rectangle": bbox,
            "minimumLevel": minimum, "maximumLevel": maximum, "tiles": len(jobs),
            "source": IMAGERY_URL, "sourceCatalog": IMAGERY_CATALOG,
            "type": "aerial orthophotography", "edition": "2024",
            "pixelSpacingMetresAtCentre": round(resolution, 3),
            "resolutionNote": "Zoom 19 is the highest compiled tile level available at Tokyo Station; higher zoom is not synthesized. Pixel spacing is not a claim about original capture resolution.",
            "modification": "Geographic tile selection only; original PNG bytes retained",
            "attribution": "東京都 / 国土交通省 Project PLATEAU / 国土地理院（航空写真）",
            "attributionUrl": IMAGERY_DOCUMENTATION,
            "license": "PLATEAU Site Policy, section 3",
            "usageTerms": "https://www.mlit.go.jp/plateau/site-policy/",
            "baseImageryUsageTerms": "https://maps.gsi.go.jp/help/use.html"}


def localize_terrain(data):
    """Keep mesh/normals; remove dynamic availability that advertises uncached tiles."""
    if data[:2] == b"\x1f\x8b":
        data = gzip.decompress(data)
    if len(data) < 96:
        raise ValueError("Truncated quantized-mesh tile")
    vertices = struct.unpack_from("<I", data, 88)[0]
    width = 2 if vertices <= 65536 else 4
    offset = 92 + vertices * 6
    offset = (offset + width - 1) // width * width
    triangles = struct.unpack_from("<I", data, offset)[0]
    offset += 4 + triangles * 3 * width
    for _ in range(4):
        count = struct.unpack_from("<I", data, offset)[0]
        offset += 4 + count * width
    if offset > len(data):
        raise ValueError("Truncated quantized-mesh indices")
    output = bytearray(data[:offset])
    while offset < len(data):
        extension, length = struct.unpack_from("<BI", data, offset)
        end = offset + 5 + length
        if end > len(data):
            raise ValueError("Truncated quantized-mesh extension")
        if extension != 4:
            output.extend(data[offset:end])
        offset = end
    return bytes(output)


def install_terrain(download, root, bbox, workers):
    target = root / "terrain" / "plateau"
    source = download.get(TERRAIN_URL + "layer.json", target / "source-layer.json")
    layer = json.loads(source.read_text())
    if layer.get("projection") != "EPSG:4326" or layer.get("scheme") != "tms":
        raise ValueError("Expected PLATEAU geographic TMS terrain")
    available = []
    jobs = []
    # Tokyo's source tiles terminate at level 15 (about 10 m vertex spacing).
    # Both global roots remain present; terrain elsewhere is upsampled from
    # downloaded ancestors. A one-tile margin avoids edge gaps near the cache.
    for level in range(16):
        columns, rows = 2 ** (level + 1), 2 ** level
        x0 = max(0, math.floor((bbox[0] + 180) / 360 * columns) - 1)
        x1 = min(columns - 1, math.floor((bbox[2] + 180) / 360 * columns) + 1)
        y0 = max(0, math.floor((bbox[1] + 90) / 180 * rows) - 1)
        y1 = min(rows - 1, math.floor((bbox[3] + 90) / 180 * rows) + 1)
        if level == 0:
            x0, x1, y0, y1 = 0, 1, 0, 0
        available.append([{"startX": x0, "startY": y0, "endX": x1, "endY": y1}])
        for x in range(x0, x1 + 1):
            for y in range(y0, y1 + 1):
                relative = f"{level}/{x}/{y}.terrain"
                jobs.append((TERRAIN_URL + relative, target / relative))

    def fetch(job):
        existed = job[1].is_file()
        path = download.get(*job)
        try:
            localized = localize_terrain(path.read_bytes())
        except (ValueError, EOFError, struct.error, zlib.error, gzip.BadGzipFile):
            # A new incomplete source must not poison a later resume. Keep any
            # pre-existing file intact if its format is unsupported.
            if not existed:
                path.unlink(missing_ok=True)
            raise
        temporary = path.with_suffix(path.suffix + '.tmp')
        try:
            temporary.write_bytes(localized)
            temporary.replace(path)
        finally:
            temporary.unlink(missing_ok=True)

    with ThreadPoolExecutor(max_workers=workers) as executor:
        for index, _ in enumerate(executor.map(fetch, jobs), 1):
            if index % 50 == 0:
                print(f"PLATEAU terrain: {index}/{len(jobs)} tiles", flush=True)
    layer.pop("metadataAvailability", None)
    layer["extensions"] = [value for value in layer.get("extensions", []) if value != "metadata"]
    layer["available"] = available
    layer["maxzoom"] = 15
    layer["tiles"] = ["{z}/{x}/{y}.terrain"]
    layer["description"] = "Offline Tokyo subset; dynamic availability removed; other locations use coarse ancestors"
    temporary = target / 'layer.json.tmp'
    try:
        temporary.write_text(json.dumps(layer, ensure_ascii=False, separators=(",", ":")) + "\n")
        temporary.replace(target / 'layer.json')
    finally:
        temporary.unlink(missing_ok=True)
    return {"url": "/models/terrain/plateau/", "source": TERRAIN_URL,
            "rectangle": bbox, "maximumLevel": 15, "tiles": len(jobs),
            "heightDatum": "ellipsoidal (source geoid correction retained)",
            "modification": "Selected tiles plus ancestors/margin; gzip decoded; dynamic availability removed",
            "attribution": "PLATEAU | Mapterhorn | 国土地理院",
            "sourceUrl": "https://docs.plateauview.mlit.go.jp/datasets/terrain/"}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--models-root", type=Path, default=Path("/home/kai/dev/models/webMavlink"))
    parser.add_argument("--bbox", type=float, nargs=4, default=BBOX, metavar=("WEST", "SOUTH", "EAST", "NORTH"))
    parser.add_argument("--buildings-bbox", type=float, nargs=4, default=BUILDINGS_BBOX,
                        metavar=("WEST", "SOUTH", "EAST", "NORTH"),
                        help="Independent building tile selection rectangle (default: all Chiyoda)")
    parser.add_argument("--terrain-bbox", type=float, nargs=4, default=TERRAIN_BBOX,
                        metavar=("WEST", "SOUTH", "EAST", "NORTH"),
                        help="Independent terrain rectangle (default: original Tokyo Station cache)")
    parser.add_argument("--min-zoom", type=int, default=12)
    parser.add_argument("--max-zoom", type=int, default=19)
    parser.add_argument("--max-mib", type=int, default=2048)
    parser.add_argument("--workers", type=int, default=4)
    parser.add_argument("--skip-vendor", action="store_true")
    parser.add_argument("--skip-buildings", action="store_true")
    parser.add_argument("--skip-imagery", action="store_true")
    parser.add_argument("--skip-terrain", action="store_true")
    args = parser.parse_args()
    for bbox in (args.bbox, args.buildings_bbox, args.terrain_bbox):
        west, south, east, north = bbox
        if not (-180 <= west < east <= 180 and -85 < south < north < 85):
            parser.error("Invalid geographic rectangle")
    if not (10 <= args.min_zoom <= args.max_zoom <= 19 and 1 <= args.workers <= 8 and args.max_mib > 0):
        parser.error("Require 10 <= zooms <= 19, 1..8 workers, and positive max-mib")
    if shutil.which("curl") is None:
        parser.error("curl must be installed")
    root = args.models_root.resolve()
    root.mkdir(parents=True, exist_ok=True)
    download = Downloader(args.max_mib * 1024 * 1024)
    if not args.skip_vendor:
        install_vendor(download)
    manifest_path = root / "asset-manifest.json"
    manifest = json.loads(manifest_path.read_text()) if manifest_path.exists() else {}
    # Keep a user's converted CAD model when refreshing map data.
    drone_path = root / "drone" / "multirotor.glb"
    if not drone_path.exists():
        create_model(drone_path)
        shutil.copyfile(VIEWER.parents[2] / "LICENSE", root / "drone" / "LICENSE")
        manifest["drone"] = {"url": "/models/drone/multirotor.glb",
                             "axes": "+X forward, +Y up, +Z right", "units": "metres",
                             "source": "Original OpenKAI procedural geometry", "license": "AGPL-3.0"}
    elif "drone" not in manifest:
        manifest["drone"] = {"url": "/models/drone/multirotor.glb",
                             "source": "Existing user-supplied model; preserved during map refresh"}
    manifest.update({"format": "openkai-mavlink-assets/1", "cesiumVersion": "1.138.0",
                     "threeVersion": "0.185.0"})
    natural = VIEWER / "vendor" / "cesium" / "Assets" / "Textures" / "NaturalEarthII"
    if natural.is_dir():
        shutil.copytree(natural, root / "imagery" / "natural-earth", dirs_exist_ok=True)
        manifest["baseImagery"] = {"url": "/models/imagery/natural-earth/", "source": "Natural Earth II / CesiumJS",
                                   "license": "Public domain", "sourceUrl": "https://www.naturalearthdata.com/about/terms-of-use/"}
    if not args.skip_buildings:
        manifest["buildings"] = install_buildings(download, root, args.buildings_bbox, args.workers)
    if not args.skip_imagery:
        manifest["imagery"] = install_imagery(download, root, args.bbox, args.min_zoom, args.max_zoom, args.workers)
    if not args.skip_terrain:
        manifest["terrain"] = install_terrain(download, root, args.terrain_bbox, args.workers)
    (root / "ATTRIBUTION.txt").write_text(
        "Buildings: 東京都 / 国土交通省 Project PLATEAU, Chiyoda 2025.\n"
        "Selected intersecting tiles; buildings are not geometrically clipped.\n" + DATASET_URL + "\n"
        "License: https://www.mlit.go.jp/plateau/site-policy/ (section 3).\n"
        "Photographic imagery: 東京都 / 国土交通省 Project PLATEAU / 国土地理院（航空写真）, 2024 edition.\n"
        "Original PNG tiles; geographic subset only. Highest source tile zoom: 19.\n"
        + IMAGERY_DOCUMENTATION + "\n" + IMAGERY_CATALOG + "\n"
        "License: https://www.mlit.go.jp/plateau/site-policy/ (section 3).\n"
        "GSI photographic base: https://maps.gsi.go.jp/development/ichiran.html\n"
        "https://maps.gsi.go.jp/help/use.html\n"
        "Terrain: PLATEAU | Mapterhorn | 国土地理院\n"
        "https://docs.plateauview.mlit.go.jp/datasets/terrain/\n"
        "Offline subset, gzip decoded, dynamic availability removed.\n"
        "Global overview: Natural Earth II (public domain), distributed with CesiumJS.\n"
        "https://www.naturalearthdata.com/about/terms-of-use/\n"
        "Drone: " + manifest["drone"].get("source", "User-supplied model") + ".\n"
        "See asset-manifest.json and any source-model documentation in drone/.\n")
    manifest["files"] = [{"path": str(p.relative_to(root)), "bytes": p.stat().st_size, "sha256": digest(p)}
                         for p in sorted(root.rglob("*")) if p.is_file() and p != manifest_path]
    manifest_path.write_text(json.dumps(manifest, ensure_ascii=False, indent=2) + "\n")
    print(f"Ready: {manifest_path}; {sum(item['bytes'] for item in manifest['files']) / 1024**2:.1f} MiB", flush=True)


if __name__ == "__main__":
    main()
