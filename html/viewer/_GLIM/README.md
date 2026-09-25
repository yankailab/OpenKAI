# GLIM SLAM viewer

Sensor-independent GLIM viewer with a dedicated `_WebGLIM` submap stream and
ordinary `_WSconsole` commands. Three.js assets are local; no npm build or
internet connection is required.

From the OpenKAI repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DWITH_SLAM=ON -DUSE_GLIM=ON -DWITH_VISION=ON -DUSE_OPENCV=ON \
  -DUSE_ORBBEC=ON -DWITH_UNIVERSE=ON -DWITH_PROTOCOL=ON \
  -DWITH_IO=ON -DWITH_SENSOR=ON -DUSE_WSSERVER=ON
cmake --build build -j4
./build/OpenKAI jsonCfg/GLIM_orbbec.json
```

Open `http://127.0.0.1:8080`, click **Connect viewer**, then **Start**. Opening
`index.html` directly provides a launcher to the backend HTTP server. Stream and
command ports default to 8080 and 7890. The example starts stopped, with desktop
windows disabled. Keep the Orbbec camera stationary during IMU initialization.

For Scepter, enable its SDK in the same build and select the separate application:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DUSE_SCEPTER_SDK=ON -DScepterSDK_root=/path/to/ScepterSDK/BaseSDK/Ubuntu
cmake --build build -j4
./build/OpenKAI jsonCfg/GLIM_scepter.json
```

Both cameras use this frontend. Scepter uses geometric CT odometry without an
IMU; it initializes from the first valid depth frame. Its profile and saved
controls are isolated in [jsonCfg/glim_scepter](../../../jsonCfg/glim_scepter/README.md).
Select one camera application per process; both examples use the same ports.

Use `RelWithDebInfo` for live SLAM; it retains debug symbols. The VSCode task
**GLIM: build optimized** configures and builds that mode. The **GLIM Scepter
(optimized)** launch uses it before starting GDB. CMake links GLIM's spdlog ahead
of the Scepter SDK, which exports a different bundled spdlog ABI. Preserve that
order when integrating both libraries into another build.

## Sensor and viewer connections

```text
_Orbbec (or another sensor) → _PointCloud → _GLIM → _WebGLIM → browser
                              _IMUbase ↗    ↓
                                    optional globalMapPCL
```

The viewer links directly to GLIM:

```json
{
  "class": "_WebGLIM",
  "_GLIM": "GLIM",
  "webRoot": "html/viewer/_GLIM",
  "host": "0.0.0.0",
  "port": 8080,
  "thread": {"FPS": 10},
  "matPointSize": 2
}
```

`_WebGLIM` is built when `WITH_SLAM`, `USE_GLIM` and `WITH_UNIVERSE` are enabled.
It shares the HTTP server and camera configuration with other viewers and owns
its submap protocol. It does not consume `globalMapPCL`, `vGeometry`, or the
point/line geometry streams. Add the GLIM module name to `_WSconsole.vBASE` and
enter that same module name in the viewer's command panel.

For another sensor, connect its `_PointCloud` and optional `_IMUbase` to GLIM and
select its `configPath`. Clouds use metres and increasing timestamps in
nanoseconds. When using an IMU, samples must use m/s² including gravity and
rad/s, calibrated IMU-to-cloud extrinsics, and the same capture clock as depth. The input interface gives one timestamp per complete cloud; it
does not deskew a scanning LiDAR. Keep the input cloud transform fixed in the
sensor frame. `_PointCloud` frames remain spans in a ring buffer: allocate at
least two maximum sensor frames so a completed frame survives while the next
one is written. The freshness check avoids copying the same frame on idle polls.

The Orbbec example uses native depth, a 0.3–6 m range, 5 cm preprocessing cells,
and a 25 cm GICP grid. Its IMU-to-depth transform is the connected Gemini 335's
factory calibration. Changing sensors or enabling RGB-aligned clouds requires
matching extrinsics and settings. Separate IMU callbacks retain the 200 Hz
samples; depth and IMU use device capture timestamps.

The Scepter example uses native depth with color/alignment disabled. Geometric
odometry needs no fabricated IMU samples. With no gravity measurement, its map
starts in the initial camera's optical frame (X right, Y down, Z forward). The
viewer up-vector matches that frame. Scepter's complete clouds carry monotonic
host publication timestamps. Both submapping and global mapping disable IMU
factors. The final active odometry window and mapper lookahead are flushed on
Stop, including a scan containing only one valid depth frame.

## Map and controls

Only completed submaps appear in the browser. Before the first submap completes,
the cloud is empty while the sensor marker, FoV and trajectory update. Submap
completion depends on keyframe selection, motion and the odometry smoother's
lag; it is not tied to video FPS. **Stop** flushes remaining frames and finishes
the map. With global mapping disabled, the viewer shows pose and trajectory
without a completed-submap cloud.

Each completed submap's local points arrive once per connection. The browser
retains them, adds later submaps, and applies separate pose corrections when
mapping optimization changes their placement. Reconnection replays the complete
current session. Slow clients catch up without losing submaps. There is no
`nMapPoints` cap on this accumulated scene; each individual submap is limited to
10 million points by the stream protocol.

- **Start** creates a new session and clears the previous map and pose. It is
  idempotent while tracking.
- **Stop** finalizes and optimizes mapping, retaining the map and last pose.
- **Reset** stops and clears the map, pose and session trajectory.
- **Disconnect viewer** stops the map connection; the separate command
  connection can still control SLAM. Disconnecting does not stop tracking.
- **Fit map**, **Reset view**, and **Follow sensor** control the viewing camera.
- **Show trajectory** displays up to 8192 recent segments sampled from fresh
  status poses, with breaks across stale telemetry. This browser history is
  cleared for a new session or command connection; it is not a saved or
  retrospectively optimized GLIM trajectory.

The left panel shows map-frame position in metres, quaternion XYZW, roll/pitch/
yaw in degrees (ZYX convention), state and counts. The gold sensor marker turns
gray when its pose is stale or tracking stops. Pose availability is not an
accuracy estimate. The navigation API keeps continuous local odometry; status
applies the latest global correction to align the marker with the map. The FoV,
range and forward axis are visual settings, independent of sensor calibration.

**Distance to eye** colors nearby points warm and distant points cool, with an
automatic metre legend following orbit and zoom. Coloring runs on the GPU and
uses the viewer camera, not the tracked sensor. **Submap** assigns a distinct
color to each completed map section. GLIM does not preserve camera RGB here.

The optional `globalMapPCL` output remains available to other modules. By default,
`bPublishLiveMap: false` updates that ring only after a completed-submap change
or finalization, subject to `tMapUpdateNs`; `nMapPoints` caps that output alone.
Set `bPublishLiveMap: true` to include the legacy bounded recent-frame preview
for those consumers. Neither setting changes `_WebGLIM`'s completed-submap
stream. `nLiveFrames` also bounds unfinished points retained for PLY export.

## Parameters and point-cloud export

Edit SLAM parameters while stopped. **Start** applies edited values before
starting the session. **Save parameters** validates and applies the values and
writes the module's `fConfig`: `jsonCfg/GLIM.controls.json` for Orbbec and
`jsonCfg/glim_scepter/GLIM.controls.json` for Scepter.
**Load saved** reloads that file while stopped. On process startup, an existing
`fConfig` overrides the selected profile and optional module `parameters` object.
Values applied without saving last only for the current process.

The panel exposes input filtering and supported odometry/submap/global mapping
controls. Distances and voxel sizes are metres; submap rotation thresholds are
radians. Backend-specific groups appear only for supported implementations.
Module selection, IMU calibration and unexposed settings stay in `configPath`.
Restart OpenKAI after changing those profile files; the editable controls apply
on the next Start without replacing GLIM's process-global configuration.

**Save point cloud** writes a binary little-endian XYZ/RGB PLY on the backend PC
using `_PCfile::savePLY`. It snapshots all completed submaps plus retained
unfinished frames at their current poses, including up to `nLiveFrames` of
recent history. This export is independent of the display ring's `nMapPoints`
limit. It contains GLIM's processed points, not all raw sensor samples. Stop first
to export the finalized, optimized map, or save during tracking for a snapshot.

Orbbec creates files under `data/glim`; Scepter uses `data/glim_scepter`.
The module's `exportPath` controls this directory. The
reply shows the absolute backend path and written point count; this is not a
browser download. A command may supply an explicit `.ply` path whose parent
already exists. The default export directory is created automatically.

## WSconsole commands

Send JSON terminated with `EOJ` on the command connection:

```json
{"module":"GLIM","cmd":"start","requestId":"1"}
{"module":"GLIM","cmd":"stop","requestId":"2"}
{"module":"GLIM","cmd":"reset","requestId":"3"}
{"module":"GLIM","cmd":"getStatus","requestId":"4"}
{"module":"GLIM","cmd":"getConfig","requestId":"5"}
{"module":"GLIM","cmd":"setConfig","config":{"preprocess":{"distanceFar":5.0}},"requestId":"6"}
{"module":"GLIM","cmd":"saveConfig","requestId":"7"}
{"module":"GLIM","cmd":"loadConfig","requestId":"8"}
{"module":"GLIM","cmd":"savePointCloud","requestId":"9"}
{"module":"GLIM","cmd":"savePointCloud","path":"/tmp/glim-map.ply","requestId":"10"}
```

`setConfig`, `saveConfig` and `loadConfig` require stopped tracking. `setConfig`
and optional `saveConfig.config` accept partial nested objects and reject unknown
fields, invalid ranges and incompatible minimum/target/neighbor point counts.
`saveConfig` without `config` saves current values. Configuration replies include
`config` and `configFile`; export replies include `path` and `points`.

Replies echo `cmd`, `module`, `requestId`, and include `bSuccess`, optional
`error`, and normally `status`. Status includes state, pose validity/freshness,
position/orientation/angles, frame/map/submap counts and `canSavePointCloud`.
`session` and `revision` are decimal strings matching the submap stream.
`imuSamples`, `maxIMUgapNs`, `frameIntervalMs`, `processingMs`, cumulative `workMs`,
`updates`, `inputPoints` and `registrationPoints` support
[performance measurement](../../../docs/GLIM.md#measuring-performance).
`poseTimestampNs`, `inputTimestampNs`, and `mapTimestampNs` are decimal strings
containing nanoseconds. Sensor pose/input timestamps use the capture clock;
map-output timestamps use backend monotonic time. Do not subtract values from these different clocks.

## Dedicated submap protocol

Connect a WebSocket to `/stream/glim`. The server first sends a JSON `hello`
with `protocol: "openkai.glim"`, `version: 2`, `stream: "glim"`, camera/style
settings, `maxSubmapPoints: 10000000` and `maxChunkPoints: 65536`.
Send `start` to begin. Acknowledge every subsequent text or binary data message
with `next` after applying it. `pause` suspends transmission; `start` resumes.
There is at most one unacknowledged data message. The hello needs no ACK.

Data messages are:

- `reset`: JSON `type`, `session`, `revision`. Clear cached geometry and partial
  chunks. Sent first on each connection and whenever the SLAM session changes,
  including resets to an empty map.
- `submap`: JSON `type`, `session`, `revision`, `id`, `timestampNs`, `pointCount`,
  `pose`. Allocate immutable local XYZ storage; chunks follow. `pose` is a
  column-major 4×4 transform from submap-local coordinates to the map frame.
- Binary chunks: the header below followed by `countPoints × 3` float32 local
  XYZ values. Chunks are ordered within a submap; display it when complete.
- `pose`: JSON `type`, `session`, `revision`, `id`, `pose`. Update the existing
  submap transform without uploading its points again.

JSON `session`, `revision`, `id` and `timestampNs` are decimal strings to preserve
64-bit values in JavaScript. All binary values are little-endian:

| Offset | Type | Field |
| ---: | --- | --- |
| 0 | uint32 | Magic `0x324d4c47` (bytes `GLM2`) |
| 4 | uint32 | Version `2` |
| 8 | uint32 | Kind `1` (point chunk) |
| 12 | uint32 | Header size `56` |
| 16 | uint64 | Session |
| 24 | uint64 | Submap ID |
| 32 | uint64 | Sensor timestamp in nanoseconds |
| 40 | uint32 | Total points in this submap |
| 44 | uint32 | First point offset |
| 48 | uint32 | Points in this chunk, at most 65536 |
| 52 | uint32 | Reserved `0` |
| 56 | float32[] | Interleaved local XYZ |

Revisions can coalesce because each server snapshot retains every completed
submap. Each connection maintains its own delivery cursor and pose state.
Clients add geometry only for unseen IDs, keep earlier submaps across revisions,
and remove them only on reset. The geometry viewer's `/stream/points` and
`/stream/lines` routes and binary layouts are not accepted by `_WebGLIM`.

## Validation

```sh
cmake -S test/slam -B /tmp/openkai-slam-tests
cmake --build /tmp/openkai-slam-tests -j4
ctest --test-dir /tmp/openkai-slam-tests --output-on-failure
python3 test/slam/browser.py build/OpenKAI
python3 test/slam/browser.py build/OpenKAI --hardware
python3 test/slam/browser.py build/OpenKAI --config jsonCfg/GLIM_scepter.json
python3 test/slam/browser.py build/OpenKAI --config jsonCfg/GLIM_scepter.json --hardware
```

Transport tests cover slow-client catch-up, chunking, pose-only corrections,
reconnection, session resets and route isolation with synthetic submaps.
The browser test needs Chrome/Chromium, local sockets and, in hardware mode,
camera access (USB or network). It uses temporary ports/configuration and the
actual UI/WSconsole. The native suite covers CPU odometry with IMU and CT
odometry without IMU, including complete final-frame mapping on Stop.
Artifacts: `/tmp/openkai-glim-viewer.png` and
`/tmp/openkai-glim-browser-backend.log`.
