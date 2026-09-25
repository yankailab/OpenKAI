# GLIM in OpenKAI

Build against the installed GLIM CMake package (the source checkout is not compiled into OpenKAI):

```bash
cmake -S . -B build-glim -DWITH_SLAM=ON -DUSE_GLIM=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build-glim -j4
```

Use `-DCMAKE_PREFIX_PATH=/your/install/prefix` for a nonstandard installation. `glim::glim` supplies GTSAM, gtsam_points, Eigen, OpenMP, spdlog and Boost dependencies. GLIM is only required when both `WITH_SLAM` and `USE_GLIM` are enabled. SLAM also builds the shared navigation, geometry and IMU base classes if their wider module groups are disabled.

Include `jsonCfg/_GLIM.json` using the application's `APP.vInclude`, or copy its three module definitions into your existing camera configuration. Connect the camera's `_PointCloud` and `_IMUbase` outputs to `slamPoints` and `slamIMU`. Alternatively, change the SLAM module's input names to your existing modules. The example supplies input buffers; it needs a sensor producer to feed them. Paths are relative to the process working directory.

`jsonCfg/glim_orbbec` contains CPU configuration templates adapted from GLIM 1.2.2, with its MIT license. You can also point `configPath` at a copy of `/home/kai/dev/glim/config`. In that copy, select the CPU odometry, sub-mapping and global-mapping JSON files in `config.json`; the upstream configuration selects GPU modules by default. Every `_GLIM` instance in one process must use the same configuration directory because GLIM configuration is global. Restart the process after editing those profile files. The supported web controls use per-instance constructor parameters and apply on the next session without replacing global configuration.

Before using a real sensor, set `T_lidar_imu` in `config_sensors.json` to its calibrated IMU-to-point-cloud transform. The `glim_orbbec` profile now contains the attached Gemini 335's factory IMU-to-native-depth transform: translation `[0.000246, 0.000065, -0.016948]` meters and identity rotation. This is device/frame-specific; RGB-aligned clouds require IMU-to-color calibration instead. Point coordinates must be in meters; acceleration in m/s² (including gravity); angular velocity in rad/s. Point-cloud and IMU timestamps must increase in nanoseconds and share one capture clock. `_Orbbec` uses device timestamps and separate 200 Hz IMU callbacks, avoiding sample loss and USB-arrival jitter from video framesets. Keep the input point cloud in a fixed sensor frame, and calibrate extrinsics against that frame. Do not feed the estimated pose back into its input `_PointCloud` transform.

The frame interface treats each cloud as a single exposure, with zero per-point time offsets. It suits depth-camera clouds; it does not deskew a scanning LiDAR. Each producer calls `frameStart()`, `add(..., sensorTimestampNs)` for its points, then `frameStop()`. Frames are index spans in the point cloud's ring buffer. Oversized frames retain the newest `nP` points, and a completed frame becomes unavailable once a subsequent write overwrites its span. Allocate at least two maximum-sized input frames to keep the preceding frame available while the next is built. IMU samples use `addGyro(value, timestampNs)` and `addAcc(value, timestampNs)`; pairing consumes those queues, so use one SLAM consumer per IMU buffer.

The Orbbec CPU odometry example requires an IMU. Run `build/OpenKAI jsonCfg/GLIM_scepter.json` for Scepter cameras without an IMU. Its isolated [glim_scepter profile](../jsonCfg/glim_scepter/README.md) selects CT odometry, disables `enable_imu` in both mapping configs, and uses identity sensor extrinsics. The map starts in the first camera's optical frame, without gravity alignment. No IMU module or synthetic inertial samples are needed. The shared frontend, WSconsole controls, submap stream and PLY export work with both profiles. The adapter checks the selected estimator's IMU requirement at startup.

Scepter and Orbbec SDKs can be enabled in the same build. GLIM's spdlog must precede the Scepter SDK in the link order because the SDK exports an incompatible bundled spdlog; CMake declares the explicit dependency to preserve that order. Camera/SLAM saved controls for Scepter live under `jsonCfg/glim_scepter`, and its PLY output defaults to `data/glim_scepter`. The Orbbec settings remain separate. Sensor rates and preprocessing parameters still need tuning for the scene and camera.

`_SLAMbase` owns input linking, timestamp deduplication, the worker, pose publication and the session lifecycle. `_GLIM` owns preprocessing, odometry, submaps and global mapping. Processing is sequential on the OpenKAI worker; if processing falls behind, it takes the newest completed cloud instead of building an unbounded frame queue. `getLastFrameIfNew()` checks freshness under the ring lock before copying, so idle polls do not recopy the preceding cloud. This input optimization is independent of the viewer. IMU-required odometry waits for a paired IMU sample later than the cloud timestamp.

- `start()` starts the worker and, by default, a session. Set `bAutoStart: false` to start tracking explicitly.
- `startTracking()` creates a new session, or returns success if one is already active. Initialization may need several seconds of IMU and point-cloud data.
- `stopTracking()` finishes mapping and retains the last pose and map. Confidence becomes zero.
- `saveMap(path)` saves the finished GLIM map after `stopTracking()`. The saved map includes global optimization; the navigation pose is continuous local odometry in GLIM's world frame, expressed as `T_world_lidar` for the input cloud frame.
- `reset()` stops tracking, discards the map, and resets position and orientation. Call `startTracking()` to begin again.

`_NavBase::setConfidence()` uses a 0–100 scale and optionally expires stale updates. SLAM defaults to a one-second `tConfidenceTimeoutNs`; zero disables expiry. GLIM supplies no scalar tracking-quality score, so this adapter reports 100 for an available finite pose and 0 for initialization, insufficient points, stopped tracking or expired updates. It is a pose-availability signal, not an accuracy estimate. `bTracking()` reports whether the session is active, including initialization.

Run the hardware-independent tests with:

```bash
cmake -S test/slam -B /tmp/openkai-slam-tests
cmake --build /tmp/openkai-slam-tests -j4
ctest --test-dir /tmp/openkai-slam-tests --output-on-failure
```

The IMU-free test additionally checks immediate CT initialization, stationary and moving depth-only scenes, parameter persistence, and exact final-frame coverage in completed submaps, including a single-frame scan. The adapter retains CT's active smoother window separately from export previews and flushes the native mapper's lookahead when IMU is disabled.

These exercise ring-buffer frame spans and wraparound, IMU pairing, confidence expiry, real GLIM CPU estimation on stationary and moving synthetic scenes, map saving, parameter validation/persistence, PLY writing, dedicated submap transport, and session restart/reset. The motion test translates 0.5 m and rotates 23 degrees before returning, with independently synthesized 200 Hz IMU measurements. Live sensor calibration and trajectory accuracy still require a hardware run.

## Viewer and completed submaps

`jsonCfg/GLIM_orbbec.json` and `jsonCfg/GLIM_scepter.json` connect camera → input `_PointCloud` → `_GLIM` →
`_WebGLIM`. The viewer links directly with `"_GLIM": "GLIM"` and uses the shared
`HttpServer` with its own `/stream/glim` protocol. `_WSconsole` carries controls,
parameters and pose/status independently. `_WebGLIM` requires `WITH_UNIVERSE`
(the existing web support), `WITH_SLAM`, and `USE_GLIM`.

The backend caches each completed submap's immutable local points once, with a
stable session-local ID and current map pose. The browser accumulates those
submaps and uploads each submap's geometry once. Graph optimization sends pose
corrections, without regenerating or retransmitting unchanged point arrays.
There is no point cloud before the first completed submap; pose, FoV and the
browser's recent trajectory update while tracking. Keyframe selection and
smoother lag determine when submaps finish. Stop flushes remaining mapping work.
With `bMapping: false`, the dedicated viewer displays pose and trajectory only.

The stream sends a versioned JSON hello, then flow-controlled reset/submap/pose
metadata and bounded binary XYZ chunks. Every data message requires a `next`
credit; each client has its own cursor. Reconnection replays all completed
submaps, slow clients catch up, and a new session resets both complete and partial
geometry. Revisions may update poses without changing IDs or point arrays.
The dedicated layout and routes are separate from geometry point/line streams;
see the [wire contract](../html/viewer/_GLIM/README.md#dedicated-submap-protocol).

`globalMapPCL` remains an optional `_PointCloud` output for other modules. With
`bPublishLiveMap: false` (default), only completed-submap changes and finalization
refresh this output, subject to `tMapUpdateNs`. `nMapPoints` caps this ring output
and does not limit the dedicated viewer's accumulated submaps. Set
`bPublishLiveMap: true` to retain the prior continuous recent-frame preview for
other consumers. `nLiveFrames` bounds retained unfinished history; `_WebGLIM`
never streams those individual frames. Keep the optional map ring at least
`nMapPoints` large and its transform at identity.

The browser trajectory is a bounded history of fresh status poses, not GLIM's
optimized historical trajectory. Completed-submap pose corrections update the
map and latest sensor marker; they do not retrospectively solve the browser's
old trajectory samples. See [the viewer guide](../html/viewer/_GLIM/README.md)
for controls, distance coloring, sensor setup and launch instructions.

## Parameters and PLY export

The web parameter panel edits a validated JSON object exposed by `getConfig`.
Common fields are `bMapping`, `nMinPoints`, and `preprocess` fields
`distanceNear`, `distanceFar`, `voxelResolution`, `targetPoints`, `kNeighbors`,
and `threads`. Recognized CPU and CT odometry add `odometry` controls; standard
submapping/global mapping add their corresponding groups. Other selected
backends still load through GLIM's module factory. Unexposed settings and sensor
calibration continue to come from the selected profile.

Configuration changes require stopped tracking. `setConfig` merges and validates
partial values in memory; `saveConfig` also writes the configured `fConfig`;
`loadConfig` reads and validates that file. Startup loads profile defaults, then
an optional module `parameters` object, then an existing `fConfig`. The example
persists to `jsonCfg/GLIM.controls.json`. Values take effect on the next Start;
Start applies unsaved UI edits first. Unknown fields, wrong types, invalid ranges
and inconsistent point-count limits are rejected.

Commands use the usual `_WSconsole` envelope and trailing `EOJ`:

```json
{"module":"GLIM","cmd":"getConfig","requestId":"1"}
{"module":"GLIM","cmd":"setConfig","config":{"preprocess":{"distanceFar":5.0}},"requestId":"2"}
{"module":"GLIM","cmd":"saveConfig","requestId":"3"}
{"module":"GLIM","cmd":"loadConfig","requestId":"4"}
{"module":"GLIM","cmd":"savePointCloud","requestId":"5"}
```

Replies echo the command/module/request ID, report `bSuccess` and optional
`error`, and normally include `status`. Parameter replies add `config` and
`configFile`; point-cloud saves add `path` and `points`. `session` and `revision`
in status are decimal strings shared with the submap protocol.

`savePointCloud` uses `_PCfile::savePLY` to write binary little-endian XYZ float32
and RGB uint8. It snapshots all completed submaps at their latest optimized
poses plus retained unfinished frames, up to `nLiveFrames` of recent history.
It bypasses `nMapPoints` and the optional display ring; it still exports GLIM's
processed map points, not every raw depth sample. Snapshot collection holds the
estimator lock and file I/O occurs after releasing it. Saving is allowed during
tracking; stop first when a finalized, optimized export is desired.

Without a `path`, the command creates `exportPath` (default `data/glim`) and a
unique `map-<timestamp>.ply`. With an explicit `.ply` path, its parent must already
exist. The reply names the file on the backend PC; the button does not download
a file in the browser. This PLY export is distinct from `saveMap(path)`, which
saves GLIM's native finished map after tracking stops.

## Measuring performance

Use the **GLIM: build optimized** VSCode task for live SLAM. It configures
`build` as `RelWithDebInfo`, preserves the cached component switches, and builds
`build/OpenKAI`. The **GLIM Scepter (optimized)** launch runs this task before
starting GDB with `jsonCfg/GLIM_scepter.json`; enable the SDK first as described
in the Scepter profile guide. The original Orbbec launch remains available. Debug (`-O0`) retains expensive Eigen
and container operations in the point-cloud hot path; changing the thread's
target FPS does not remove that work.

The dedicated viewer transfers immutable submaps when they complete and small
pose updates when needed. The optional map-ring publication reuses conversion
buffers, caches affine coefficients per source cloud, and replaces the ring
without clearing its unused capacity. Identity output poses skip a second
transform. Input freshness remains checked under the ring lock before copying.
`_PointCloud` still stores frames as ring indices; replacement and wraparound
semantics are covered by the SLAM integration test.

Measure the real camera with an active point-cloud stream:

```sh
python3 test/slam/profile.py build/OpenKAI --warmup 10 --seconds 30 --output /tmp/glim-profile.json
```

The profiler uses temporary ports/configuration, starts and stops its own backend,
and writes raw status samples, per-stage results, and a sibling `.log` file. Keep
the camera still during initialization. Compare runs with the same sensor,
scene, mapping settings and compiler settings. Run long enough to produce
completed submaps before comparing sustained mapping and stream throughput;
network traffic is event-driven rather than one point cloud per video frame.

`getStatus` exposes cumulative `workMs` for IMU ingestion, input copying/conversion,
preprocessing, odometry, mapping and map publication. Subtract two readings and
divide by the change in `frames` for amortized milliseconds per processed frame.
These totals include polls that did not produce a pose; they exclude worker sleep,
command-lock waiting and asynchronous HTTP/WebSocket serialization. `publishMap`
measures the optional ring output, not the dedicated stream. `inputPoints`,
`registrationPoints` and `updates` show
the workload. `processingMs` is the last frame's work time, so periodic samples
of it are not a per-frame timing distribution.

With the dedicated submap stream, a stationary Gemini 335 run measured **29.98
SLAM frames/s** over 20 seconds after 8 seconds of warmup. There was no repeated
cloud transfer between submaps. Stop finalized one 4,345-point submap, delivered
in one 52,196-byte binary chunk with no stream errors. This stationary run checks
throughput and finalization; the synthetic motion and transport tests exercise
multiple submaps, corrections, slow clients and resets during a transfer.

Historical measurements of the preceding `_WebGeometryBase` display pipeline
on the attached Gemini 335 and Intel Core i9-14900KF, with 10 seconds of warmup
followed by 30 seconds of sampling and an active geometry stream:

| Build / implementation | SLAM frames/s | Input ms/frame | Map publication ms/frame |
| --- | ---: | ---: | ---: |
| Original, Debug | 1.49 | 52.04 | 498.16 |
| Optimized hot paths, Debug | 5.78 | 8.65 | 64.92 |
| Optimized hot paths, RelWithDebInfo | 29.94 | 1.54 | 0.38 |

These results predate the dedicated completed-submap stream and are retained
to document the original bottleneck, not as a benchmark of the current transport.
They are stage totals divided by processed frames, including the configured
map-publication interval. Sensor/registration/map limits were unchanged; the
runs processed approximately 176–198k valid input points, 5.8–6k registration
points, and 338–360k displayed map points. The final run had no stream errors
and a maximum IMU gap of 4.943 ms. This measures throughput in the camera's
current scene; motion accuracy is a separate measurement. The subsequent
hardware browser test also passed Start/Stop/Reset, retained submaps, map
streaming, pose/FoV rendering and reconnect at 30.0 SLAM frames/s.

# Installing the dependencies
```bash
sudo apt update
sudo apt install --no-install-recommends \
    libomp-dev libmetis-dev \
    libfmt-dev libspdlog-dev \
    libboost-serialization-dev \
    libboost-filesystem-dev \
    libboost-thread-dev \
    libboost-program-options-dev \
    libboost-date-time-dev \
    libboost-timer-dev \
    libboost-chrono-dev \
    libboost-regex-dev \
    libboost-random-dev \
    libboost-graph-dev

unset ROS_VERSION
```

```bash
git clone --depth 1 --branch 4.3a1 https://github.com/borglab/gtsam.git
```

Edit
```bash
nano gtsam/gtsam/geometry/SL4.cpp
```
add near the top

```Cpp
#include <cassert>
```

Edit
```bash
nano gtsam/gtsam/slam/FrobeniusFactor.h
```

Around lines 200–225, replace the evaluateError() implementation with this:
```Cpp
Vector evaluateError(const T& T1, const T& T2, OptionalMatrixType H1,
                     OptionalMatrixType H2) const override {
  const bool computeJacobians = H1 || H2;

  // predict T2*T1
  typename T::Jacobian H_T21_T2;
  const T hatT21 =
      traits<T>::Between(T2, T1,
                         computeJacobians ? &H_T21_T2 : nullptr);

  // Calculate \hat T21 * T12_, which is predicted to be I_NxN
  typename T::Jacobian H_pred_hat = T::Jacobian::Zero();
  const T pred =
      traits<T>::Compose(hatT21, T12_,
                         computeJacobians ? &H_pred_hat : nullptr);

  // Move to constructor
  const MatrixN I = MatrixN::Identity();
  const VectorD vecI = Eigen::Map<const VectorD>(I.data());

  // Calculate error
  Eigen::Matrix<double, Dim, T::dimension> H_vec_pred;
  Vector error =
      traits<T>::Vec(pred, computeJacobians ? &H_vec_pred : nullptr) - vecI;

  // Do chain rule only when Jacobians were actually requested
  if (computeJacobians) {
    const auto H_error_hat21 = H_vec_pred * H_pred_hat;
    if (H1) *H1 = H_error_hat21;
    if (H2) *H2 = H_error_hat21 * H_T21_T2;
  }

  return error;
}
```

Build
```bash
cd gtsam
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DGTSAM_BUILD_TESTS=OFF \
      -DGTSAM_BUILD_EXAMPLES_ALWAYS=OFF \
      -DGTSAM_BUILD_TIMING_ALWAYS=OFF \
      -DGTSAM_BUILD_UNSTABLE=ON \
      -DGTSAM_BUILD_PYTHON=OFF \
      -DGTSAM_INSTALL_MATLAB_TOOLBOX=OFF \
      -DGTSAM_WITH_TBB=OFF \
      -DGTSAM_USE_SYSTEM_EIGEN=ON \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
      -DGTSAM_BUILD_WITH_MARCH_NATIVE=OFF ../

make -j$(nproc)
sudo make install
sudo ldconfig
```

gtsam_points
```bash
git clone --depth 1 --branch v1.2.2 https://github.com/koide3/gtsam_points.git
cd gtsam_points
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
      -DBUILD_WITH_CUDA=OFF \
      -DBUILD_WITH_TBB=OFF \
      -DBUILD_WITH_OPENMP=ON \
      -DBUILD_WITH_MARCH_NATIVE=OFF \
      -DBUILD_DEMO=OFF \
      -DBUILD_EXAMPLE=OFF \
      -DBUILD_TOOLS=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_TESTS_PCL=OFF ../

make -j$(nproc)
sudo make install
sudo ldconfig
```


GLIM
```bash
git clone --depth 1 --branch v1.2.2 https://github.com/koide3/glim.git
cd glim
mkdir build
cd build

unset ROS_VERSION
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
      -DBUILD_WITH_CUDA=OFF \
      -DBUILD_WITH_VIEWER=OFF \
      -DBUILD_WITH_OPENCV=OFF \
      -DBUILD_WITH_MARCH_NATIVE=OFF ../

make -j$(nproc)
sudo make install
sudo ldconfig
```
