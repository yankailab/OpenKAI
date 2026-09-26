# OpenKAI Scepter viewer

From the repository root, run:

```sh
build/OpenKAI jsonCfg/Scepter.json
```

Open `http://localhost:8080/` and click **Start**.
Opening `index.html` directly also works: enter the backend host and ports, then
click **Start** to navigate to the backend's served viewer.

The point cloud uses `_WebGeometryBase`'s version-6 `/stream/points` and
`/stream/lines` connections, as in the Orbbec viewer. Camera commands use the
existing `_WSconsole` connection on port **7890**, with module **scepter**.
There is no IMU panel or IMU connection. All three.js assets are vendored.

## Controls and persistence

The right panel groups controls into streams/point cloud, trigger, ToF exposure,
color, filters/IR, network/time, AI module, and SDK preset. Every serializable
`ScCtrl` field is included, plus stream capture switches, point cloud stride and
scale. Hot-plug function pointers and user-data pointers remain C++ only.

A field change sends a `setConfig` patch immediately. Invalid types/ranges are
rejected before any changes are made. SDK failures are returned per field. A
closed camera rejects live edits. Stream profile changes pause and restart
capture; the command handler shares the frame lock with capture and conversion.
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

## Checks

```sh
cmake --build build -j 4
python3 test/scepter/native.py build
python3 test/scepter/config.py build
python3 test/scepter/browser.py build/OpenKAI
```

Native checks use the installed SDK with simulated calls, requiring no camera.
They cover frame handling, configuration initialization and saving, malformed commands, offline edits,
SDK rejections, exposure dependencies and stream restart. The browser check needs
Chromium/Chrome and localhost sockets. It uses temporary ports/config/save files,
checks the actual WSconsole connection, controls, persistence, fragmented replies
and disconnection, and checks live points if a camera opens. It writes a screenshot
to `/tmp/openkai-scepter-viewer.png`.
