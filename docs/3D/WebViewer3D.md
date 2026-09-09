# Browser 3D viewer

`_WebViewer3D` derives from `_GeometryViewerBase` and reads points and lines from
the same `_GeometryBase::get()` ring buffers as `_ImGUIviewer`, plus compact
`_OctreeGrid::get(OCTGRID_CELLS*)` snapshots. The C++ process
serves the browser application and a binary WebSocket on one port. A separate
WebSocket connects JSON application commands to `_WSconsole`. All browser
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
no Boost runtime library, wsServer, Open3D, ImGui, or desktop GL backend is needed
for streaming. The current sample uses `_Scepter` camera geometry and octree
cells; enable its camera build dependencies when running that configuration.
The optional `_PCfile` source can load `data/PointCloud/StanfordBunny/bun000.ply`.
Run from the repository root so relative data paths resolve.

Open `html/webViewer3D/index.html` directly in the browser, enter the backend IP
and port (default `8080`), and click **Start**. The local launcher navigates to
the C++ server and automatically connects. Alternatively, visit
`http://BACKEND_IP:8080/` and click **Start** there. **Start** opens the stream
and command sockets independently; **Stop** closes both. The Commands section
under Objects has its own Connect/Disconnect buttons and a read-only `cmdState`
console. A command connection failure leaves geometry streaming active.

The launcher redirects because browsers restrict ES modules loaded from
`file://`; the renderer and all imports run from the backend's HTTP origin.
See the [three.js installation guide](https://threejs.org/manual/en/installation.html).

CMake copies the frontend next to the executable as `html/webViewer3D/` after
building and installs it under `bin/html/webViewer3D/`. By default the viewer
looks in the working directory and then beside the executable on Linux. Set
`webRoot` explicitly for other deployment layouts. Copy the complete frontend
directory with the executable when deploying offline.

## JSON command connection

The command port defaults to `7890` and uses the IP/host entered in the page.
Change it in the Commands panel before connecting. The local HTML launcher
preserves this port when navigating to the C++ server; `?cmdPort=7890` can also be
used in a hosted page URL. Command Connect/Disconnect affects only that socket.
After an unexpected command disconnect, use Connect to reconnect; commands are
never queued or replayed automatically.

For the existing `_WSconsole` backend, enable `WITH_IO`, `USE_WSSERVER`,
`WITH_PROTOCOL` and `WITH_UI` in addition to your application's build options.
Include the companion configuration in your application's `APP` block:

```json
"vInclude": ["jsonCfg/WebViewer3D_commands.json"]
```

That file defines `_WebSocketServer` on port 7890 in text mode (`wsMode: 2`) and
`_WSconsole` linked through `_IObase`. Extend its `vBASE` list with the names of
modules that should receive commands. `_WSconsole` dispatches using the JSON
`module` field and calls that module's `console(const json &, void *)` method.
The module implements its own commands and can reply through `_JSONbase::sendJson()`.
The companion config allows one command client because the existing server's
read/write interface targets client 0; geometry streaming retains its own
independent multi-client support.

The classic-script API is intentionally small and reusable:

```javascript
wsInit(); // Connect using the page's host and command port.
wsSendCmd({ cmd: 'test', module: 'tester', v: 0 });
wsStop(); // Disconnect the command socket only.
```

`wsSocket`, `strEOJ`, `wsInit()` and `cmdHandler(event)` retain the existing scheme.
`wsSendCmd()` serializes an object with the `EOJ` suffix expected by
`_JSONbase::recvJson()`. That literal delimiter cannot appear in command strings.
Default replies are JSON without a delimiter (`msgFinishSend: ""`); the handler
also accepts an optional trailing EOJ. It assembles replies split across the
legacy 512-byte WebSocket messages, then dispatches to `handleCmd(jCmd)` in
`js/wsCmdHandler.js`. Add page-specific reply handling there. Heartbeats
(`{"cmd":"hb"}`), replies and connection messages appear in `cmdState`.
Reply buffering is capped at 64 KiB of text and console history at 16 KiB.
All content is displayed as text, including malformed replies.

## Configuration

### Grid cell picker

The **Grid cell picker** panel below the camera controls shows the picked-cell
count and **Clear** / **Send** buttons. Left-click a cell's volume to toggle its
selection. If the ray crosses several cells, the deepest ID wins; equal-depth
hits use the nearest cell. Camera drags and multi-touch gestures do not select.
Selected cells appear as red wire boxes over the scene. Hidden grid objects are
not pickable.

Selections are independent of streamed occupancy. They remain visible and can
be deselected after a cell expires or a source disappears, including across
stream reconnects during the lifetime of the page. **Fit scene** includes visible
retained selections. **Clear** removes all selections; **Send** keeps them.

When a source's root center, root size, or maximum depth changes, selections are
remapped by their old world-space volume, allowing a selection to split into
multiple new IDs. Volumes outside the new root are clipped. Fully covered
subtrees are represented by a single cell. Boundary cells are refined to the
new maximum depth and retained if they overlap the old selection, giving the
smallest representable cover of the selected volume. A coarser new grid can
expand the covered volume, but does not erase an in-bounds selection. Remapping
has a 100,000-node work budget to keep very deep boundary cases responsive;
remaining boundary cells cover the volume at a coarser depth.

**Send** is enabled when there are selections and the independent command socket
is connected. It calls the existing `wsSendCmd()` once per selected grid source,
so grids with different root headers never share an ambiguous ID list. The JSON
contract for the future backend receiver is:

```json
{
  "cmd": "gridCellSelection",
  "module": "octGrid",
  "vPorigin": ["0", "0", "0"],
  "vRootCellSize": ["2", "2", "2"],
  "cellIDs": ["00000000000000000000000000000000"]
}
```

Each ID is exactly 32 lowercase ASCII hexadecimal characters, encoding the 16
bytes in stream order (least-significant byte first). It is **not** a hex dump
of a single big-endian integer. The example selects the root cell. Header
coordinates and full root extents are arrays of ordinary ASCII decimal strings
in metres, using locale-independent formatting. There are no color fields.
`wsSendCmd()` adds the normal `EOJ` terminator. A successful send means the command
was submitted to the socket; no backend selection receiver is implemented yet.

### Geometry sources

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
| `nCbuf` | `100000` | Maximum occupied cells collected per grid source |
| `vGeometry[].nC` | `nCbuf` | Per-grid cell limit; zero sends an empty grid |
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

World-origin pointers mark `(0, 0, 0)` with an O label and three arrows, each
exactly 1 metre long: X in red, Y in green, and Z in blue. They remain visible
when the reference grid is hidden and stay fixed to the world coordinate axes.

Positions are float32. Colors are normalized RGBA8; black source colors use the
material fallback, matching the ImGui viewer's convention. Invalid timestamps,
expired geometry, and nonfinite positions are omitted. Lines use native WebGL
line segments at one pixel wide; thick-line materials are not implemented.
Grid cells carry RGB8 (including black) and use the object opacity. Their boxes
are drawn with one shared wire box and one GPU instance per cell.

## Structure and streaming behavior

| File | Responsibility |
| --- | --- |
| `src/Net/HttpServer.*` | Static HTTP files, MIME types, request deadlines, path containment, WebSocket upgrade hook |
| `src/IO/WebSocketStream.*` | Asynchronous WebSocket sessions, shared snapshots, bounded delivery, lifecycle |
| `src/3D/Viewer/WebViewer3DProtocol.h` | Versioned little-endian binary encoding |
| `src/3D/Viewer/_WebViewer3D.*` | Framework configuration, collection, filtering and snapshot publication |
| `html/webViewer3D/js/launcher.js` | Local-file launcher and endpoint validation |
| `html/webViewer3D/js/wsStreamBase.js` | Geometry connection, protocol greeting, automatic reconnect and stream acknowledgements |
| `html/webViewer3D/js/wsCmdBase.js` | Independent JSON command socket, sending and console status |
| `html/webViewer3D/js/wsCmdHandler.js` | Reply buffering and page-specific JSON command handlers |
| `html/webViewer3D/js/protocol.js` | Frame validation and typed-array views |
| `html/webViewer3D/js/viewer3D.js` | Three.js scene, GPU buffers, camera and rendering |
| `html/webViewer3D/js/gridCellPicker.js` | Click picking, persistent red selections and command payloads |
| `html/webViewer3D/js/gridSelection.js` | Exact ID encoding and volume remapping between root headers |
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

## Binary protocol, version 2

All integers and IEEE float32 values are little-endian. A WebSocket binary
message contains one complete snapshot. Reserved fields are zero.

| Frame header offset | Type | Value |
| --- | --- | --- |
| 0 | uint32 | `0x31443357` (`W3D1`) |
| 4 | uint32 | Version `2` |
| 8 | uint32 | Sequence number, wraps at 2^32 |
| 12 | uint32 | Object count |
| 16 | uint32 | Entire message length in bytes |
| 20 | uint32 | Reserved |
| 24 | uint64 | Backend boot-clock timestamp in microseconds |

Each object starts with a 64-byte header:

| Object header offset | Type | Value |
| --- | --- | --- |
| 0, 4, 8 | uint32 × 3 | Object ID, point count P, line count L |
| 12 | uint32 | Occupied cell count C |
| 16, 20 | float32 × 2 | Point size, opacity |
| 24 | float32 × 6 | Axis-aligned bounds: min XYZ, max XYZ |
| 48 | uint32 | Flags: bit 0 means a grid header is present |
| 52–63 | bytes | Reserved |

The header is immediately followed by point XYZ (`12P` bytes), point RGBA
(`4P` bytes), line endpoint XYZ (`24L` bytes, A then B), and line endpoint RGBA
(`8L` bytes). All sections remain four-byte aligned. Each point costs 16 bytes;
each line costs 32 bytes. When flag bit 0 is set, these sections are followed by
a 40-byte grid header, including when C is zero:

| Grid header offset | Type | Value |
| --- | --- | --- |
| 0 | float32 × 3 | Root cell center XYZ (`vPorigin`) |
| 12 | float32 × 3 | Full root cell extents XYZ (`vRootCellSize`) |
| 24 | uint32 | Maximum depth, 0–40 |
| 28 | uint32 | Reserved |
| 32 | uint64 | Grid snapshot publication timestamp in microseconds |

Next come C interleaved records: 16 ID bytes, then three RGB8 bytes. The ID is
little-endian, low 64-bit word first. Its top two bits are zero; bits 125–6
hold up to 40 child indices, three bits per level, starting at bit 123. Bits
5–0 hold the cell depth. Depth 0 identifies the root (ID zero); only `depth`
child segments are used and all remaining path bits are zero. A child index
uses X/Y/Z masks 4/2/1, with a set bit choosing the positive half of that axis.

Each cell costs exactly **19 bytes**, down from 384 bytes for twelve streamed
lines (about 95% less cell payload). Records have no per-cell padding. Zero to
three zero bytes follow the whole cell section to align the next object to four
bytes. Header presence is explicit so an empty grid can clear old boxes.

The browser accepts legacy version 1 point/line frames as well as version 2.
Point and line sections retain their original encoding. Older browsers must be
updated alongside the version 2 backend. Full snapshots preserve reconnect and
object removal behavior.

## Occupied cell interface

`_OctreeGrid::get(OCTGRID_CELLS*, tExpire, nMaxCells)` copies one coherent header
and its occupied cell records. It includes occupied ancestors, as the previous
wireframe did, in root-first traversal order. `nMaxCells` in the grid config sets
the publication cap; if omitted, `floor(nMaxLines / 12)` preserves the old cap
(default 8333). The viewers have independent `nCbuf` and per-object `nC` caps.
Point and line interfaces remain available; the grid itself returns no lines.

RGB comes from each cell's averaged point color, clamped and rounded to RGB8.
An explicitly configured `vColCellOcc` keeps the previous uniform color override.
The sample omits this setting so cell colors follow the point cloud. Remove it
from existing configurations to enable per-cell colors. For grid cells, `matCol`
controls only opacity; its RGB does not tint cells. The box shader uses the same
output color conversion as the point-cloud material.
`dTexpireCell` still removes old occupancy; viewer expiry uses the shared snapshot
publication time. An empty snapshot replaces previous boxes.

Both viewers retain the full ID and box geometry. Browser `GridBoxes.getCell(i)`
returns the ID bytes, RGB, and a `THREE.Box3` for an instance; ImGui keeps an
`IMGUI_VIEWER_BOX` with UUID, center, size, and color. The browser picker uses
these records for selection and command sending. Rendering positions remain float32, so very deep cells can become
visually indistinguishable even though their 128-bit IDs remain exact.

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
rendering, protocol rejection, camera/visibility controls, and Stop/Start. A
separate command fixture emulates `_WSconsole` framing to check JSON + EOJ sends,
split replies, bounded console history and isolation between command and stream
connections:

```bash
python3 test/webViewer3D/browser.py /tmp/openkai-webviewer-tests/viewer_fixture html/webViewer3D
```

The browser test writes `/tmp/openkai-webviewer.png`. There are no npm or Python
package dependencies. Vendor provenance and hashes are recorded in
`html/webViewer3D/vendor/README.md`.

To verify the real framework collector with the sample point cloud and octree:

```bash
python3 test/webViewer3D/backend.py /path/to/build-web/OpenKAI
python3 test/webViewer3D/octree.py /path/to/build-web/OpenKAI
```

For a completed build with ImGui and `CMAKE_EXPORT_COMPILE_COMMANDS=ON`, run the
native grid/viewer API checks (ID lookup and stability through depth 40, colors,
caps, expiry, and clearing):

```bash
python3 test/webViewer3D/native_cells.py build
```
