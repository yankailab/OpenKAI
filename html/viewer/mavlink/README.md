# OpenKAI MAVLink Earth viewer

Run from the repository root:

```sh
cmake --build build -j2
build/OpenKAI jsonCfg/WebMavlinkStream.json
```

Open **http://localhost:8080/** and click **Start**. The embedded HTTP server
serves the entire viewer, including Cesium and three.js. The example listens
for viewer MAVLink UDP packets on **14552**, filtering system **1**, component **1**.
Change `mavlink.devSystemID` / `devComponentID` for your vehicle. The sender must
already stream telemetry; this viewer does not request message rates, send
heartbeats, arm the vehicle, or issue commands. The viewer's `udpMavlink` connection
has no transmit stream attached. A separate `udpSitl` connection on **14551**
provides the transport wiring for future OpenKAI handlers.

For an existing OpenKAI application, copy the `viewer` object into its launch
configuration and set `viewer.MavlinkStream` to its existing `MavlinkStream`
data object. It can share that object with other consumers without consuming
messages. The module is registered with `WITH_UNIVERSE=ON`, as are the existing
web viewers. The standalone example also needs `WITH_IO=ON` and
`WITH_PROTOCOL=ON`. `USE_WSSERVER` is not required for this viewer's Beast-based
HTTP/WebSocket transport.

## ArduCopter SITL and QGroundControl

With Copter SITL already built in `/home/kai/dev/ardupilot`, run this in another
terminal from the OpenKAI repository root:

```sh
docs/ArduPilot/run_web_mavlink_sitl.sh
```

The [launcher](../../../docs/ArduPilot/run_web_mavlink_sitl.sh) loads
[WebMavlinkStream.parm](../../../jsonCfg/ardupilot/WebMavlinkStream.parm). Its
`SIM_OPOS_*` parameters start the simulation near Tokyo Station's Marunouchi plaza
at **35.6812, 139.7655**, heading north. Ground altitude **5 m MSL** is a chosen
simulation value, not a surveyed elevation. The parameter file also sets the
vehicle ID and telemetry rates. Map windows and UDP destinations are launch
options; they cannot be configured in a `.parm` file. This launcher omits
`--map` and `--console` and disables the implicit UDP outputs.

| MAVProxy destination | Receiving application | Role |
| --- | --- | --- |
| `127.0.0.1:14550` | QGroundControl | Telemetry and commands |
| `127.0.0.1:14551` | OpenKAI `udpSitl` → `mavlinkSitl` → `sitlVehicle` | Future bidirectional handlers |
| `127.0.0.1:14552` | OpenKAI `udpMavlink` → `mavlink` → `vehicle` | Passive web visualization |

Three destinations supply the two OpenKAI UDP connections plus QGC. OpenKAI and
QGC bind the UDP listeners; MAVProxy sends to them and relays replies to SITL.
Enable QGC's automatic UDP connection on port **14550**. The browser receives
telemetry through **http://localhost:8080/** and WebSocket, with no UDP connection.
The two OpenKAI decoders use separate MAVLink channels **1** and **0**.

`udpSitl.bW2R:true` learns MAVProxy's return address from received packets.
Future handlers should use `sitlVehicle`; the receive-only viewer uses `vehicle`.
Command sending still requires completing the existing
`MavMsgBase::getMsgQueue` TODO. When implementing handlers, pass sender system
**200**, component **191** explicitly to the message setters: the decoder's
configured `mySystemID` / `myComponentID` do not currently set encoded IDs.

The launcher stores simulation state in `build/sitl-web-mavlink`. Existing
`eeprom.bin` values can override parameter-file defaults. To reset this dedicated
simulation's saved parameters, run `docs/ArduPilot/run_web_mavlink_sitl.sh -w`.
`ARDUPILOT_DIR` and `SITL_RUN_DIR` override the checkout and state directory;
additional arguments are passed to `sim_vehicle.py`. See the
[development setup](../../../docs/ArduPilot/AP_dev_setup.md#webmavlinkstream-with-tokyo-station-sitl)
for details and the official [SITL](https://ardupilot.org/dev/docs/using-sitl-for-ardupilot-testing.html)
and [MAVProxy forwarding](https://ardupilot.org/mavproxy/docs/getting_started/forwarding.html)
documentation.

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
  imagery/plateau-ortho-2024/coverage.json
  imagery/gsi/{z}/{x}/{y}.png
  imagery/gsi/coverage.json
  plateau/tokyo-23wards/tileset.json
  plateau/tokyo-23wards/{ward-code}/tileset.json
  plateau/tokyo-central/...  # retained Chiyoda files, reused by the aggregate
  terrain/plateau/layer.json
  ... referenced PLATEAU tile content and attribution/manifest files
```

Use the **Source** and **Type** dropdowns at the upper left to choose the map.
Only one source/type is rendered at a time: switching removes the previous
imagery instead of overlaying local and online tiles. Downloaded PLATEAU
buildings stay visible with every choice, and the same terrain geometry remains
in use. Cesium and three.js are served locally.

| Source | Available types |
| --- | --- |
| Downloaded | PLATEAU satellite-style aerial photos; cached GSI map; terrain elevation colors |
| Esri | Satellite; street map |
| OpenStreetMap | Street map |
| GSI | Elevation relief (Japan) |
| Natural Earth | Offline world overview |

The default is `viewer.scene.mapSource: "downloaded-satellite"`. The browser
retains an explicitly chosen source through telemetry reconnects. Areas outside
a downloaded source's coverage remain untextured; selecting an online source
or Natural Earth changes the whole globe's map source. Failed online tiles also
leave an untextured surface, without silently switching to another provider.

`viewer.scene.downloadedMap` describes the optional cached GSI street-map tiles.
Online options are enabled by `viewer.scene.onlineImagery`, which also configures
the Esri satellite source:

```json
{
  "enabled": true,
  "provider": "arcgis",
  "url": "https://services.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer",
  "maximumLevel": 19
}
```

This public [World Imagery service](https://services.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer)
works without an API key and supplies its imagery attribution to Cesium.
Resolution varies by location and available source imagery. Requests go directly
from the browser to the provider; online tiles are not downloaded into
`modelsRoot`.

Set `enabled` to `false` for strictly offline operation with no online map
requests. Configurations without `onlineImagery` also remain offline. The
viewer reports the selected source's availability in **Map & model status**.
It retries failed connections after 5, 15, and 30 seconds, then at most once a minute, and retries
immediately when the browser reports that it is back online. Restart the backend
and refresh the viewer after changing the JSON configuration.

Other Cesium-compatible ArcGIS MapServer URLs can use `provider: "arcgis"`.
For an XYZ imagery service, use `provider: "xyz"`, a `url` containing
`{z}/{x}/{y}`, `maximumLevel`, and the provider's attribution in `credit`.
XYZ tiles must use Web Mercator and permit browser cross-origin requests.
Choose a service URL intended for third-party map clients and retain its
required attribution. Override `scene.mapSources.esriMap`, `.openStreetMap`, or
`.gsiElevation` to configure those catalogue entries, or set an entry's
`enabled: false` to omit it. Provider access requirements and usage terms apply.
OpenStreetMap is used for interactive viewing with normal browser caching,
according to its [tile policy](https://operations.osmfoundation.org/policies/tiles/).
GSI relief follows its [published tile catalogue](https://maps.gsi.go.jp/development/ichiran.html).

Downloaded elevation colors visualize the existing terrain's height, including
its coarser coverage outside Tokyo. They are a terrain visualization, not newly
downloaded elevation measurements. GSI elevation is a colored relief tile map;
selecting it also leaves the terrain geometry unchanged.

The building cache contains **10,065 building tiles for all 23 Tokyo wards**
from the **2025 PLATEAU datasets**, totaling **15.35 GiB**. Each source combines
textured LOD2 buildings with LOD1 buildings where detailed geometry is unavailable.
The aggregate tileset shares one Cesium building cache across the wards and
reuses the original 633 Chiyoda tiles. The photographic surface uses
the **PLATEAU-Ortho 2024 layer at zoom levels 12–19**, with approximately
**0.24 m per pixel** at Tokyo's latitude at the highest level. These are aerial
orthophotos providing a satellite-style surface; the layer name is not a claim
that every photograph was captured in 2024. Level 19 is the highest supported
by the selected endpoint; level 20 was checked and is unavailable, so no
artificially upscaled higher levels are stored.

The photographic selection contains **221,524 tile coordinates** intersecting
the official MLIT N03 2025 boundaries of wards **13101–13123**. The bounding
rectangle is **139.5627–139.9190° E, 35.5281–35.8178° N**. Original PNG pixels
are retained: **221,515 photographs** were downloaded or reused; nine coastal
coordinates return `Tile not found` from the provider. The same ward footprint
selects **14,616 GSI street-map tiles**
at levels 12–17, and the terrain cache covers the bounding rectangle through
level 15, including ancestors and a margin (**5,431 terrain tiles**).
The complete local asset folder, including the retained drone and source
metadata, occupies approximately **38.35 GiB**.

`scene.imagery.availableTilesUrl` and `scene.downloadedMap.availableTilesUrl`
point to compact coverage files listing successfully cached coordinates. Areas
outside the ward footprint and any source tiles returning 404 stay untextured
without generating missing-file requests or disabling the selected layer.
`source-missing-tiles.json` records unavailable source coordinates. Source URLs,
actual downloaded counts, file sizes and checksums are in `asset-manifest.json`.

To reproduce the downloaded assets on another machine, run:

```sh
python3 -m venv /tmp/openkai-map-cache-venv
/tmp/openkai-map-cache-venv/bin/pip install requests shapely
/tmp/openkai-map-cache-venv/bin/python html/viewer/mavlink/tools/download_tokyo23.py \
  --models-root /home/kai/dev/models/webMavlink --workers 32
```

The downloader resumes completed files, verifies PNG integrity and embedded
building resources, and publishes each dataset only after its transfer succeeds.
`--plan` fetches metadata and reports tile counts; `--only buildings`, `imagery`,
`map` or `terrain` limits a resumed run. The default new-download budget is
80 GiB, with a 5 GiB free-space reserve; adjust `--max-gib` and `--reserve-gib`
if needed. `--inventory PATH` additionally verifies building byte counts and
ZIP CRCs against a saved source inventory. The supplied drone model is retained.

The original `download_assets.py` remains available for a smaller Chiyoda cache
and for installing pinned CesiumJS 1.138.0 / three.js r185 libraries. It requires
Python 3.9+ and `curl`, and has a default 2048 MiB budget. Its building source
remains Chiyoda; use `download_tokyo23.py` for the full ward collection.

For manual download, the [official Chiyoda 2025 dataset](https://www.geospatial.jp/ckan/dataset/plateau-13101-chiyoda-ku-2025)
provides the PLATEAU city model and 3D Tiles resources. Prefer a prepared **3D
Tiles** building resource, extract it below `modelsRoot/plateau`, and point
`scene.buildings[].url` at its `tileset.json`. Include every referenced tile and
texture, not just the entry-point JSON. For photographs, use the
[PLATEAU orthophoto documentation](https://docs.plateauview.mlit.go.jp/datasets/ortho/)
and its [current tile catalog](https://tile.plateauview.mlit.go.jp/tiles/catalog.json),
preserving `{z}/{x}/{y}.png` and the source attribution. The included downloader
automates both operations. The [official Tokyo LOD2 catalog](https://api.plateauview.mlit.go.jp/datacatalog/3dtiles/13-bldg-lod2-latest/tileset.json)
provides the per-ward building entry points; the cache selects codes 13101–13123.

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
from the camera, including while following the aircraft; the full ward cache
extends geographic coverage throughout Tokyo's 23 wards. Detail settings apply
where building tiles have been downloaded.

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

The map drone and the three.js attitude preview both load
`/models/drone/multirotor.glb`, now converted from the supplied
`/home/kai/dev/models/drone.step` (Hydrone White). The cardboard payload box is
removed; the aircraft, gripping clamps and CAD colors are retained. The previous
assembly with its box is backed up as `drone/multirotor-with-payload.glb`, and the
original procedural model as `drone/multirotor-procedural.glb`.

The GLB is approximately **5.44 MiB**, with 218,758 triangles. Its physical
dimensions are **0.3547 m forward × 0.1530 m high × 0.4063 m wide**. The CAD's
millimetres are converted to metres and native `−Y forward / +Z up` becomes
**+X forward, +Y up, +Z right**. The original flight pivot near the rotor array
is preserved when removing the box, keeping the attitude rotation center fixed.
The attitude preview automatically frames the entire assembly; the map uses
its physical dimensions with the existing distance-based visibility enlargement.

The viewer
explicitly transforms this frame into MAVLink body FRD (forward/right/down),
then NED (north/east/down), then Earth-fixed coordinates. It disables Cesium's
automatic glTF axis correction for this model. A zero attitude points north;
positive yaw turns east, positive pitch raises the nose, and positive roll
lowers the right side. Replacing the GLB requires matching these raw axes.

The reusable converter requires an optional CAD environment only during setup:

```sh
python3 -m venv /tmp/openkai-step-venv
/tmp/openkai-step-venv/bin/pip install cadquery-ocp==8.0.1.0.0 numpy
/tmp/openkai-step-venv/bin/python html/viewer/mavlink/tools/convert_step.py \
  --input /home/kai/dev/models/drone.step \
  --exclude-hydrone-payload \
  --output /home/kai/dev/models/webMavlink/drone/multirotor.glb
```

`--linear-deflection-mm` and `--angular-deflection-degrees` control tessellation;
defaults are 0.15 mm and 20°. `--exclude-hydrone-payload` removes only the verified
box from this supplied Hydrone assembly; omit it to export the complete assembly.
The original STEP file is unchanged. The adjacent `multirotor.conversion.json`
records source/output hashes, excluded geometry, scale, axes, bounds and mesh statistics. The source CAD's
rights are retained; `drone/LICENSE` applies to the backed-up procedural model.
Map-data refreshes preserve the installed GLB and its manifest metadata. A fresh
asset setup without an existing model generates the procedural fallback until
the STEP conversion is installed.

`ATTITUDE_QUATERNION` and `ATTITUDE` are supported; the backend uses the newest
valid sample and sends a normalized WXYZ quaternion plus radians for the panel.
The map keeps a bounded trail and offers locate, follow, FPV, Tokyo overview,
map-source selection and trail controls. These are local viewing controls.
The flight trail is **cobalt blue (`#0047AB`)**.
Use **left-drag to orbit**, **right-drag to pan**, and the **mouse wheel to zoom**.

**FPV**, immediately after Follow, places the camera at the aircraft's measured
position and looks along its forward axis. Roll, pitch and yaw all affect the
view, with a 70° field of view. FPV and Follow are mutually exclusive. The map's
own drone model and marker are hidden in FPV, while the attitude preview remains
visible. Mouse camera motion is disabled until FPV is exited. Stale or invalid
position/attitude holds the last camera pose; fresh telemetry resumes tracking.
Click FPV again, Follow, Locate drone or Tokyo to leave FPV.

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
python3 html/viewer/mavlink/tests/imagery-smoke.py
```

The integration test launches a temporary backend on loopback ports, injects
known MAVLink UDP telemetry, and uses Chrome/Chromium to check rendered values,
local Cesium/three.js resources, no outbound application frames, staleness and
reconnection. It never connects to an autopilot. `--models-root` selects another
asset directory; `--screenshot /tmp/mavlink.png` captures the actual viewer.
Add `--overview-screenshot /tmp/chiyoda.png` to also verify and capture western
Chiyoda buildings from a camera height of 3.5 km.
The telemetry test explicitly disables online imagery, and Chrome's host
resolver blocks internet access during both browser tests. The imagery test
uses loopback tile providers to verify exclusive source selection, downloaded
coverage boundaries, selected-provider failure/recovery, and switching while
requests are pending. The telemetry test also checks the source/type controls,
unchanged building primitives, FPV world position and attitude, moving telemetry,
stale-data hold and recovery, mouse locking, and camera settings restored on exit.
