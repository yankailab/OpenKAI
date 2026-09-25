# OpenKAI Orbbec viewer

From the repository root, run `build/OpenKAI jsonCfg/Orbbec.json`, then open
`http://localhost:8080/`. You can also open this directory's `index.html`, enter
the backend host/ports and click **Start**. The geometry service serves the page
and the vendored three.js assets; no npm or external web server is needed.

The center is copied from `_GeometryBase` and uses `_WebGeometryBase`'s existing
version-5 `/stream/points` and `/stream/lines` connections. Camera controls and
IMU preview use a separate `_WSconsole` connection on port **7890**. Module names
default to **Orbbec** and **obIMU**, matching `jsonCfg/Orbbec.json`.

## Camera controls

**Load config** retrieves the current backend parameters, readable device values,
and supported ranges. The collapsible sections cover stream settings and every
control in `OrbbecCtrl`. Unsupported scalar controls are marked as unavailable.
Structured controls accept complete JSON objects using the SDK field names;
their placeholders show examples, which must be adjusted for your camera.

A field change sends a `setConfig` patch immediately. Device failures are shown
and the field returns to the backend's accepted value. Stream dimensions, frame
rates, device selection and capture switches restart the pipeline. A failed
restart restores the previous configuration and attempts to reopen it. Changes
to individual SDK properties call their setters without restarting capture;
some model-specific properties can still be rejected while streaming.

Blank numeric/string/object controls or **Preserve camera setting** remove that
override without resetting the camera. Loading device values never turns them
into overrides. Presets may reset other device properties; use **Load config**
to refresh the device readbacks after applying a preset.

Only **Save config** writes to `Orbbec.fConfig`, configured as
`jsonCfg/Orbbec.controls.json`. This separate file contains non-null values that
differ from the application JSON's startup settings. Unchanged/default values
are omitted. It is loaded over those startup settings at initialization. The
application JSON, module links and thread settings are not overwritten. Removing
an override omits it from the saved file, so the next launch uses the startup
configuration for that setting. A save failure is returned to the browser.

## IMU preview

**Start IMU** subscribes to `_IMUbase`'s slower `threadStream` (30 Hz in the sample
configuration); **Stop IMU** unsubscribes. Camera IMU acquisition and the fast
fusion thread continue independently. Enable `Orbbec.bIMU`, link `_IMUbase`, and
set `obIMU.bFusion: true` to get orientation. Fusion runs at full speed without
an FPS sleep. Preview pacing comes from `obIMU.threadStream.FPS` (30 in the sample
configuration), read during initialization.

Six charts show the latest gyro XYZ and acceleration XYZ over ten seconds. The
coordinate arrows rotate using the fused quaternion. Gyro values use rad/s and
acceleration uses m/s², following the [Orbbec SDK IMU units](https://orbbec.github.io/OrbbecSDK/doc/tutorial/English/OrbbecSDK_C%2B%2B_API_user_guide-v1.0.pdf).
Mahony fusion uses paired sensor timestamps in nanoseconds, converted to seconds. The SDK capture timestamps are converted from microseconds when samples enter OpenKAI.
Euler angles are roll/pitch/yaw in radians on the wire and degrees on screen.
Orientation is relative; no magnetometer/absolute yaw reference is available.

## JSON messages

Requests are JSON objects followed by `EOJ`, using the existing WSconsole framing.
Replies are JSON objects, possibly split across WebSocket text messages. Requests
and replies carry `module` and optional `requestId` for routing/correlation.

```json
{"module":"Orbbec","cmd":"loadConfig","requestId":"camera-1"}
{"module":"Orbbec","cmd":"setConfig","config":{"OB_PROP_COLOR_EXPOSURE_INT":100}}
{"module":"Orbbec","cmd":"saveConfig"}
{"module":"obIMU","cmd":"startStream"}
{"module":"obIMU","cmd":"stopStream"}
```

Camera replies include `bSuccess`, `config` and `deviceOpen`; load also includes
`schema`. Rejections include `error` or per-field `errors`. `loadConfig` retrieves
live parameters; the C++ `loadConfig()` method reads the saved override file.
No browser request can select a save-file path.

IMU samples use `cmd: "imuData"`, `gyro: [x,y,z]`, `acc: [x,y,z]`,
`quaternion: [w,x,y,z]`, `rpy: [roll,pitch,yaw]`, `tGyro`, `tAcc`, `tFusion`,
`fusion` and `orientationValid`. Raw preview samples are retained separately from
fusion queues. Fusion publishes orientation through a nonblocking snapshot;
preview JSON construction and sending happen after releasing the snapshot locks.
Eigen's canonical ZYX decomposition supplies roll/pitch/yaw. Invalid input samples
are discarded; timestamp resets and long fusion gaps restart integration. Stop
replies are serialized after prior samples.

## Checks

```sh
cmake --build build -j 4
python3 test/orbbec/native.py build
python3 test/orbbec/browser.py build/OpenKAI
```

The native test needs the installed SDK but no camera. It checks command
validation, sparse save/reload, IMU pairing, fusion timing and stream stop. It also
checks preview FPS, negative-yaw angles, and continued fusion while preview reading
and transport sending are deliberately blocked.
The browser test needs Chrome/Chromium and runs a temporary headless version of
`jsonCfg/Orbbec.json` with temporary ports and save files. With a camera, it checks
real point clouds, fused IMU data, and live/save separation; without one, it checks
offline command rejection. Fragmented IMU JSON also exercises all six graphs and
orientation independently of hardware. A screenshot is written to
`/tmp/openkai-orbbec-viewer.png`.
