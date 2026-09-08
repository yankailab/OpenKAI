# Browser 3D viewer

`_WebViewer3D` derives from `_GeometryViewerBase` and reads points and lines from
the same `_GeometryBase::get()` ring buffers as `_ImGUIviewer`. The C++ process
serves the browser application and a binary WebSocket on one port. All browser
assets, including a pinned three.js release, are in `html/webViewer3D/`.
There is no browser-side installation, npm build, CDN, or separate web server.

## Build and run

Add `-DUSE_WEBVIEWER3D=ON` to your existing CMake configuration. A minimal build is:

```bash
cmake -S . -B build-web -DUSE_WEBVIEWER3D=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-web -j4
./build-web/OpenKAI jsonCfg/WebViewer3D.json
```

This enables `WITH_3D` and requires Boost headers version 1.70 or later in addition
to OpenKAI's normal build dependencies. Beast and Asio are compiled from headers;
no Boost runtime library, wsServer, Open3D, ImGui, or desktop GL backend is needed.
The sample loads the repository's `data/PointCloud/StanfordBunny/bun000.ply` through
the native `_PCfile` reader and displays `_OctreeGrid` lines alongside it. Run the
sample from the repository root so its data path resolves.

Open `html/webViewer3D/index.html` directly in the browser, enter the backend IP
and port (default `8080`), and click **Start**. The local launcher navigates to
the C++ server and automatically connects. Alternatively, visit
`http://BACKEND_IP:8080/` and click **Start** there.

The launcher redirects because browsers restrict ES modules loaded from
`file://`; the renderer and all imports run from the backend's HTTP origin.
See the [three.js installation guide](https://threejs.org/manual/en/installation.html).

CMake copies the frontend next to the executable as `html/webViewer3D/` after
building and installs it under `bin/html/webViewer3D/`. By default the viewer
looks in the working directory and then beside the executable on Linux. Set
`webRoot` explicitly for other deployment layouts. Copy the complete frontend
directory with the executable when deploying offline.

## Configuration

The existing `vGeometryBase` name list works:

```json
{
  "viewer": {
    "class": "_WebViewer3D",
    "thread": { "FPS": 30 },
    "host": "0.0.0.0",
    "port": 8080,
    "nPbuf": 200000,
    "nLbuf": 100000,
    "dTexpire": 0,
    "vCamEye": [0, -8, 4],
    "vCamLookAt": [0, 0, 0],
    "vCamUp": [0, 0, 1],
    "vGeometryBase": ["points", "lines"]
  }
}
```

Optional `vGeometry` entries add or override individual sources:

```json
"vGeometry": [
  { "_GeometryBase": "points", "nP": 200000, "nL": 0, "matPointSize": 2 },
  { "_GeometryBase": "lines", "nP": 0, "nL": 100000,
    "matCol": [0.3, 0.8, 1, 0.8], "bVisible": true }
]
```

| Setting | Default | Meaning |
| --- | --- | --- |
| `host`, `port` | `0.0.0.0`, `8080` | HTTP and WebSocket bind address and shared port |
| `webRoot` | `html/webViewer3D` | Directory of browser resources |
| `nClientMax` | `8` | Maximum simultaneous streaming clients, configurable from 1 to 64 |
| `thread.FPS` | framework default | Maximum geometry collection/publication rate |
| `nPbuf`, `nLbuf` | `200000`, `100000` | Scratch ring-buffer capacities, per source |
| `dTexpire` | `0` | Maximum geometry age in microseconds; zero disables expiry |
| `bAutoBound`, `bShowGrid` | `true`, `true` | Fit the first nonempty frame; show reference grid |
| `vBgCol` | `[0.035,0.045,0.065,1]` | Background color |
| `vGeometry[].nP`, `.nL` | scratch capacities | Per-object limits; zero omits that primitive type |
| `matPointSize`, `matCol` | `2`, `[1,1,1,1]` | Point size in pixels, fallback RGB and object opacity |
| `bVisible` | `true` | Exclude an object from streaming when false |

Camera configuration uses the base keys `camProjType` (0 perspective, 1
orthographic), `camFov`, `vCamNF`, `vCamLR`, `vCamBT`, `vCamEye`, `vCamLookAt`, and
`vCamUp`. It is sent when a browser connects. Camera movement, reset, visibility,
grid, and point scale are local to each browser. Automatic fitting runs once
on the first nonempty frame; **Fit scene** can be used subsequently.

Positions are float32. Colors are normalized RGBA8; black source colors use the
material fallback, matching the ImGui viewer's convention. Invalid timestamps,
expired geometry, and nonfinite positions are omitted. Lines use native WebGL
line segments at one pixel wide; thick-line materials are not implemented.

## Structure and streaming behavior

| File | Responsibility |
| --- | --- |
| `src/Net/HttpServer.*` | Static HTTP files, MIME types, request deadlines, path containment, WebSocket upgrade hook |
| `src/IO/WebSocketStream.*` | Asynchronous WebSocket sessions, shared snapshots, bounded delivery, lifecycle |
| `src/3D/Viewer/WebViewer3DProtocol.h` | Versioned little-endian binary encoding |
| `src/3D/Viewer/_WebViewer3D.*` | Framework configuration, collection, filtering and snapshot publication |
| `html/webViewer3D/js/launcher.js` | Local-file launcher and endpoint validation |
| `html/webViewer3D/js/wsBase.js` | Connection, protocol greeting, automatic reconnect and commands |
| `html/webViewer3D/js/protocol.js` | Frame validation and typed-array views |
| `html/webViewer3D/js/viewer3D.js` | Three.js scene, GPU buffers, camera and rendering |
| `html/webViewer3D/js/main.js` | UI, animation loop and render acknowledgements |

HTTP and WebSocket IO run on one asynchronous worker. Geometry collection runs
on a separate owned thread, stopped and joined before its resources are released.
The network thread never calls geometry providers. Providers retain responsibility
for synchronizing concurrent writes with their `get()` methods, as with ImGui.

Each connection receives a JSON `hello` with version, object names and camera
settings. It sends `start` to subscribe. One full binary snapshot is then sent;
the browser sends `next` after uploading and rendering it. While a browser is busy,
the backend replaces the latest snapshot and discards intermediate versions.
There is no growing FIFO of old frames. Each socket has at most one active write,
and all clients share immutable frame storage. `pause` disables delivery; `start`
resumes it. Ping/pong, close, masking, fragmentation and large frames are handled
by [Boost.Beast](https://www.boost.org/doc/libs/latest/libs/beast/doc/html/beast/using_websocket.html).

Geometry collection is skipped when there are no clients. Scratch vectors,
snapshot buffers, and browser GPU arrays are reused. Position/color sections
decode as views into the received ArrayBuffer; the renderer copies them into
reusable GPU upload arrays and updates only the occupied ranges. Empty objects
clear their previous geometry; absent objects release their rendering resources.
Total worst-case configured snapshot size is limited to 64 MiB and 1024 sources.

The existing `_WebSocketServer` and `_WebSocket` remain compatible with their
current users. They were evaluated for this transport, but their FIFO path uses
512-byte packets. The installed wsServer library also has a global client table,
blocking sends and no server shutdown API. Merely increasing the FIFO size would
not provide cancellable IO or independent slow-client handling. The new transport
is therefore a separate component under `src/IO`, with no dependency on the
legacy `USE_WSSERVER` option.

The embedded server provides plain HTTP/WS for local or trusted-network use. It
does not implement TLS or authentication. To expose it through HTTPS, use a TLS
endpoint that also forwards `/stream` WebSocket upgrades. The browser selects
`wss://` when the page is served over HTTPS.

## Binary protocol, version 1

All integers and IEEE float32 values are little-endian. A WebSocket binary
message contains one complete snapshot. Reserved fields are zero.

| Frame header offset | Type | Value |
| --- | --- | --- |
| 0 | uint32 | `0x31443357` (`W3D1`) |
| 4 | uint32 | Version `1` |
| 8 | uint32 | Sequence number, wraps at 2^32 |
| 12 | uint32 | Object count |
| 16 | uint32 | Entire message length in bytes |
| 20 | uint32 | Reserved |
| 24 | uint64 | Backend boot-clock timestamp in microseconds |

Each object starts with a 64-byte header:

| Object header offset | Type | Value |
| --- | --- | --- |
| 0, 4, 8 | uint32 × 3 | Object ID, point count P, line count L |
| 12 | uint32 | Reserved |
| 16, 20 | float32 × 2 | Point size, opacity |
| 24 | float32 × 6 | Axis-aligned bounds: min XYZ, max XYZ |
| 48–63 | bytes | Reserved |

The header is immediately followed by point XYZ (`12P` bytes), point RGBA
(`4P` bytes), line endpoint XYZ (`24L` bytes, A then B), and line endpoint RGBA
(`8L` bytes). All sections remain four-byte aligned. Each point costs 16 bytes;
each line costs 32 bytes. Full snapshots provide simple reconnect and object
removal behavior; delta updates can be added under a new protocol version.

## Verification

The standalone transport suite needs only Boost headers, CMake, a C++17 compiler,
and Python 3. It exercises real loopback sockets, path traversal (including
symlinks), large and fragmented frames, multiple peers, ACK flow control,
pause/resume, ping/pong, and shutdown with live connections.

```bash
cmake -S test/webViewer3D -B /tmp/openkai-webviewer-tests
cmake --build /tmp/openkai-webviewer-tests -j2
ctest --test-dir /tmp/openkai-webviewer-tests --output-on-failure
```

An optional Chrome/Chromium test also checks the local HTML launcher, real WebGL2
rendering, protocol rejection, camera/visibility controls, and Stop/Start:

```bash
python3 test/webViewer3D/browser.py /tmp/openkai-webviewer-tests/viewer_fixture html/webViewer3D
```

The browser test writes `/tmp/openkai-webviewer.png`. There are no npm or Python
package dependencies. Vendor provenance and hashes are recorded in
`html/webViewer3D/vendor/README.md`.

To verify the real framework collector with the sample point cloud and octree:

```bash
python3 test/webViewer3D/backend.py /path/to/build-web/OpenKAI
```
