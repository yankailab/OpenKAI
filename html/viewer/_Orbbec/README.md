# OpenKAI Orbbec viewer

From the repository root, run `build/OpenKAI jsonCfg/Orbbec.json`, then open
`http://localhost:8080/`. You can also open this directory's `index.html`, enter
the backend host/ports and click **Start**. The geometry service serves the page
and the vendored three.js assets; no npm or external web server is needed.

The center is copied from `_GeometryBase` and uses `_WebGeometry`'s existing
version-6 `/stream/points` and `/stream/lines` connections. Camera controls use a
separate `_WSconsole` connection on port **7890**. The camera module name defaults
to **Orbbec**, matching `jsonCfg/Orbbec.json`.

## Camera controls

**Refresh config** retrieves the current backend parameters, readable device values,
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
into overrides. Presets may reset other device properties; use **Refresh config**
to refresh the device readbacks after applying a preset.

**Save config** updates the camera module in the original launch JSON file.
Current controls are saved in place; unset optional overrides are removed.
Module links, thread settings and unrelated modules are preserved. The saved
settings are read by `loadConfig()` on the next launch. A save failure is
returned to the browser.

Camera IMU capture publishes to the `IMUstream` named by `IMUstreamOut` for independent
consumers such as SLAM. This page displays the camera point cloud and camera
controls; the former `_IMUbase` preview commands are removed.

## JSON messages

Requests are JSON objects followed by `EOJ`, using the existing WSconsole framing.
Replies are JSON objects, possibly split across WebSocket text messages. Requests
and replies carry `module` and optional `requestId` for routing/correlation.

```json
{"module":"Orbbec","cmd":"getConfig","requestId":"camera-1"}
{"module":"Orbbec","cmd":"setConfig","config":{"OB_PROP_COLOR_EXPOSURE_INT":100}}
{"module":"Orbbec","cmd":"saveConfig"}
```

Camera replies include `bSuccess`, `config` and `deviceOpen`; load also includes
`schema`. Rejections include `error` or per-field `errors`. `getConfig` retrieves
live parameters.
No browser request can select a save-file path.

The shared geometry snapshot regression checks are documented in
[`../tests/README.md`](../tests/README.md). Camera hardware is required to validate
live capture and device property changes.
