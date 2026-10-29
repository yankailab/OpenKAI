# OpenKAI Scepter viewer

From the repository root, run:

```sh
build/OpenKAI jsonCfg/Scepter.json
```

Open `http://localhost:8080/` and click **Start**.
Opening `index.html` directly also works: enter the backend host and ports, then
click **Start** to navigate to the backend's served viewer.

The point cloud uses `_WebGeometry`'s version-6 `/stream/points` and
`/stream/lines` connections, as in the Orbbec viewer. Camera commands use the
existing `_WSconsole` connection on port **7890**, with module **scepter**.
There is no IMU panel or IMU connection. All three.js assets are vendored.

## Controls and persistence

The right panel groups controls into streams/point cloud, trigger, ToF exposure,
color, filters/IR, network/time, AI module, and SDK preset. Every serializable
`ScCtrl` field is included, plus stream capture switches, point cloud stride and
scale. Hot-plug function pointers and user-data pointers remain C++ only.

`pclColorMaxAgeMs` bounds reuse of the last aligned color image when depth and
color arrive separately or color frames are lost. The default is 1000 ms;
`0` requires color from the same capture. Both host age and camera timestamp
difference must be within the limit, and image dimensions must match. Depth
continues updating with white points if color is unavailable or expires.
Retained frames are cleared when the camera closes or configuration is applied.
Increasing the limit can smooth color gaps on Wi-Fi at the cost of color lag
on moving objects. It does not change depth positions or their timestamps.

A field change sends a `setConfig` patch immediately. Invalid types/ranges are
rejected before any changes are made. SDK failures are returned per field. A
closed camera rejects live edits. Stream profile changes pause and restart
capture; the command handler shares the frame lock with capture and conversion.
Waiting commands run between captures, even when capture takes longer than the
configured worker interval. Sending replies does not hold the camera lock.
Model-specific controls can be rejected by the device. Limits in the form check
the SDK types and documented ranges, not device-specific capability discovery.

Manual exposure/gain, trigger parameters, HDR/WDR exposure lists and AI settings
are retained while their corresponding mode is inactive, then sent when the mode
is enabled. Enabling HDR disables auto ToF exposure; selecting auto exposure
disables HDR. Exposure times are in microseconds. A full-frame color AEC ROI
follows resolution changes; a custom ROI must fit the selected color resolution.
Integer-keyed maps use JSON pairs, such as `[[0,4000],[1,1000]]` for exposure times
and `[[0,true],[3,false]]` for AI frame types. Empty lists store no indexed entries;
removing entries does not reset the corresponding camera values.

**Refresh config** refreshes the current backend parameters, as in the Orbbec viewer.
**Save config** updates the camera module in the original launch JSON file,
including defaults, false values and empty collections. Module links, thread
settings and unrelated modules are preserved. Browser commands cannot choose
the save path. No edits are written to disk until **Save config** is used.

`loadConfig()` reads the saved module settings on the next launch. All configured
controls are sent when the camera opens. An optional SDK preset is loaded before
explicit controls.

## JSON messages

Requests are JSON objects followed by `EOJ`; replies can span multiple WebSocket
text messages. `module` and `requestId` route and correlate replies.

```json
{"module":"scepter","cmd":"getConfig","requestId":"camera-1"}
{"module":"scepter","cmd":"setConfig","config":{"scTimeFilterThreshold":2}}
{"module":"scepter","cmd":"saveConfig"}
```

Replies contain `bSuccess`, `config`, `deviceOpen` and optional `errors`/`error`.
`getConfig` replies also contain the categorized control `schema`.
Request IDs include a random viewer identifier to distinguish replies across tabs.
For concurrent command clients, configure `_WebSocketServer` with
`bReadAllClients: true`, `wsMode: 3`, and a sufficient `nClientMax` (the
`AProver_LiDAR` sample uses eight). Each request must be sent as one complete
WebSocket message, including its `EOJ` terminator, as this viewer does.

## Checks

The color-buffer regression test runs without a camera and covers independent
depth/color arrivals, SDK buffer reuse, capture timeouts, color expiry,
configuration resets, preservation of depth timestamps, publication while
the SDK is blocked waiting for the next capture, and configuration replies
between consecutive captures:

```sh
python3 test/run_scepter_color_tests.py build
```

```sh
python3 test/Sensor/ScepterLive.py build/OpenKAI --ip 192.168.16.101
```

The live check requires a reachable camera, Chromium/Chrome and localhost sockets.
Stop other camera applications first. It uses temporary ports and a camera-only
launch file, connects a second command client before the browser, and checks
Refresh, editing the host color-cache age, saving, and continuing colored clouds.
Save writes only the temporary JSON; the original runtime color-cache age is
restored before shutdown. It writes `/tmp/openkai-scepter-live.png` and
`/tmp/openkai-scepter-live.log`. Add `--single-client` to isolate capture locking
from command routing.
