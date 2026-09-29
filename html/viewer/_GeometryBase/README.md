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
  "dTexpire": 0,
  "bAutoBound": true,
  "bShowGrid": true,
  "vGeometry": [
    { "PCLframe": "points", "nP": 400000, "nL": 0, "matPointSize": 2 },
    { "LineFrame": "lines", "nP": 0, "nL": 10000 }
  ]
}
```

`vGeometry` resolves independent `PCLframe` and `LineFrame` DataObjects. Declare
those streams in the launch configuration and connect producers to the same
names. A source entry may contain either or both stream types; `name` optionally
sets its display label. Per-source `nP` and `nL` are capped by `nPbuf` and
`nLbuf`; zero disables that type for the source. `bVisible` and the fallback
color `matCol` control rendering.

Each changed stream timestamp replaces the complete point or line frame,
including empty publications that clear the display. `dTexpire` filters individual
record timestamps in nanoseconds; zero disables expiry. The backend copies
source records with `get` and reuses those copies and encoded data until the
stream timestamp changes or records expire. Equal timestamps cannot signal a
changed payload. `bFrame` and module-based geometry sources are no longer used.

The viewer uses the same [version-6 protocol](../../../docs/3D/WebViewer3D.md#binary-protocol-version-6)
as `_SelectableOctGrid`: `/stream/points` and `/stream/lines`, JSON `hello`, binary
RGB snapshots, and `start`/`next` flow control. Each connection reconnects and
clears independently. The backend reuses the existing protocol serialization.
Cell streams and `_WSconsole` commands are not part of this viewer.

Regression checks for both geometry viewers are documented in
[`../tests/README.md`](../tests/README.md).
