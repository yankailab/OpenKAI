# OpenKAI Livox1 viewer

Build with the sensor, geometry, and command-server modules enabled:

```sh
cmake -S . -B build -DBUILD_EXECUTABLE=ON -DWITH_SENSOR=ON \
  -DWITH_UNIVERSE=ON -DWITH_IO=ON -DWITH_PROTOCOL=ON -DUSE_WSSERVER=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build -j2
```

Use the repository's normal build dependencies, including wsServer for the
command connection. No Livox SDK or Open3D dependency is needed.

From the repository root, set `Lvx1.lvxIP` in `jsonCfg/Livox1.json` to your
Avia-generation LiDAR's IPv4 address, then run:

```sh
build/OpenKAI jsonCfg/Livox1.json
```

The example address `192.168.1.100` is a placeholder, not a discovered address.
Connect the OpenKAI computer to the sensor's Ethernet network. Open
`http://localhost:8080/` and click **Start** to connect the point cloud and command
channels. **Connect** in the right panel opens just the command channel.
The browser's **IP / host** is the computer running OpenKAI. For another computer,
open `http://<OpenKAI-PC-IP>:8080/`.

The viewer uses the existing OpenKAI layout, IMU preview, and geometry renderer.
All assets, including Three.js, are local; npm and an internet connection are not
needed. `_Livox1` implements the first-generation Livox Ethernet protocol
without an external Livox SDK. Use `_Livox2` and its existing viewer for MID-360
and other second-generation protocol devices.

## Network configuration

The sample runs `_WebGeometry` on TCP **8080** and `_WSconsole` on TCP **7890**.
`_Livox1` owns its UDP sockets, and the driver settings are on the `Lvx1` instance:

```json
{
  "Lvx1": {
    "class": "_Livox1",
    "thread": {"FPS": 0},
    "lvxIP": "192.168.1.100",
    "lvxPortBroadcast": 55000,
    "lvxPortCmd": 65000,
    "lvxHostPortCmd": 56000,
    "lvxHostPortPCL": 56001,
    "lvxHostPortIMU": 56002,
    "PCLframeOut": "livoxPoints",
    "IMUstreamOut": "livoxIMU",
    "bIMUpreview": true
  }
}
```

The driver selects the host Ethernet IPv4 address using the route to `lvxIP`.
Set `lvxHostIP` to a local address to override that choice; `0.0.0.0` selects it
automatically. UDP port 55000 receives device broadcasts; the sensor command port
is 65000. The handshake advertises the configured host command, point-cloud,
and IMU ports. Network values are read-only in the viewer; edit the launch file
and restart OpenKAI to change them. Allow this UDP traffic through any firewall.
Stop other applications controlling the same sensor before connecting.

To preview the page separately, serve the whole HTML tree:

```sh
python3 -m http.server 8000 --directory html
```

Open `http://localhost:8000/viewer/_Livox1/`, set **IP / host** to the OpenKAI
computer and **Stream port** to **8080**. Add `?cmdPort=7891` when using another
command port. Opening `index.html` directly also provides a launcher; **Start**
then navigates to the backend page.

## Device controls

The module defaults to **Lvx1**, matching the launch file. **Refresh config**
loads the driver's parameter schema and status. The schema determines which
controls are available for the connected model and firmware, including:

- Cartesian or spherical coordinates (`lvxCoordinate`).
- First, strongest, dual, or triple returns (`lvxReturnMode`), where supported.
- Non-repetitive or repetitive scanning (`lvxScanPattern`), where supported.
- High sensitivity (`lvxHighSensitivity`) on supported Avia and Tele-15 firmware.
- Normal, power saving, or standby power mode (`lvxWorkMode`).
- Point cloud sampling (`lvxSampling`), IMU capture (`lvxIMUdataEn`), and fan
  control (`lvxFan`), where supported.
- OpenKAI frame duration, frame size limit, distance range, point color,
  reflectivity, IMU preview, and stabilization.

Unsupported controls are disabled. Device identity, network configuration, and
status are read-only. Each edit sends one `setConfig` patch. The viewer displays
queued writes and waits for device acknowledgements before allowing another edit
or save. Rejected edits restore the last accepted configuration. **Save config**
writes the current settings to the launch JSON used to start OpenKAI.

Status refreshes every two seconds and more often while settings are pending.
The device panel includes firmware, current work state, the heartbeat status code,
and packet/frame counters.
Refresh pauses while a field has focus so unfinished input is preserved. Frame
duration is OpenKAI's point-cloud accumulation window, independent of the
sensor's scan pattern. Power saving, standby, and disabled sampling can stop
point-cloud delivery; select normal mode and enable sampling to resume it.

## IMU preview

Click **Start IMU** for orientation axes and six gyro/acceleration plots. The
preview reads a non-consuming snapshot at up to 10 Hz. **Stop IMU** only stops
browser polling; `IMUstreamOut` remains available to other modules. Gyroscope
units are rad/s and acceleration units are m/s². Orientation is relative; yaw
can drift without a heading reference. Disabled, disconnected, or stale samples
clear the display.

`bIMUpreview` enables backend preview fusion; the sample enables it. Disabling
preview removes that computation while preserving raw IMU capture. Point-cloud
stabilization (`bIMUstab`) requires preview fusion and is off in the sample.
Keep stabilization off when another module needs the raw cloud for SLAM.
Devices without an IMU do not provide preview samples.

## Existing streaming protocol

The viewer imports the same `html/common/js/geometryProtocol.js`,
`geometryConnection.js`, and `geometryViewer3D.js` as `_Livox2`. Point clouds
and lines retain the existing `_WebGeometry` version-7 binary format at
`/stream/points` and `/stream/lines`, including render-credit flow control.
No LiDAR-specific streaming format is introduced.

The sensor's UDP commands and samples follow Livox's
[legacy communication protocol](https://github.com/Livox-SDK/Livox-SDK/wiki/Livox-SDK-Communication-Protocol).

Configuration and IMU preview use the separate `_WSconsole` WebSocket. Requests
end with the literal `EOJ` delimiter; replies can span WebSocket messages.
`module`, `cmd`, and `requestId` correlate each response:

```json
{"module":"Lvx1","cmd":"getConfig","requestId":"lidar-1"}
{"module":"Lvx1","cmd":"setConfig","requestId":"lidar-2","config":{"lvxReturnMode":2}}
{"module":"Lvx1","cmd":"setConfig","requestId":"lidar-3","config":{"frameDurationMs":100}}
{"module":"Lvx1","cmd":"saveConfig","requestId":"lidar-4"}
{"module":"Lvx1","cmd":"getIMU","requestId":"imu-1"}
```

Config replies contain `bSuccess`, `config`, `schema`, `deviceOpen`, and
`status`. `pendingSettings` and `configApplied` distinguish accepted settings
from acknowledged writes; `pendingCommands` also counts other requests such as
heartbeats. `lastError` reports device failures. Schema metadata
supplies types, ranges, enum choices, and device support flags.

IMU replies expose validity flags, `gyro`, `acc`, a WXYZ `quaternion`, and `rpy`
in radians. `tGyro`, `tAcc`, and `tFusion` are nanoseconds encoded as decimal
strings to preserve precision in the browser.
The driver handles signed PPS offsets, including compensated IMU samples before
the pulse, on a shared timeline for points and IMU. PPS alone supplies relative
seconds; an absolute date requires UTC or PTP synchronization.

## Validation

```sh
python3 test/run_livox1_tests.py build
python3 test/run_livox2_tests.py build
python3 html/viewer/_Livox1/tests/browser-smoke.py
python3 html/viewer/_Livox1/tests/browser-smoke.py --response /tmp/livox1-config.json
python3 html/viewer/_Livox1/tests/browser-smoke.py --live-url http://localhost:8080/
```

The C++ regression suites use synthetic packets and loopback UDP endpoints;
they do not access a physical LiDAR. The browser test uses installed
Chrome/Chromium and Python's standard library. Default
mode exercises fragmented replies, Avia controls, unsupported parameters,
validation, request correlation, acknowledgements, saving, IMU lifecycle, stale
samples, the version-7 binary point and line streams, render credits, and mobile
layout. `--response` additionally renders a captured driver `getConfig` reply.
`--screenshot /tmp/livox1.png` saves a desktop preview.

Live mode reads configuration, IMU, and point clouds without changing sensor
settings. It requires a running Avia-generation backend with sampling, IMU
capture, and preview fusion enabled. Leave a command-client slot available.
Simulated tests do not establish hardware compatibility; physical LiDAR
verification is separate.

Protocol reference: [Livox SDK Communication Protocol](https://github.com/Livox-SDK/Livox-SDK/wiki/Livox-SDK-Communication-Protocol).
