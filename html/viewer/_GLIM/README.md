# GLIM SLAM viewer

Sensor-independent GLIM viewer derived from `_Orbbec`. It uses the existing
`_WebGeometryBase` binary point/line streams and `_WSconsole` command connection.
All Three.js assets are local; no npm build or internet connection is required.

From the OpenKAI repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DWITH_SLAM=ON -DUSE_GLIM=ON -DWITH_VISION=ON -DUSE_OPENCV=ON \
  -DUSE_ORBBEC=ON -DWITH_UNIVERSE=ON -DWITH_PROTOCOL=ON \
  -DWITH_IO=ON -DWITH_SENSOR=ON -DUSE_WSSERVER=ON -DUSE_SCEPTER_SDK=OFF
cmake --build build -j4
./build/OpenKAI jsonCfg/GLIM_orbbec.json
```

Open `http://127.0.0.1:8080`, click **Connect viewer**, then **Start** in the SLAM
panel. Opening `index.html` directly also provides a launcher to the backend's
HTTP server. Stream and command ports default to 8080 and 7890 respectively.
`GLIM_orbbec.json` starts with tracking stopped and disables the desktop windows.
It requires a build with GLIM, Orbbec, OpenCV, Universe, Protocol and WS server
support. Keep the camera stationary during IMU initialization.

Use the optimized `RelWithDebInfo` build for live SLAM; it retains debug symbols.
The unoptimized Debug build can drop most camera frames while converting and
publishing point clouds. The Orbbec profile uses native depth (without RGB
alignment), a 0.3–6 m input range, 5 cm preprocessing cells and a 25 cm GICP
search grid. Its IMU-to-depth transform comes from the connected Gemini 335's
factory calibration. Recalibrate the transform and retune the range/resolutions
when changing sensors or enabling RGB-aligned clouds. IMU callbacks run separately
from video framesets so all 200 Hz samples reach SLAM. Both use device capture
timestamps; host USB arrival times are unsuitable for IMU integration.

The current `build/OpenKAI` configuration has Scepter disabled. Keep
`USE_SCEPTER_SDK=OFF` when using GLIM with this installed SDK: its exported
`spdlog` symbols conflict with GLIM's logger.

Pipeline:

```
_Orbbec (or another sensor) → obPCL → GLIM → glimPCL → _WebGeometryBase
                               ↑
                             obIMU
```

To use another sensor, connect its point cloud and optional IMU to `_GLIM`, select
its GLIM configuration, and set `globalMapPCL` to a separate `_PointCloud` output.
Add the GLIM module name to `_WSconsole.vBASE` and enter that name in the viewer.
Use `bFrame: true` and `dTexpire: 0` on `_WebGeometryBase`, with its `vGeometry`
point source set to the map output. `_PointCloud` frames use ring-buffer indices;
size the input ring to hold at least two maximum-sized sensor frames, and size the
map ring to at least `nMapPoints`. Oversized frames retain only the newest `nP`
points; completed frames expire when later writes overwrite their slots.
Keep input and map buffers at identity
transforms. SLAM expects metres, m/s², rad/s, calibrated sensor extrinsics and
synchronized microsecond timestamps. The existing input interface supplies one
timestamp per whole cloud; it does not deskew a scanning LiDAR.

- **Start** starts a new session (idempotent while active). A new session clears
  the previous map and pose.
- **Stop** finalizes and optimizes mapping, retaining the completed map and pose.
- **Reset** stops and clears both map and pose; click Start to track again.
- **Disconnect viewer** stops geometry streaming. The separate command connection
  can still control SLAM. Disconnecting either connection does not stop SLAM.
- **Fit map**, **Reset view**, and **Follow sensor** control the viewing camera.
  The sensor marker and FoV box show the input sensor pose in the global map.
  Set the forward axis, horizontal/vertical FoV and range for your sensor. These
  are visual guides, not calibration parameters or automatically read intrinsics.

Point colors default to **Distance to eye**: warm colors are nearby, cool colors
are farther from the viewer's camera. The meter legend automatically adapts to
the cloud and follows orbit/zoom, including while SLAM is stopped. Coloring runs
on the GPU; a small sample of the cloud determines the range. This is distance
from your viewing position, not the tracked sensor. Select **Map stage** to see
the original blue completed submaps and green recent frames instead.

Completed submaps use their optimized poses. Recent odometry frames use the same map frame,
so the display updates before the first submap completes. Recent history is
bounded by `nLiveFrames` (default 100); it is replaced by completed submaps. The
live poses follow every smoother update instead of waiting for marginalization.
The published display cloud is uniformly sampled to `nMapPoints` (default 400000)
and refreshed at most every `tMapUpdateUs` (default 200000). GLIM's internal map
and saved map are unaffected by this display limit. This adapter does not retain
RGB in GLIM's map; the Map stage option indicates completed/recent geometry. With `bMapping:
false`, only the bounded recent odometry frames are available.

The left panel shows map-frame position (metres), orientation (quaternion XYZW),
roll/pitch/yaw (degrees, ZYX convention), session state, and map/frame counts.
The gold marker turns gray when the pose is stale or tracking is stopped. Fresh pose
availability is not a tracking accuracy estimate. The navigation API retains
continuous odometry; the viewer applies the latest global submap correction to
align its sensor marker with loop-closure updates.

Commands are ordinary `_WSconsole` JSON, terminated with `EOJ`:

```json
{"module":"GLIM","cmd":"start","requestId":"example-1"}
{"module":"GLIM","cmd":"stop","requestId":"example-2"}
{"module":"GLIM","cmd":"reset","requestId":"example-3"}
{"module":"GLIM","cmd":"getStatus","requestId":"example-4"}
```

Replies echo `cmd`, `module`, `requestId`, and include `bSuccess`, optional
`error`, and `status`. Status contains `state`, `tracking`, `poseValid`,
`poseFresh`, `position`, `orientation`, `angles`, `frames`, `mapPoints`,
`submaps`, `liveFrames`, `mapOutput`, `mapping`, and timestamps in microseconds.
`imuSamples` counts consumed IMU pairs, `maxIMUgapUs` reports the largest gap,
`frameIntervalMs` reports the latest input-frame interval, and `processingMs`
reports the last processed frame's work (including any map update). These help
distinguish a fresh network stream from a SLAM worker that is dropping frames.
Pose/input timestamps use the sensor clock; map timestamps use backend monotonic
time. Do not subtract timestamps from these two clocks. The page polls status
with one outstanding request, handles fragmented JSON replies, and marks stale
telemetry when replies stop. Points always travel through `_WebGeometryBase`.

Validation:

```sh
cmake -S test/slam -B /tmp/openkai-slam-tests
cmake --build /tmp/openkai-slam-tests -j4
ctest --test-dir /tmp/openkai-slam-tests --output-on-failure
python3 test/slam/browser.py build/OpenKAI
python3 test/slam/browser.py build/OpenKAI --hardware
```

The browser test needs Chrome/Chromium, local socket access, and (in hardware
mode) USB access. It uses temporary ports and configuration, runs the real UI and
WSconsole, and verifies map retention/reset. Hardware mode additionally requires
live poses and streamed map points. Artifacts: `/tmp/openkai-glim-viewer.png` and
`/tmp/openkai-glim-browser-backend.log`.
