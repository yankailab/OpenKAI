# OpenKAI RoboSense Airy viewer

Run from the repository root with a build that enables `WITH_SENSOR`,
`WITH_UNIVERSE`, `WITH_IO`, `WITH_PROTOCOL` and `USE_WSSERVER`:

```sh
build/OpenKAI jsonCfg/Airy.json
```

Open `http://localhost:8080/` and click **Start**. Geometry uses the shared
`_WebGeometry` `/stream/points` and `/stream/lines` endpoints. Sensor commands use
`_WSconsole` on port **7890**, module **Airy**. The host field is the OpenKAI
computer's address. Opening `index.html` directly also works as a launcher for
that backend. Three.js assets are included locally; no new web packages or
RoboSense SDK are required.

A separate build without optional sensor SDKs can use:

```sh
cmake -S . -B build-airy -DWITH_SENSOR=ON -DWITH_UNIVERSE=ON \
  -DWITH_IO=ON -DWITH_PROTOCOL=ON -DUSE_WSSERVER=ON
cmake --build build-airy --parallel 4
build-airy/OpenKAI jsonCfg/Airy.json
```

## Network and calibration

The Airy guide gives factory defaults **192.168.1.200** for the sensor and
**192.168.1.102/24** for the receiving computer, with UDP **6699** for MSOP
points and **7788** for DIFOP device information. The connected sensor also
outputs IMU packets on UDP **6688** at **200 Hz**. Assign the destination address
to the computer's connected Ethernet interface. Run only one receiver on those
ports at a time. `deviceIP` filters the sender, while `bindIP` selects the local
interface; neither changes the sensor's network settings.

Airy uses its own 1248-byte MSOP/DIFOP formats, 48/96-channel packet models,
5 mm range units and per-unit angular calibration. The native receiver waits
for valid DIFOP calibration before publishing points, then splits scans at the
azimuth wrap and retains per-point capture timestamps. The output is in metres,
with X forward, Y left and Z up. The linked `_TransformRigid` instance's `vT` and
`vR` define the OpenKAI mounting transform. The supplied configuration reserves 200,000 points per frame to
accommodate dual returns; the manual specifies 10 Hz and a nominal 60 m maximum
range, with range varying by channel and target reflectivity.

The implementation follows the supplied Airy User Guide and checks firing
schedules and IMU fields against RoboSense's
[Airy decoder reference](https://github.com/RoboSense-LiDAR/rs_driver/blob/main/src/rs_driver/driver/decoder/decoder_RSAIRY.hpp).
It does not link or include the SDK. Packet model 192 is rejected explicitly;
the native decoder supports the documented 48/96-channel layouts.

## Controls

Every backend schema field is shown in the control panel. Local receiver
addresses, ports, buffers, timeouts, point limits, distance filters, color,
timestamps and IMU publication are editable. A change sends
an immediate `setConfig` patch. **Save config** writes the accepted values to
the publisher's section in the original launch JSON, preserving linked instances'
configuration. Configure and save the mounting transform separately through its
`_TransformRigid` instance during integration. Invalid changes display the backend error and restore
the accepted values. Device status refreshes every two seconds when no field
is being edited. The IMU samples section shows raw angular velocity in rad/s
and acceleration in m/s² in the IMU's own axes.

The left IMU preview uses the same layout as the Livox viewer: click **Start IMU**
after connecting to display colored XYZ orientation axes, roll/pitch/yaw, and
six scrolling angular-velocity and acceleration graphs. **Stop IMU** stops the
preview. Orientation is estimated from acceleration and gyroscope samples;
yaw may drift without a heading reference. The estimate is for visualization
and does not change the point cloud, raw IMU stream or GLIM configuration.

`bIMUpreview` defaults to `false` and skips backend preview fusion and snapshot
work. `Airy.json` enables it for sensor verification; `GLIM_Airy.json` disables
it. Change **Enable IMU preview** in the controls and save the configuration to
persist the choice. Browser Start/Stop controls polling; disabling this setting
is what stops backend preview computation. Raw IMU publication stays independent.

**Open sensor settings** opens Airy's built-in web interface at its detected
address. The Airy guide's Appendix A.2 describes the device controls there:
network destination/ports, return and operating modes, time synchronization,
phase lock, IMU rate/ranges, reflectivity and trail filters, rain/blockage
settings, frame start angle and laser state. These hardware parameters are
reported read-only in OpenKAI because the manual documents the device web UI
without specifying a supported parameter-write API. Changing a sensor port or
destination in that interface also requires updating the corresponding local
receiver setting. Stop SLAM before changing sensor timing, filters or transforms.

The default command service routes commands from one client at a time. Disconnect
one viewer's command connection before connecting another; geometry streams can
stay connected. This also applies when switching between the raw Airy viewer
and the GLIM viewer in `jsonCfg/GLIM_Airy.json`.

## IMU and GLIM

`Airy.json` publishes IMU samples to `airyIMU`; `bIMUoutput` controls publication
inside OpenKAI and does not turn the sensor's IMU on or off. `GLIM_Airy.json`
also exposes this stream but uses continuous-time LiDAR-only odometry by default.
The Airy DIFOP contains a nonidentity IMU calibration quaternion and translation;
the supplied manual does not define their transform direction and translation
units sufficiently to use them as GLIM extrinsics. Do not substitute identity
or Livox calibration. See [the Airy GLIM profile](../../../jsonCfg/glim_airy/README.md)
for launch and tuning instructions.

## Verification

```sh
python3 html/viewer/_Airy/tests/browser-smoke.py
python3 html/viewer/_Airy/tests/browser-smoke.py --live-url http://localhost:8080/
python3 test/run_airy_protocol_tests.py --sanitize
python3 test/run_airy_tests.py build
python3 test/Sensor/LiDAR/airy.py build/OpenKAI --live --seconds 10 --browser
python3 test/Sensor/LiDAR/airy.py build/OpenKAI --live --glim --seconds 15
```

The mock check exercises schema fields, typed validation, fragmented replies,
request correlation, error rollback, persistence commands, hardware settings
links, IMU telemetry, orientation, graph sampling and disconnect behavior. The
live check reads configuration, streams geometry and verifies fresh IMU
orientation without changing settings. Both check the mobile layout.
Use `--response /path/to/getConfig.json` to replay every field in a captured
backend response, or `--screenshot /tmp/airy.png` to save a preview.
