# Shared web UI assets

The viewers, AProver and console editor share this directory. Keep
their paths relative to `html/common` when copying or deploying a UI.

## Styles

Load shared stylesheets in this order, followed by the UI's own stylesheet:

| Stylesheet | Used by |
| --- | --- |
| `css/theme.css` | All UIs |
| `css/viewer.css` | All viewers |
| `css/point-cloud.css` | All viewers except MAVLink |
| `css/viewer-panels.css` | GLIM, Orbbec, RealSense, Scepter, SporSensor |
| `css/sensor-controls.css` | Orbbec, RealSense, Scepter, SporSensor |
| `css/control.css` | AProver |

Shared rules contain matching declarations from the existing UIs. Page-specific
layouts, controls and responsive differences remain in each UI's `css/` folder.
GeometryBase needs only shared stylesheets.

## Scripts and images

- `js/wsCmdBase.js`: common viewer command-socket lifecycle.
- `js/viewerLauncher.js`: launcher for viewers with a command port; imports each
  page's own `js/main.js` relative to the document.
- `js/geometryConnection.js`: point, line and grid stream connections.
- `js/geometryProtocol.js`: point/line decoding for GeometryBase and sensor viewers.
- `js/geometryViewer3D.js`: GeometryBase and sensor rendering. Each page's import
  map resolves Three.js and OrbitControls to its local `vendor/` assets.
- `img/OK.png`: the shared OpenKAI logo.

Distinct protocols, controls, renderers, map assets and vendor directories remain
with their UIs. Empty local `css/`, `js/` and `img/` folders retain `.gitkeep` files.

## Serving

OpenKAI's viewer servers mount this directory at `/common/`, resolving it from
`webRoot/../../common`. CMake copies and installs it alongside the viewers.
For a standalone static server, serve the complete `html` tree, for example:

```sh
python3 -m http.server 8000 --directory html
```

Then open `/viewer/_GeometryBase/`, `/AProver/aprover.html`, or
`/console/editor/` as appropriate. Directly opening a viewer's `index.html`
still launches its backend connection, and the editor still works offline.

[AProver](../AProver/aprover.html) is the companion UI for
[`samples/AProver_LiDAR`](../../samples/AProver_LiDAR/). Serve the `html` tree
on the sample's host and open `http://<sample-host>:8000/AProver/aprover.html`.
It connects to that host's WebSocket port `7890`, using the `apRover` controller
and `geoFence` modules configured in
[`AProver_LiDAR.json`](../../samples/AProver_LiDAR/AProver_LiDAR.json).
