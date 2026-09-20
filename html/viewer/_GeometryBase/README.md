# OpenKAI _GeometryBase viewer

Open `index.html`, enter the backend host and port, and click **Start**, or open
`http://localhost:8080/` while the backend is running. Drag to orbit, right drag to
pan, and scroll to zoom. The viewer shows point clouds and lines with a connection
bar and statistics. Camera pose, automatic fitting, and the reference grid come
from the backend configuration.

The backend serves this directory, including the vendored three.js assets; no
npm, CDN, or separate web server is required. Enable `WITH_UNIVERSE` when building
OpenKAI. CMake copies and installs both web viewers.

```json
"viewer": {
  "class": "_WebGeometryBase",
  "bON": true,
  "thread": { "FPS": 30 },
  "host": "0.0.0.0",
  "port": 8080,
  "nPbuf": 400000,
  "nLbuf": 10000,
  "bFrame": true,
  "dTexpire": 0,
  "bAutoBound": true,
  "bShowGrid": true,
  "vGeometry": [
    { "_GeometryBase": "points", "nP": 400000, "nL": 0, "matPointSize": 2 },
    { "_GeometryBase": "lines", "nP": 0, "nL": 10000 }
  ]
}
```

`vGeometry` resolves to a vector of `_GeometryBase*` sources. Per-source `nP` and
`nL` are capped by `nPbuf` and `nLbuf`; zero disables that type for the source.
`bVisible` and the fallback color `matCol` retain the existing viewer's meanings.

- `bFrame: false` (default) reads points using `get()` and the geometry ring buffer.
- `bFrame: true` reads points using `getLastFrame()`. `_PointCloud` provides the
  latest completed frame; sources without frame support return an empty snapshot.
- Lines always use `get()`. `dTexpire` filters point/line timestamps in stream mode
  and the completed point frame's timestamp in frame mode; zero disables expiry.

[`jsonCfg/Orbbec.json`](../../../jsonCfg/Orbbec.json) enables this viewer and colored
Orbbec point clouds, with `bFrame: true` and the `obPCL` source.

The viewer uses the same [version-5 protocol](../../../docs/3D/WebViewer3D.md#binary-protocol-version-5)
as `_SelectableOctGrid`: `/stream/points` and `/stream/lines`, JSON `hello`, binary
RGB snapshots, and `start`/`next` flow control. Each connection reconnects and
clears independently. The backend reuses the existing protocol serialization.
Cell streams and `_WSconsole` commands are not part of this viewer.

Checks (from the repository root, after building OpenKAI):

```sh
python3 test/webViewer3D/geometry.py build
cmake -S test/webViewer3D -B /tmp/openkai-web-tests
cmake --build /tmp/openkai-web-tests
ctest --test-dir /tmp/openkai-web-tests --output-on-failure
python3 test/webViewer3D/geometry_browser.py /tmp/openkai-web-tests/viewer_fixture html/viewer/_GeometryBase
```

The native checks use synthetic `_PointCloud` frames and `_Line` data. The browser
check requires Chromium or Google Chrome and exercises the frontend against the
existing version-5 fixture, including mouse navigation and cell-frame rejection.
