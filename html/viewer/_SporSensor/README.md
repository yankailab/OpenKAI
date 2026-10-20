# OpenKAI SporSensor viewer

Build OpenKAI with the local C++17 SDK and launch the supplied configuration:

```sh
cmake -S . -B build \
  -DWITH_SENSOR=ON -DWITH_UNIVERSE=ON \
  -DWITH_IO=ON -DWITH_PROTOCOL=ON -DUSE_WSSERVER=ON \
  -DUSE_SPOR_SENSOR_SDK=ON \
  -DSporSensorSDK_ROOT=/home/kai/dev/SporSensorSDK
cmake --build build -j 4
build/OpenKAI jsonCfg/SporSensor.json
```

Open `http://localhost:8080/` and click **Start**. The supplied VS Code launch
configuration launches the same JSON. Opening this folder's `index.html` directly
also works: enter the OpenKAI backend host and ports, then click **Start** to
navigate to its served viewer. The host in the page is the OpenKAI computer;
the scanner address is the separate `address` setting, initially `192.168.80.8`.

This uses the same folder structure, vendored three.js, `_WebGeometry` version-6
`/stream/points` and `/stream/lines` WebSockets, and `_WSconsole` JSON commands as
the Orbbec, RealSense and Scepter viewers. The default command port is **7890**
and module name is **sporSensor**. No new point-cloud transport is introduced.

## Profiles and connection

The latest profile appears in the X/Z plane. Sensor coordinates are converted
from millimetres to metres in `PCLframe`; host `yOffsetM` positions the line along
Y. The viewer can orbit, pan, zoom, fit the current profile and reset the camera.
Brightness colors use the optional per-point intensity; samples without
brightness use the configured fallback color. Invalid sensor samples are omitted.
The host can select top, bottom or both sides and stride output sampling.

**Start** and **Stop** in the header connect/disconnect the web stream.
**Start acquisition** and **Stop acquisition** in the control panel operate the
sensor. They are separate operations. A software trigger requires running
acquisition and software trigger mode. Increase the receive timeout for sparse
external or software triggers.

Close SLK before using the configuration service. The SDK configuration client
needs bidirectional communication, including two incoming TCP connections to the
OpenKAI computer. `localAddress` selects the wired network interface if necessary;
`sensorId` selects the sensor's discovery namespace. The supplied device uses
`10000845`. The native command/data ports and configuration timeout, reconnect
interval, protocol version and profile-decoder limits are exposed in the form.

## Configuration and persistence

The form is generated from the module's complete configuration schema. It
includes all SDK connection and parser options, acquisition/scan settings,
exposure, laser state, timing and power, brightness, trigger source and gate,
scan/frame rate, measurement output, area of interest, six-axis transformation,
and gap-filling, median, smoothing and decimation filters in both X and Y.
Each control exposes its type, limits, units and current readback when available.
SDK fields without typed setters can be inspected using **Read project** or a
specific model GUID and optional device ID under **Read model**.

An edit sends one `setConfig` patch, except the four area values, which are
staged together until **Apply area** is clicked. **Preserve area** removes all four
overrides together and leaves the current sensor ROI unchanged. Pending area
edits must be applied or cleared before saving.

Optional blank fields and **Preserve sensor
setting** are stored as `null`; they do not force a value such as zero or false.
Removing an override preserves the sensor's current value and does not restore
an earlier value. Read-only values cannot be edited. Device failures appear in
the status line and returned accepted values replace rejected edits.

ROI dimensions and translations use millimetres; sensor rotations use degrees.
Laser power is the vendor 0–100 scale. Exposure selects single-exposure mode.
Laser timing (Normally/Strobe) differs from emission state (Triggered/Always
on/Off). `frameRateHz` uses verified settings; `scanRateHz` uses the native
acknowledgment. Select one rate override and preserve the other.
Filters require uniform profile spacing. X filters operate within a profile;
Y filters act across profiles in point-cloud processing. Filter amounts use
sensor point-spacing units, not an integer window count. Firmware limits still
apply. **Use maximum frame rate** chooses the current maximum once.

**Refresh config** reloads module parameters and sensor status. The snapshot
panel exposes device models/readbacks and explicit project/model queries.
**Save config** saves the module's accepted configuration into the original
launch JSON; it does not send the sensor's SaveJob command. Browser commands
cannot choose a save path. Sensor retention after power loss is separate from
OpenKAI's saved configuration. Leaving a parameter null means the sensor's
setting is preserved on the next launch.

## JSON commands

Requests use the existing `EOJ` framing. Responses may span multiple WebSocket
text messages. `module` and `requestId` route and correlate replies.

```json
{"module":"sporSensor","cmd":"getConfig","requestId":"spor-1"}
{"module":"sporSensor","cmd":"setConfig","config":{"laserPower":80},"requestId":"spor-2"}
{"module":"sporSensor","cmd":"setConfig","config":{"laserPower":null},"requestId":"spor-3"}
{"module":"sporSensor","cmd":"saveConfig","requestId":"spor-4"}
{"module":"sporSensor","cmd":"softwareTrigger","requestId":"spor-5"}
{"module":"sporSensor","cmd":"readModel","guid":"DEA16047-ADEB-4D37-818B-6E4F8DF3ED17","requestId":"spor-6"}
```

Configuration replies include `bSuccess`, `config`, `schema`, `actions`,
`deviceOpen`, `status`, and optional `error`/`errors`/`device`. Inspection commands
return their snapshot as `result`.

## Browser checks

Run the offline browser smoke test with installed Chrome/Chromium:

```sh
python3 html/viewer/_SporSensor/tests/browser-smoke.py
```

It checks schema rendering, optional values, booleans, enums, arrays, rejected
edits, request correlation, fragmented JSON replies, acquisition/inspection
commands, save and disconnection. To check read-only configuration access to a
running backend:

```sh
python3 html/viewer/_SporSensor/tests/browser-smoke.py --live-url http://localhost:8080/
```

Backend configuration tests and the live sensor/browser integration check:

```sh
python3 test/run_spor_sensor_tests.py build
python3 test/Sensor/LineScanner/live.py build/OpenKAI
```
