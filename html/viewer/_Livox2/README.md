# OpenKAI Livox2 viewer

From the repository root, run:

```sh
build/OpenKAI jsonCfg/Livox2.json
```

Open `http://localhost:8080/` and click **Start** for the point cloud and command
connection. **Connect** in the right panel opens only the command connection.
Click **Start IMU** for the orientation axes and six live gyro/acceleration plots.
The page, shared assets and vendored Three.js are served by OpenKAI; npm and an
internet connection are not needed.

The browser's **IP / host** is the PC running OpenKAI, not the LiDAR's address.
For another computer on the same network, open `http://<OpenKAI-PC-IP>:8080/`.
The MID-360 sensor is configured at `192.168.16.120` in `Lvx1.lvxIP`.
`_Livox2` owns its UDP sockets and automatically selects the PC's Ethernet
address from the route to the sensor.
The viewer uses TCP **8080** for the page and geometry, and TCP **7890** for
commands. The sensor uses separate UDP ports specified in the launch JSON.
The standalone launch allows four command clients; the GLIM launch allows eight.

All LiDAR networking is configured in the `_Livox2` instance:

```json
{
  "Lvx1": {
    "class": "_Livox2",
    "bIMUpreview": true,
    "thread": {"FPS": 0},
    "lvxIP": "192.168.16.120",
    "lvxHostPortCmd": 56101,
    "lvxHostPortState": 56201,
    "lvxHostPortPCL": 56301,
    "lvxHostPortIMU": 56401,
    "PCLframeOut": "livoxPoints",
    "IMUstreamOut": "livoxIMU"
  }
}
```

The four host ports above are defaults. `lvxHostIP` is optional; omit it or use
`0.0.0.0` for automatic host selection, or set a local IPv4 address explicitly.
`lvxPortQuery` defaults to 56000; the sensor advertises its command port during
discovery. `receiveBufferBytes` defaults to 4194304 per socket. `thread.FPS: 0`
lets socket polling pace the receive loop. There are no separate LiDAR `_UDP`
modules, packet streams, or receive-worker settings. Point-cloud and IMU output
DataObjects remain independent and available to other modules.

The MID-360 broadcasts discovery replies to `255.255.255.255` at the query
socket's automatically assigned local UDP port. The driver receives discovery
on a wildcard socket and selects the Ethernet address separately for its four
fixed host ports. If a firewall is enabled, allow UDP replies from the sensor,
including these broadcast discovery replies and traffic to the configured host
ports. The status panel shows the active host address and whether the UDP
sockets are open.

For a standalone page preview, serve the whole HTML tree:

```sh
python3 -m http.server 8000 --directory html
```

Then open `http://localhost:8000/viewer/_Livox2/`, set the stream port to **8080**
and the host to the OpenKAI PC. Directly opening `index.html` also works as a
launcher: **Start** navigates to the backend page. Add `?cmdPort=7891` to the
page URL when using a different command port.

## Settings and status

The LiDAR module defaults to **Lvx1**. **Refresh config** retrieves the driver's
current configuration, supported parameter schema and device status. Controls
include coordinate format, detection/work modes, embedded IMU, point-cloud frame
duration, distance filtering, reflectivity/color, IMU stabilization, maximum frame
size and timeout, as supported by the connected device. Network settings and
device identity are read-only; edit network settings on `Lvx1` in the launch
JSON before restarting OpenKAI. MID-360 does not expose unsupported frame-rate
or scan-pattern choices.

Edits send a single `setConfig` patch. Device writes are asynchronous: the viewer
shows queued settings and waits for sensor acknowledgements before allowing
another edit or save. Device errors appear above the controls. **Save config**
writes the current configuration to the original launch JSON; the browser cannot
choose another file. Status and packet counters refresh every two seconds, or
more often while commands are pending. Periodic refresh pauses while an editable
field has focus so unfinished input is preserved.

Point-cloud frame duration controls OpenKAI's accumulation window, rather than
the MID-360's physical scan rate. Shorter windows reduce latency and produce
fewer points in each frame. IMU stabilization changes the displayed/output point
coordinates; leave it off when GLIM consumes the raw LiDAR and IMU streams.

## IMU and SLAM

The IMU preview polls a non-consuming snapshot at up to 10 Hz. **Stop IMU** only
stops browser polling. Embedded IMU capture and the `IMUstreamOut` used by GLIM
continue independently. Gyroscope units are rad/s; acceleration is m/s².
Orientation is relative, and yaw can drift without an external heading reference.
Disabled, disconnected or stale data clears the plots and orientation rather than
remaining displayed as live samples. Changing the selected module or disconnecting
also stops and clears the preview.

`bIMUpreview` defaults to `false` and skips backend preview fusion and snapshot
work. `Livox2.json` enables it for sensor verification; `GLIM_livox2.json`
disables it. Change **Enable IMU preview** in the controls and save the
configuration to persist the choice. Browser Start/Stop controls polling;
disabling this setting is what stops backend preview computation.
Livox's optional `bIMUstab` cloud stabilization uses the same orientation estimate
and requires `bIMUpreview: true`. Turn stabilization off before disabling preview,
or update both flags together. Incompatible combinations are rejected.

Use `jsonCfg/GLIM_livox2.json` with its `jsonCfg/glim_livox2/` settings to run
LiDAR-inertial SLAM. The configuration enables the MID-360 IMU and keeps its data
available to GLIM regardless of whether a browser is connected.

## Protocol

Point clouds and lines use the existing `_WebGeometry` version-6 binary streams
at `/stream/points` and `/stream/lines`, with credit-based flow control.
Configuration and IMU preview use the separate `_WSconsole` WebSocket, with
JSON requests followed by the literal `EOJ` delimiter. Replies may be fragmented
across WebSocket messages. Both directions include `module` and `requestId`.

```json
{"module":"Lvx1","cmd":"getConfig","requestId":"lidar-1"}
{"module":"Lvx1","cmd":"setConfig","requestId":"lidar-2","config":{"lvxIMUdataEn":1}}
{"module":"Lvx1","cmd":"setConfig","requestId":"lidar-3","config":{"frameDurationMs":100}}
{"module":"Lvx1","cmd":"saveConfig","requestId":"lidar-4"}
{"module":"Lvx1","cmd":"getIMU","requestId":"imu-1"}
```

Configuration replies contain `bSuccess`, `config`, `schema`, `deviceOpen` and
`status`; failed requests include `error` or per-field `errors`. Schema metadata
provides types, limits, choices and read-only flags. `status.pendingCommands` and
`status.configApplied` distinguish accepted changes from sensor acknowledgements;
`lastError` reports asynchronous device failures.

`getIMU` replies contain `bSuccess`, `enabled`, `previewEnabled`, `deviceOpen`, `available`, `gyro`
and `acc` XYZ arrays, plus `orientationValid`, `quaternion` in WXYZ order and `rpy`
in radians. `tGyro`, `tAcc` and `tFusion` are nanosecond timestamps encoded as
decimal strings to preserve browser precision. Always honor the validity flags.

## Validation

```sh
python3 html/viewer/_Livox2/tests/browser-smoke.py
python3 html/viewer/_Livox2/tests/browser-smoke.py --live-url http://localhost:8080/
```

The test uses Chrome/Chromium and Python's standard library. The default mode
tests the actual page with simulated fragmented replies, schema rendering,
numeric/boolean/vector edits, validation, request correlation, asynchronous
acknowledgements, saving, IMU lifecycle, stale samples and mobile layout.
`--response /path/to/getConfig.json` additionally replays a captured schema.
`--screenshot /tmp/livox2.png` saves the desktop preview.

Live mode reads configuration and IMU, checks real WebSocket framing,
reconnection and live point-cloud rendering, and requires a running backend with
the sensor IMU enabled. Leave one command-client slot available for the test.
Driver and GLIM validation commands are:

```sh
python3 test/run_livox2_tests.py build
python3 test/run_glim_timing_tests.py build
python3 test/Sensor/LiDAR/live.py build/OpenKAI
python3 test/Sensor/LiDAR/live.py build/OpenKAI --glim --seconds 20
```

The first two commands need no sensor. The hardware test starts its own backend
with a temporary launch file and HTTP ports, verifies all three point formats,
IMU enable/disable, invalid settings, frame duration, save and shutdown, and
restores the settings it edits. Stop other Livox backends before running it.
`--glim` also checks stationary inertial initialization, tracking and PLY export.
Use `--output /tmp/livox-test` to retain logs and results.

Live driver verification on 2026-10-23 used the owned UDP sockets with the
MID-360 at `192.168.16.120`: the 20-second capture published 199 frames and
received 4,021 IMU samples. All three point formats and clean shutdown passed,
with no invalid packets, command errors, or command retries. This run did not
repeat the browser verification below.

Live verification on 2026-10-22 used the MID-360 at `192.168.16.120`: about
10 clouds/s and 200 IMU samples/s, with configuration readback and all three point
formats passing. Chromium rendered the live cloud, controls and IMU preview;
reconnection and mobile layout passed. Motion accuracy and loop closure require
a separate moving-sensor test.
