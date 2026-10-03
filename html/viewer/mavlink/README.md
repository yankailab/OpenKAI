# OpenKAI MAVLink Earth viewer

Run from the repository root:

```sh
cmake --build build -j2
build/OpenKAI jsonCfg/WebMavlinkStream.json
```

Open **http://localhost:8080/** and click **Start**. The embedded HTTP server
serves the entire viewer, including Cesium and three.js. The example listens
for MAVLink UDP packets on **14550**, filtering system **1**, component **1**.
Change `mavlink.devSystemID` / `devComponentID` for your vehicle. The sender must
already stream telemetry; this viewer does not request message rates, send
heartbeats, arm the vehicle, or issue commands. The example has no UDP transmit
stream attached.

For an existing OpenKAI application, copy the `viewer` object into its launch
configuration and set `viewer.MavlinkStream` to its existing `MavlinkStream`
data object. It can share that object with other consumers without consuming
messages. The module is registered with `WITH_UNIVERSE=ON`, as are the existing
web viewers. The standalone example also needs `WITH_IO=ON` and
`WITH_PROTOCOL=ON`. `USE_WSSERVER` is not required for this viewer's Beast-based
HTTP/WebSocket transport.

## Map, models and offline use

`viewer.modelsRoot` defaults to **`/home/kai/dev/models/webMavlink`** and is
served under **`/models`**. `viewer.webRoot` points to `html/viewer/mavlink`.
Both paths are resolved by the backend; browser URLs in `viewer.scene` use the
HTTP mount, not filesystem paths. The mount is read-only and rejects traversal
and symlinks that escape its configured directory.

The local asset layout is:

```text
webMavlink/
  drone/multirotor.glb
  imagery/plateau-ortho-2024/{z}/{x}/{y}.png
  plateau/tokyo-central/tileset.json
  terrain/plateau/layer.json
  ... referenced PLATEAU tile content and attribution/manifest files
```

The vendored Cesium distribution includes Natural Earth II imagery for the
whole globe. High-resolution PLATEAU aerial photography and buildings cover
central Tokyo. Beyond that coverage the globe still works
offline at the Natural Earth resolution. No Cesium ion account, API token,
CDN request, npm process, or separate web server is needed at runtime.

The building cache contains **633 textured Chiyoda 2025 LOD2 tiles**, covering
the entire source district (approximately **4.8 × 4.0 km**), including Tokyo
Station, Marunouchi, Otemachi and western Chiyoda. The photographic surface uses
the **PLATEAU-Ortho 2024 layer at zoom levels 12–19**, with approximately
**0.24 m per pixel** at Tokyo's latitude at the highest level. These are aerial
orthophotos providing a satellite-style surface; the layer name is not a claim
that every photograph was captured in 2024. Level 19 is the highest supported
by the selected endpoint; level 20 was checked and is unavailable, so no
artificially upscaled higher levels are stored.

The **10,495 photographic tiles** cover longitude **139.725–139.790**, latitude
**35.665–35.710**, including a margin around Chiyoda, and occupy approximately
**1.1 GiB**. The existing **208 elevation
tiles** remain in use. The cache is local and bounded; it does not contain all
Tokyo wards. Source URLs, coverage, file sizes and checksums are recorded in
`asset-manifest.json`.

To reproduce the downloaded assets on another machine, run:

```sh
python3 html/viewer/mavlink/tools/download_assets.py --skip-vendor \
  --models-root /home/kai/dev/models/webMavlink
```

Python 3.9+ and `curl` are required only for setup. Omitting `--skip-vendor`
also refreshes the pinned CesiumJS 1.138.0 and three.js r185 distributions,
verifying their archive hashes. The downloader checks embedded building
resources and PNG signatures, records per-file SHA-256 hashes in
`asset-manifest.json`, and retains source/attribution information. Its default
download budget is 2048 MiB. `--bbox WEST SOUTH EAST NORTH` and
`--min-zoom` / `--max-zoom` customize the photographic cache; update
`scene.imagery` to match the resulting manifest. Building and terrain coverage
have independent `--buildings-bbox` and `--terrain-bbox` options. Their defaults
select all Chiyoda buildings and preserve the original terrain area. The building
source remains Chiyoda.

For manual download, the [official Chiyoda 2025 dataset](https://www.geospatial.jp/ckan/dataset/plateau-13101-chiyoda-ku-2025)
provides the PLATEAU city model and 3D Tiles resources. Prefer a prepared **3D
Tiles** building resource, extract it below `modelsRoot/plateau`, and point
`scene.buildings[].url` at its `tileset.json`. Include every referenced tile and
texture, not just the entry-point JSON. For photographs, use the
[PLATEAU orthophoto documentation](https://docs.plateauview.mlit.go.jp/datasets/ortho/)
and its [current tile catalog](https://tile.plateauview.mlit.go.jp/tiles/catalog.json),
preserving `{z}/{x}/{y}.png` and the source attribution. The included downloader
automates both operations for the configured area.

The example uses locally cached [PLATEAU terrain](https://docs.plateauview.mlit.go.jp/datasets/terrain/)
through `scene.terrain.url: "/models/terrain/plateau/"`. Its quantized-mesh
tiles retain the source's ellipsoid height datum and include global root tiles,
ancestors and a margin around the Tokyo extent, through level 15. Terrain outside
the cached extent uses coarser tiles; this is not a worldwide detailed elevation
dataset. Local availability metadata prevents requests for uncached branches.
An empty terrain URL selects the ellipsoid. To use a different Cesium-compatible
terrain dataset, place it inside `modelsRoot` and point the URL at its directory.

`scene.buildings` accepts multiple `{url, credit}` entries, each pointing to a
3D Tiles `tileset.json`. Preserve each tileset's referenced directory hierarchy
when importing additional districts. Downloaded CityGML must first be converted
to 3D Tiles; Cesium cannot render CityGML directly.

Building detail now uses a screen-space error of **2 pixels**, reduced from 8,
and disables distance-based and viewport-edge detail relaxation. Camera motion
does not suppress building tile requests. These settings preserve detail farther
from the camera, including while following the aircraft; the full Chiyoda cache
also removes the former small-area download limit. They do not extend geographic
coverage beyond the source district.

Tune these settings in `viewer.scene.buildingRendering`:

```json
{
  "maximumScreenSpaceError": 2,
  "dynamicScreenSpaceError": false,
  "foveatedScreenSpaceError": false,
  "cullRequestsWhileMoving": false,
  "cacheMegabytes": 1024,
  "maximumCacheOverflowMegabytes": 1024
}
```

The cache and overflow limits apply per tileset and are allocated on demand,
up to **2 GiB** with these defaults. Smaller screen-space error keeps finer
geometry farther away and increases rendering cost. For a GPU with less memory,
reduce both cache settings and raise the screen-space error. Cesium may relax
detail when the configured memory budget is reached.

## Position and attitude

The map drone and the three.js attitude preview both load the same multirotor
GLB. The generated model has a marked nose, four motors/propellers and landing
gear. Raw model axes are **+X forward, +Y up, +Z right**, in metres. The viewer
explicitly transforms this frame into MAVLink body FRD (forward/right/down),
then NED (north/east/down), then Earth-fixed coordinates. It disables Cesium's
automatic glTF axis correction for this model. A zero attitude points north;
positive yaw turns east, positive pitch raises the nose, and positive roll
lowers the right side. Replacing the GLB requires matching these raw axes.

`ATTITUDE_QUATERNION` and `ATTITUDE` are supported; the backend uses the newest
valid sample and sends a normalized WXYZ quaternion plus radians for the panel.
The map keeps a bounded trail and offers locate, follow, Tokyo overview,
buildings visibility and trail controls. These are local viewing controls.
Use **left-drag to orbit**, **right-drag to pan**, and the **mouse wheel to zoom**.

`GLOBAL_POSITION_INT.alt` is altitude above mean sea level, while Cesium uses
height above the WGS84 ellipsoid. The viewer uses
`scene.altitude.geoidSeparationM` when configured, or derives the local
separation from fresh nearby `GPS_RAW_INT` MSL and ellipsoid heights. A previously
measured separation can be retained near that location during GPS dropout and
is labeled as cached. Without either source, it labels the rendered height as
approximate. Relative altitude is
displayed separately and is never substituted for absolute altitude. Set a
locally appropriate geoid separation for accurate alignment with buildings.
See the [MAVLink common message definitions](https://mavlink.io/en/messages/common.html)
and [Cesium model coordinate options](https://cesium.com/learn/cesiumjs/ref-doc/Model.html).

## Dedicated receive-only protocol

The only endpoint is **`/stream/mavlink`**. It uses ordinary WebSocket JSON text
frames, without `EOJ`, geometry headers, subscription messages or acknowledgments.
It is separate from the geometry streams and `_WSconsole`. Clients send no
application messages; an application message closes this read-only connection.

The first frame is:

```json
{"protocol":"openkai.mavlink","version":1,"type":"hello","readOnly":true,"config":{"staleAfterMs":3000}}
```

`config` contains `viewer.scene` with the configured stale timeout added.
Subsequent `type:"telemetry"` frames contain `sequence`, `timeMs`, `connected`,
and these nullable objects:

| Object | Source | Main fields |
| --- | --- | --- |
| `heartbeat` | HEARTBEAT | `armed`, `baseMode`, `customMode`, `systemStatus`, `vehicleType`, `autopilot` |
| `position` | GLOBAL_POSITION_INT | `latitudeDeg`, `longitudeDeg`, `altitudeMslM`, `relativeAltitudeM`, `velocityNedMps`, `groundSpeedMps`, `headingDeg` |
| `attitude` | ATTITUDE / ATTITUDE_QUATERNION | `rollRad`, `pitchRad`, `yawRad`, `quaternionWxyz`, `angularVelocityRadS` |
| `battery` | BATTERY_STATUS / SYS_STATUS fallback | `remainingPct`, `voltageV`, `currentA`, `temperatureC`, `consumedMah`, `id` |
| `gps` | GPS_RAW_INT | `fixType`, `satellitesVisible`, `hdop`, `vdop`, `altitudeEllipsoidM` |
| `system` | SYS_STATUS | `loadPct`, `voltageV`, `currentA`, `remainingPct` |
| `home` | HOME_POSITION | `latitudeDeg`, `longitudeDeg`, `altitudeMslM` |
| `statusText` | STATUSTEXT | `severity`, `text`, `id`, `chunkSeq` |

Each received object includes `ageMs` and `stale`, based on the backend's
monotonic receive timestamps. Position, attitude, GPS and home also carry
`valid`. Unknown MAVLink sentinel values become JSON `null`. `connected` means
a fresh heartbeat was received, independently of whether the WebSocket is open.
Chunked STATUSTEXT is presented as the latest received chunk, not assembled.

`thread.FPS` controls snapshot publishing (20 Hz in the example).
`staleAfterMs` controls telemetry expiry (3000 ms by default); increase it if the
autopilot deliberately emits battery or other status messages more slowly.
The browser also ages cached values between frames and clears aircraft state on
disconnect/reconnect. Slow connections coalesce intermediate snapshots instead
of building an unbounded queue.

## Verification

```sh
cmake -S test/Net -B /tmp/openkai-http-tests
cmake --build /tmp/openkai-http-tests -j2
ctest --test-dir /tmp/openkai-http-tests --output-on-failure
python3 test/run_web_mavlink_tests.py build
python3 html/viewer/mavlink/tests/live-smoke.py --executable build/OpenKAI
```

The integration test launches a temporary backend on loopback ports, injects
known MAVLink UDP telemetry, and uses Chrome/Chromium to check rendered values,
local Cesium/three.js resources, no outbound application frames, staleness and
reconnection. It never connects to an autopilot. `--models-root` selects another
asset directory; `--screenshot /tmp/mavlink.png` captures the actual viewer.
Add `--overview-screenshot /tmp/chiyoda.png` to also verify and capture western
Chiyoda buildings from a camera height of 3.5 km.
Chrome's host resolver blocks internet access during this test.
