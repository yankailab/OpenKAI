# OpenKAI RoboSense E1R viewer

The driver uses `WITH_SENSOR` and the existing `WITH_UNIVERSE` point-cloud
infrastructure. The example viewer also uses `WITH_IO`, `WITH_PROTOCOL`, and
`USE_WSSERVER`. A build without optional sensor SDKs is:

```sh
cmake -S . -B build-e1r -DWITH_SENSOR=ON -DWITH_UNIVERSE=ON \
  -DWITH_IO=ON -DWITH_PROTOCOL=ON -DUSE_WSSERVER=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
cmake --build build-e1r -j4
build-e1r/OpenKAI jsonCfg/E1R.json
```

With an existing build that enables those options, run from the repository root:

```sh
build/OpenKAI jsonCfg/E1R.json
```

Open `http://localhost:8080/` and click **Start**. The point cloud uses the shared
`_WebGeometry` `/stream/points` and `/stream/lines` endpoints. Commands use
`_WSconsole` on port **7890**, module **E1R**. Enter the OpenKAI computer's address
in the viewer's host field. Opening `index.html` directly also works as a launcher
for the backend's served viewer. Three.js assets are included locally, matching
the other OpenKAI viewers; no additional web packages are required.

The E1R guide gives factory defaults **192.168.1.200** for the sensor and
**192.168.1.102** for the destination computer, with UDP **6699** for MSOP points
and **7788** for DIFOP device information. Configure the computer's Ethernet
interface to receive the sensor's unicast traffic. Changing OpenKAI's bind/source
addresses changes the receiver only; it does not change the sensor's network
configuration.

The controls are generated from the driver's schema so every exposed parameter
is available, including receiver ports/buffers/timeouts, distance/noise filters,
reflectivity/color, timestamps, position and orientation. A field change sends
an immediate `setConfig` patch. **Save config** persists settings to the original
launch JSON; the browser cannot choose a different save path. Invalid changes
show the backend error and restore the accepted values. Settings can be edited
while waiting for sensor data. **Refresh config** reloads the backend state;
device status also refreshes every two seconds when no field is being edited.

With noise filtering enabled, point attributes `0` (unclassified output observed
on firmware 8.1.1) and `1` (normal in the manual) are accepted. Attribute `2`
(noise) and unknown attribute values are rejected. Zero ranges are always dropped.
Firmware 8.1.1 reports `0` at the manual's frame-rate field offset even though
the observed stream runs at 10 Hz, so that field is displayed as a raw code.

The supplied E1R manual describes MSOP and DIFOP as output protocols. Device
network, frame-rate, return, synchronization and PHY parameters are displayed
read-only from DIFOP; no undocumented sensor commands are sent. They are
separate from editable OpenKAI receiver settings. Fields requiring a restart
are labeled by the driver schema. If the sensor is configured to a different
destination address, update the Ethernet interface or use the manufacturer's
configuration tool before receiving points.

Requests are JSON objects terminated by `EOJ`; fragmented replies are supported:

```json
{"module":"E1R","cmd":"getConfig","requestId":"e1r-1"}
{"module":"E1R","cmd":"setConfig","requestId":"e1r-2","config":{"bDropNoise":true}}
{"module":"E1R","cmd":"saveConfig","requestId":"e1r-3"}
```

Run the browser checks with installed Chrome/Chromium and Python's standard
library; no sensor is needed for the mock test:

```sh
python3 test/Sensor/LiDAR/e1r.py build/OpenKAI
python3 test/Sensor/LiDAR/e1r.py build/OpenKAI --live
python3 test/Sensor/LiDAR/e1r.py build/OpenKAI --live --seconds 30 --browser --output /tmp/e1r-live
python3 html/viewer/_E1R/tests/browser-smoke.py
python3 html/viewer/_E1R/tests/browser-smoke.py --live-url http://localhost:8080/
```

The integration check starts OpenKAI with a temporary launch file and synthetic
UDP packets, verifying decoding, filtering, pose transforms, rejected changes,
persistence, socket reconfiguration and shutdown. Its `--live` mode reads the
physical sensor using the network settings in `jsonCfg/E1R.json`.

The live check reads configuration and streams geometry without modifying
settings. The mock check covers all controls, fragmented/correlated replies,
validation, save, device readback, status polling and disconnect behavior. Both
check the mobile layout. `--response /path/to/getConfig.json` additionally checks
all fields in a captured backend reply; `--screenshot /tmp/e1r.png` saves a preview.

Protocol and driver regression tests and a synthetic UDP-to-viewer test:

```sh
python3 test/run_e1r_protocol_tests.py
python3 test/run_e1r_tests.py build
python3 test/Sensor/LiDAR/e1r.py build/OpenKAI
python3 test/Sensor/LiDAR/e1r.py build/OpenKAI --live
```

The synthetic test uses temporary configuration and loopback ports; `--live`
reads the real sensor without sending device commands. The wire decoder follows
the E1R User Guide v1.0 sections 4.3.1–4.3.2: 1200-byte MSOP packets with 96
points, signed Q15 direction vectors and 5 mm radius units, plus 256-byte DIFOP
packets. It validates lengths, headers, sensor type and timestamp bounds. The
E2E trailer is decoded, but CRC verification is not implemented because the
guide does not specify its coverage and initialization. The driver uses standard
POSIX UDP sockets and existing OpenKAI types; no RoboSense SDK, PCL, or libpcap
is added.
