#!/usr/bin/env python3
"""Exercise exclusive map selection and provider recovery in real Chrome.

All providers are served by a loopback fixture; no outside network or backend is
used. Requires Chrome/Chromium and Python's standard library only.
"""
import importlib.util
from http.server import SimpleHTTPRequestHandler, ThreadingHTTPServer
import json
import math
from pathlib import Path
import struct
import threading
from urllib.parse import urlsplit
import zlib

ROOT = Path(__file__).resolve().parents[4]
VIEWER = ROOT / "html/viewer/mavlink"
spec = importlib.util.spec_from_file_location("live_smoke", Path(__file__).with_name("live-smoke.py"))
live = importlib.util.module_from_spec(spec)
spec.loader.exec_module(live)
wait_for = live.browser_tools.wait_for


def png(red, green, blue):
    """Opaque 256px fixture without Pillow or other image dependencies."""
    def chunk(kind, data):
        return struct.pack(">I", len(data)) + kind + data + struct.pack(">I", zlib.crc32(kind + data))
    rows = (b"\0" + bytes((red, green, blue)) * 256) * 256
    return (b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", 256, 256, 8, 2, 0, 0, 0))
            + chunk(b"IDAT", zlib.compress(rows)) + chunk(b"IEND", b""))


HARNESS = b"""<!doctype html><html><head><meta charset="utf-8">
<link rel="stylesheet" href="/vendor/cesium/Widgets/widgets.css">
<style>html,body,#map{width:100%;height:100%;margin:0;overflow:hidden}</style>
<script>window.CESIUM_BASE_URL='/vendor/cesium/'</script>
<script src="/vendor/cesium/Cesium.js"></script></head><body><div id="map"></div>
<script type="module">
import {MapViewer} from '/js/mapViewer.js';
window.statuses={};
window.map=new MapViewer('map',(key,text,error=false)=>{statuses[key]={text,error}},()=>{});
window.sceneErrors=[];
map.viewer.scene.renderError.addEventListener((scene,error)=>sceneErrors.push(String(error)));
window.centerPixel=()=>{
  map.viewer.scene.requestRender(); map.viewer.scene.render();
  const source=map.viewer.canvas, canvas=document.createElement('canvas');
  canvas.width=source.width;canvas.height=source.height;
  const context=canvas.getContext('2d');context.drawImage(source,0,0);
  return [...context.getImageData(Math.floor(canvas.width/2),Math.floor(canvas.height/2),1,1).data];
};
window.moveTo=(longitude,latitude)=>{
  map.viewer.camera.setView({destination:Cesium.Cartesian3.fromDegrees(longitude,latitude,10000),
    orientation:{heading:0,pitch:-Math.PI/2,roll:0}});
  map.viewer.scene.requestRender();
};
</script></body></html>"""


class Fixture(ThreadingHTTPServer):
    daemon_threads = True

    def __init__(self):
        self.online_fails = False
        self.downloaded_fails = False
        self.requests = []
        self.metadata_started = threading.Event()
        self.metadata_release = threading.Event()
        self.coverage_started = threading.Event()
        self.coverage_release = threading.Event()
        self.coverage = {}
        super().__init__(("127.0.0.1", 0), Handler)


class Handler(SimpleHTTPRequestHandler):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, directory=str(VIEWER), **kwargs)

    def log_message(self, *_):
        pass

    def respond(self, content, kind, status=200):
        self.send_response(status)
        self.send_header("Content-Type", kind)
        self.send_header("Content-Length", str(len(content)))
        self.send_header("Cache-Control", "no-store")
        self.end_headers()
        try:
            self.wfile.write(content)
        except (BrokenPipeError, ConnectionResetError):
            pass

    def do_GET(self):
        path = urlsplit(self.path).path
        self.server.requests.append(path)
        if path == "/imagery-test.html":
            return self.respond(HARNESS, "text/html")
        if path in ("/coverage.json", "/delayed-coverage.json"):
            if path == "/delayed-coverage.json":
                self.server.coverage_started.set()
                self.server.coverage_release.wait(10)
            return self.respond(json.dumps(self.server.coverage).encode(), "application/json")
        if path.startswith(("/online/", "/replacement/")):
            if self.server.online_fails:
                return self.respond(b"Provider temporarily unavailable", "text/plain", 503)
            return self.respond(png(32, 64, 190), "image/png")
        if path.startswith("/streets/"):
            return self.respond(png(190, 64, 32), "image/png")
        if path.startswith("/elevation/"):
            return self.respond(png(190, 32, 190), "image/png")
        if path.startswith("/downloaded/"):
            if self.server.downloaded_fails:
                return self.respond(b"Tile not downloaded", "text/plain", 404)
            return self.respond(png(32, 180, 64), "image/png")
        if path.rstrip("/") == "/delayed-arcgis":
            self.server.metadata_started.set()
            self.server.metadata_release.wait(10)
            metadata = {
                "currentVersion": 10.9, "copyrightText": "Fixture provider",
                "tileInfo": {"rows": 256, "cols": 256, "format": "PNG",
                             "spatialReference": {"wkid": 102100},
                             "lods": [{"level": level} for level in range(6)]},
                "fullExtent": {"xmin": -20037508.342789, "ymin": -20037508.342789,
                               "xmax": 20037508.342789, "ymax": 20037508.342789,
                               "spatialReference": {"wkid": 102100}},
            }
            return self.respond(json.dumps(metadata).encode(), "application/json")
        if path.startswith("/delayed-arcgis/tile/"):
            return self.respond(png(32, 64, 190), "image/png")
        if path == "/models/drone/multirotor.glb":
            model = Path("/home/kai/dev/models/webMavlink/drone/multirotor.glb")
            return self.respond(model.read_bytes(), "model/gltf-binary")
        return super().do_GET()


def layer_urls(browser):
    return browser.evaluate("Array.from({length:map.viewer.imageryLayers.length},(_,i)=>String(map.viewer.imageryLayers.get(i).imageryProvider.url))")


def assert_only(browser, path):
    urls = layer_urls(browser)
    assert len(urls) == 1 and path in urls[0], urls
    assert browser.evaluate("map.viewer.scene.primitives.contains(window.buildingSentinel) && window.buildingSentinel.show"), "Map selection removed or hid building primitives"


def select(browser, source):
    browser.evaluate("map.mapSources.select(" + json.dumps(source) + ")")
    wait_for(browser, "map.mapSources.state === 'ready'", timeout=30)


def neutral_pixel(browser):
    # The neutral globe should be uncovered, without Cesium stretching the
    # downloaded source's edge tiles over the entire planet as a base layer.
    return wait_for(browser, "(()=>{const p=centerPixel();return p[0]<65&&p[1]<90&&p[2]<105&&p[3]===255?p:false})()", timeout=30)


def blue_pixel(browser):
    return wait_for(browser, "(()=>{const p=centerPixel();return p[2]>p[1]+60&&p[2]>p[0]+60?p:false})()", timeout=30)


def main():
    with Fixture() as server:
        thread = threading.Thread(target=server.serve_forever, daemon=True)
        thread.start()
        origin = f"http://localhost:{server.server_port}"
        try:
            with live.Browser() as browser:
                browser.call("Network.emulateNetworkConditions", offline=False, latency=0,
                             downloadThroughput=-1, uploadThroughput=-1)
                browser.call("Page.navigate", url=origin + "/imagery-test.html")
                wait_for(browser, "window.map && map.viewer.imageryLayers.length === 1", timeout=30)
                config = {
                    "initialView": {"longitude": 139.7671, "latitude": 35.6812,
                                    "height": 10000, "heading": 0, "pitch": -90},
                    "imagery": {"url": "/downloaded/{z}/{x}/{y}.png", "maximumLevel": 14,
                                "rectangle": [139.725, 35.665, 139.79, 35.71], "credit": "Local fixture"},
                    "onlineImagery": {"enabled": True, "provider": "xyz",
                                      "url": origin + "/online/{z}/{x}/{y}.png",
                                      "maximumLevel": 8, "credit": "Online fixture"},
                    "mapSources": {
                        "esriMap": {"provider": "xyz", "url": origin + "/streets/{z}/{x}/{y}.png", "maximumLevel": 8},
                        "openStreetMap": {"provider": "xyz", "url": origin + "/streets/{z}/{x}/{y}.png", "maximumLevel": 8},
                        "gsiElevation": {"provider": "xyz", "url": origin + "/elevation/{z}/{x}/{y}.png", "maximumLevel": 8},
                    },
                    "buildings": [], "terrain": None,
                    "drone": {"url": "/models/drone/multirotor.glb"},
                }
                browser.evaluate("window.fixtureConfig=" + json.dumps(config))
                browser.evaluate("map.configure(fixtureConfig,location.origin)")
                browser.evaluate("window.buildingSentinel=map.viewer.scene.primitives.add(new Cesium.PrimitiveCollection());true")
                select(browser, "downloaded-satellite")
                local_pixel = wait_for(browser, "(()=>{const p=centerPixel();return p[1]>p[2]+60&&p[1]>p[0]+60?p:false})()", timeout=30)
                assert_only(browser, "/downloaded/")

                # Exclusivity applies outside cached coverage as well: no world
                # background or online layer may leak in, and no edge stretching.
                start = len(server.requests)
                browser.evaluate("moveTo(135.5023,34.6937)")
                uncovered_pixel = neutral_pixel(browser)
                assert_only(browser, "/downloaded/")
                assert not any(path.startswith(("/online/", "/streets/", "/elevation/")) for path in server.requests[start:])

                # Explicitly selecting online satellite must show blue pixels
                # even inside the area where green downloaded tiles are present.
                browser.evaluate("moveTo(139.7671,35.6812)")
                select(browser, "esri-satellite")
                online_pixel = blue_pixel(browser)
                assert_only(browser, "/online/")
                browser.evaluate("moveTo(135.5023,34.6937)")
                blue_pixel(browser)
                select(browser, "osm-map")
                streets_pixel = wait_for(browser, "(()=>{const p=centerPixel();return p[0]>p[1]+60&&p[0]>p[2]+60?p:false})()", timeout=30)
                assert_only(browser, "/streets/")
                select(browser, "gsi-elevation")
                elevation_pixel = wait_for(browser, "(()=>{const p=centerPixel();return p[0]>p[1]+60&&p[2]>p[1]+60?p:false})()", timeout=30)
                assert_only(browser, "/elevation/")

                # A provider outage leaves the selected source unchanged and
                # exposes neutral earth; it must never silently add another map.
                server.online_fails = True
                browser.evaluate("map.mapSources.select('esri-satellite')")
                wait_for(browser, "map.mapSources.state === 'unavailable'", timeout=30)
                assert browser.evaluate("map.mapSources.selectedId") == "esri-satellite"
                assert layer_urls(browser) == [], layer_urls(browser)
                neutral_pixel(browser)
                server.online_fails = False
                wait_for(browser, "map.mapSources.state === 'ready'", timeout=30)
                blue_pixel(browser)
                assert_only(browser, "/online/")

                browser.evaluate("window.dispatchEvent(new Event('offline'))")
                wait_for(browser, "map.mapSources.state === 'offline'", timeout=10)
                assert layer_urls(browser) == [], layer_urls(browser)
                offline_pixel = neutral_pixel(browser)
                assert browser.evaluate("map.mapSources.selectedId") == "esri-satellite"
                browser.evaluate("window.dispatchEvent(new Event('online'))")
                wait_for(browser, "map.mapSources.state === 'ready'", timeout=30)
                blue_pixel(browser)
                assert_only(browser, "/online/")

                # Metadata finishing after a selection change must not resurrect
                # the abandoned provider, even when an online event follows it.
                browser.evaluate("window.pendingConfigure=map.configure({...fixtureConfig,onlineImagery:{enabled:true,provider:'arcgis',url:location.origin+'/delayed-arcgis',maximumLevel:5}},location.origin);true")
                browser.evaluate("window.pendingSelection=map.mapSources.select('esri-satellite');true")
                assert server.metadata_started.wait(5), "ArcGIS metadata request not started"
                select(browser, "downloaded-satellite")
                server.metadata_release.set()
                browser.evaluate("pendingSelection")
                browser.evaluate("window.dispatchEvent(new Event('online'))")
                assert browser.evaluate("map.mapSources.selectedId") == "downloaded-satellite"
                assert_only(browser, "/downloaded/")
                browser.evaluate("moveTo(139.7671,35.6812)")
                wait_for(browser, "(()=>{const p=centerPixel();return p[1]>p[2]+60&&p[1]>p[0]+60})()", timeout=30)

                # Reconfiguring at reconnect keeps the user's source choice and
                # neither duplicates imagery layers nor changes the source.
                browser.evaluate("map.configure(fixtureConfig,location.origin)")
                wait_for(browser, "map.mapSources.state === 'ready'", timeout=30)
                assert browser.evaluate("map.mapSources.selectedId") == "downloaded-satellite"
                assert_only(browser, "/downloaded/")

                # A sparse Tokyo-wide cache intentionally excludes locations
                # inside its bbox. Cesium may start at z12 across >4 root tiles.
                def tile(longitude, latitude, level):
                    count = 2 ** level
                    return (int((longitude + 180) / 360 * count),
                            int((1 - math.asinh(math.tan(math.radians(latitude))) / math.pi) / 2 * count))
                server.coverage = {"format": "openkai-tile-coverage/1", "minimumLevel": 12, "maximumLevel": 14,
                                   "levels": {}}
                for level in range(12, 15):
                    west, north = tile(139.725, 35.710, level)
                    east, south = tile(139.790, 35.665, level)
                    server.coverage["levels"][str(level)] = {str(x): [[north, south]] for x in range(west, east + 1)}
                sparse = {**config, "imagery": {**config["imagery"], "minimumLevel": 12,
                           "rectangle": [139.5627, 35.5281, 139.9190, 35.8178], "availableTilesUrl": "/coverage.json"}}
                browser.evaluate("window.sparseConfig=" + json.dumps(sparse))
                browser.evaluate("map.configure(sparseConfig,location.origin)")
                wait_for(browser, "map.mapSources.state === 'ready'", timeout=30)
                wait_for(browser, "(()=>{const p=centerPixel();return p[1]>p[2]+60&&p[1]>p[0]+60})()", timeout=30)
                assert_only(browser, "/downloaded/")
                start = len(server.requests)
                missing = tile(139.90, 35.80, 14)
                result = browser.evaluate("""(async()=>{
                  const provider=map.mapSources.layer.imageryProvider;
                  const first=provider.requestImage(...""" + json.dumps([*missing, 14]) + """);
                  const second=provider.requestImage(0,0,12);
                  const canvas=await first;
                  return {shared:first===second,width:canvas.width,height:canvas.height,
                    alpha:canvas.getContext('2d').getImageData(0,0,1,1).data[3]};
                })()""")
                assert result == {"shared": True, "width": 256, "height": 256, "alpha": 0}, result
                assert not any(path.startswith("/downloaded/") for path in server.requests[start:]), server.requests[start:]
                browser.evaluate("moveTo(139.90,35.80)")
                sparse_pixel = neutral_pixel(browser)
                assert_only(browser, "/downloaded/")
                assert browser.evaluate("map.mapSources.state") == "ready"
                assert server.requests.count("/coverage.json") == 1

                # Declared cached tiles still fail normally if their files are
                # missing, rather than silently becoming intentional holes.
                server.downloaded_fails = True
                browser.evaluate("moveTo(139.7671,35.6812);map.configure(sparseConfig,location.origin)")
                wait_for(browser, "map.mapSources.state === 'unavailable'", timeout=30)
                assert layer_urls(browser) == []
                server.downloaded_fails = False

                # Late availability metadata cannot restore a superseded layer.
                browser.evaluate("window.pendingCoverage=map.configure({...sparseConfig,imagery:{...sparseConfig.imagery,availableTilesUrl:'/delayed-coverage.json'}},location.origin);true")
                assert server.coverage_started.wait(5), "Coverage metadata request not started"
                select(browser, "esri-satellite")
                server.coverage_release.set()
                browser.evaluate("pendingCoverage")
                blue_pixel(browser)
                assert_only(browser, "/online/")

                unit_results = browser.evaluate("import('/tests/unit-tests.js').then(module=>module.runTests())")
                assert not browser.evaluate("sceneErrors"), browser.evaluate("sceneErrors")
                exceptions = [event for event in browser.events if event.get("method") == "Runtime.exceptionThrown"]
                assert not exceptions, exceptions
                external = [event["params"]["request"]["url"] for event in browser.events
                            if event.get("method") == "Network.requestWillBeSent"
                            and event["params"]["request"]["url"].startswith(("http://", "https://"))
                            and not event["params"]["request"]["url"].startswith(origin + "/")]
                assert not external, external
                print(json.dumps({"downloadedPixel": local_pixel, "uncoveredPixel": uncovered_pixel,
                                  "onlinePixel": online_pixel, "streetsPixel": streets_pixel,
                                  "elevationPixel": elevation_pixel, "offlinePixel": offline_pixel,
                                  "exclusiveLayers": True, "buildingsPreserved": True,
                                  "unreachableAndRecovery": True, "metadataRace": True,
                                  "sparsePixel": sparse_pixel, "intentionalHolesDoNotRequest": True,
                                  "missingCachedTileFails": True, "coverageMetadataRace": True,
                                  "unitTests": len(unit_results),
                                  "reconnectSelection": True, "externalRequests": external}, indent=2))
        finally:
            server.metadata_release.set()
            server.coverage_release.set()
            server.shutdown()
            thread.join(timeout=5)


if __name__ == "__main__":
    main()
