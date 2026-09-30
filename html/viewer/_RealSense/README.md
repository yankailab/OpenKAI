# OpenKAI RealSense viewer

This integration is built and tested with RealSense SDK **2.58.4**. From the repository root,
run `build/OpenKAI jsonCfg/RealSense.json`, then open
`http://localhost:8080/`. You can also open this directory's `index.html`, enter
the backend host/ports and click **Start**. The geometry service serves the page
and vendored three.js assets; no npm or external web server is needed.

The layout, rendering and connection structure match the Orbbec and Scepter
viewers. Point clouds use `_WebGeometry`'s version-6 `/stream/points` and
`/stream/lines` connections. Camera controls use the separate `_WSconsole`
connection on port **7890**. The camera module defaults to **RealSense**.

## Camera controls

**Refresh config** loads capture settings and the connected camera's supported
RealSense SDK options, including their descriptions, ranges, available choices
and current values. SDK options are grouped by depth, color and motion sensor,
plus each supported processing filter. An option appears once per sensor or
filter: controls with the same SDK name on different sensors remain independent.
Read-only device values are displayed but cannot be edited. Capture configuration
uses the canonical keys `SN`, `devFPS` and `devFPSd`; SDK options appear only in
their sensor or filter section.

A field change immediately sends a `setConfig` patch. Device failures appear
above the settings, and the backend's accepted configuration replaces the edited
value. Every accepted change restarts camera capture, including SDK option
changes. Settings that can only be written before capture starts remain editable;
the backend records their writability before starting the pipeline.
Accelerometer and gyroscope rates of **0** let the SDK choose a supported rate;
the D455 configuration explicitly uses **250 Hz** acceleration and **200 Hz**
gyroscope capture. IMU samples go to the `IMUstream` named by `IMUstreamOut` for
consumers such as GLIM, independently of the browser preview.

A blank optional field or **Preserve camera setting** stores `null`, which tells
the backend to make no explicit SDK write for that option when opening capture.
Device readbacks are shown separately and never become overrides automatically.
Options can depend on other options: for example, disable automatic exposure
before setting manual exposure. The D455 exposes many switches, including auto
exposure, as SDK `float` options: their values are numeric **0** or **1**, not JSON
booleans. Controls follow each option's reported SDK type. After changing a
preset, refresh config to retrieve any other values changed by the device.

**Save config** writes the current settings to the original launch JSON file.
The browser cannot choose a destination path. Module links and unrelated modules
are preserved. Unset options are saved as `null`, including entries for read-only
options. Live temperature, baseline and other read-only values are never persisted.

For SLAM, use [GLIM_realSense.json](../../../jsonCfg/GLIM_realSense.json) and
its [sensor configuration](../../../jsonCfg/glim_realSense/config_sensors.json).
The launch file keeps depth and IMU timestamps on the shared hardware clock.
The [SDK motion documentation](https://github.com/realsenseai/librealsense/blob/master/doc/d435i.md)
explains the depth optical axes used by the IMU: right, down and forward. The
SLAM sensor configuration supplies the calibrated depth/IMU translation for the connected
D455 while retaining those shared axes.

## IMU preview

The left panel follows the Orbbec viewer layout: rotating XYZ axes, roll/pitch/yaw
and six ten-second plots for gyroscope (rad/s) and acceleration (m/s²). Connect
the command connection, then click **Start IMU**. The preview uses the camera
module selected under Camera controls and polls at up to 10 Hz. **Stop IMU** stops
only the preview; it does not change embedded IMU capture or `IMUstreamOut`.
Disconnecting or changing the camera module stops and clears the preview.

The backend estimates relative orientation from the embedded accelerometer and
gyroscope. Yaw may drift without a heading reference. Raw values and orientation
are cleared while IMU capture is disabled, the camera is closed or no recent
samples are available. Changing camera settings can briefly interrupt the preview
while capture restarts; polling resumes automatically.

## JSON messages

Requests are JSON objects followed by `EOJ`, using the existing WSconsole framing.
Replies may span multiple WebSocket text messages. Both requests and replies
carry `module` and `requestId` for routing and correlation.

```json
{"module":"RealSense","cmd":"getConfig","requestId":"camera-1"}
{"module":"RealSense","cmd":"setConfig","requestId":"camera-2","config":{"sensorOptions":{"depth":{"RS2_OPTION_EXPOSURE":8500}}}}
{"module":"RealSense","cmd":"setConfig","requestId":"camera-3","config":{"bIMU":true}}
{"module":"RealSense","cmd":"saveConfig","requestId":"camera-4"}
{"module":"RealSense","cmd":"getIMU","requestId":"imu-1"}
```

Configuration replies contain `bSuccess`, `config` and `deviceOpen`; parameter metadata
is supplied in `schema`. Rejections contain `error` or per-field `errors`.
SDK overrides are stored under `config.sensorOptions[domain][RS2_OPTION_NAME]`.
Each SDK schema entry has `domain` and `key`; capture settings have only `key`.
Types are `int`, `float`, `bool`, `string`, `size`, `range`, `rect` or `object`.
Rectangle options use `[x1, y1, x2, y2]` with integer pixel coordinates from 0 to 32767.
`choices`, when present, contains `{ "value": 0, "label": "Off" }` entries.
`getIMU` replies contain `bSuccess`, `enabled`, `deviceOpen`, `available`, `gyro`
and `acc` (XYZ arrays), plus `orientationValid`, `quaternion` (WXYZ) and `rpy`
(roll/pitch/yaw in radians). `tGyro`, `tAcc` and `tFusion` are decimal strings of
capture timestamps in nanoseconds, preserving precision in browsers. Treat
`available` and `orientationValid` as validity flags; unavailable values must not
be displayed as current samples. Polling does not consume the IMU stream.
Only this command protocol and geometry version 6 are supported.

## Validation

Run `python3 html/viewer/_RealSense/tests/browser-smoke.py` from the repository
root. The test uses Chrome/Chromium and Python's standard library to check the
actual page, fragmented replies, domain-specific patches, option types,
read-only controls, request correlation and IMU preview lifecycle without a connected camera.
To also check a captured `getConfig` reply, pass
`--response /path/to/response.json`; the test only replays JSON and does not open
the camera. Live capture and device writes require RealSense hardware.

With the viewer running and embedded IMU capture enabled on a connected camera, use
`python3 html/viewer/_RealSense/tests/browser-smoke.py --live-url http://localhost:8080/`
to test the actual command socket: Connect, parameter replies, rendered controls,
Refresh config, IMU samples, disconnect and reconnect. This mode only reads
configuration and IMU snapshots; it does not change or save camera settings.
For a different command port, append
`?cmdPort=7891` to the URL. Close other command connections before running it,
since the launch configuration allows one command client.

For the connected D455, run `python3 test/RealSense/hardware_smoke.py`. It tests
live stream combinations, SDK control readback, point-cloud transport, IMU preview
validity across capture restarts, rejected changes, saving/reloading and shutdown. It uses a temporary launch configuration
and restores the controls changed by the test. SDK option validation, preview
fusion, IMU interpolation and console shutdown also have standalone CMake/CTest projects in
`test/RealSense`, `test/SLAM` and `test/Protocol`.
