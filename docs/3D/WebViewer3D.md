# Browser selectable octree grid viewer

`_WebSelectableOctGrid` derives from `_GeometryViewerBase` and reads points and lines from
the same `_GeometryBase::get()` ring buffers as `_ImGUIselectableOctGrid`, plus compact
`_SelectableOctGrid::get(OCTGRID_CELLS*)` snapshots. The C++ process
serves the browser application and three independent binary WebSockets on one port. A separate
WebSocket connects JSON application commands to `_WSconsole`. All browser
assets, including a pinned three.js release, are in `html/viewer/_SelectableOctGrid/`.
There is no browser-side installation, npm build, CDN, or separate web server.

The viewer and its `WebSelectableOctGridProtocol.h` header live in
`src/UI/Viewer/Web/`. Its name matches the `_SelectableOctGrid` backend module
and the native viewer `_ImGUIselectableOctGrid` in `src/UI/Viewer/ImGUI/`.

Configure displayed grids with `"class": "_SelectableOctGrid"`. This module
inherits the `_OctreeGrid` calculation API and the `_ModuleBase` lifecycle, and
owns viewer snapshots, selection state, saved selections and interaction commands.
`_OctreeGrid` only ingests points, calculates occupancy, looks up cells and expires
old cells; it does not publish viewer snapshots or handle grid commands.
Existing module instance names such as `octGrid` and command names stay the same.

## Build and run

Add `-DWITH_UNIVERSE=ON` to your existing CMake configuration. A minimal build is:

```bash
cmake -S . -B build-web -DWITH_UNIVERSE=ON -DCMAKE_BUILD_TYPE=Release
cmake --build build-web -j4
./build-web/OpenKAI jsonCfg/WebViewer3D.json
```

`WITH_UNIVERSE` includes the browser viewer and requires Boost headers version 1.70
or later in addition to OpenKAI's normal build dependencies. Beast and Asio are
compiled from headers;
no Boost runtime library, wsServer, Open3D, ImGui, or desktop GL backend is needed
for streaming. The current sample uses `_Scepter` camera geometry and octree
cells; enable its camera build dependencies when running that configuration.
The optional `_PCfile` source can load `data/PointCloud/StanfordBunny/bun000.ply`.
Run from the repository root so relative data paths resolve.

Open `html/viewer/_SelectableOctGrid/index.html` directly in the browser, enter the backend IP
and port (default `8080`), and click **Start**. The local launcher navigates to
the C++ server and automatically connects. Alternatively, visit
`http://BACKEND_IP:8080/` and click **Start** there. **Start** opens the three geometry sockets
and the command socket independently; **Stop** closes all four. The Commands section
under Objects has its own Connect/Disconnect buttons and a read-only `cmdState`
console. A command connection failure leaves geometry streaming active.

The launcher redirects because browsers restrict ES modules loaded from
`file://`; the renderer and all imports run from the backend's HTTP origin.
See the [three.js installation guide](https://threejs.org/manual/en/installation.html).

CMake copies the frontend next to the executable as `html/viewer/_SelectableOctGrid/` after
building and installs it under `bin/html/viewer/_SelectableOctGrid/`. By default the viewer
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
and `WITH_PROTOCOL` in addition to your application's build options.
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
512-byte WebSocket messages, then dispatches to `handleCmd(jCmd)` in
`js/wsCmdHandler.js`. Add page-specific reply handling there. Heartbeats
(`{"cmd":"hb"}`), replies and connection messages appear in `cmdState`.
Reply buffering is capped at 8 MiB of text and console history at 16 KiB.
All content is displayed as text, including malformed replies.

## Configuration

### Grid config

The **Grid config** form below **Point scale** sets the root center and full
root cell size in metres. Origin X/Y/Z default to 0/0/0; size X/Y/Z default to
5/5/5. These are input values only: incoming stream headers never overwrite them.

**Update** is enabled after a grid header arrives and the command socket connects.
It uses `wsSendCmd()` once per known grid module, with decimal-string coordinates:

```json
{
  "cmd": "setGridConfig",
  "module": "octGrid",
  "vPorigin": ["0", "0", "0"],
  "vRootCellSize": ["5", "5", "5"]
}
```

`_SelectableOctGrid::console()` validates all six values before applying them. Coordinates
must be finite float32 values and sizes must be positive. A changed root clears
old occupancy, publishes the new header with an empty snapshot, and rebuilds from
point sources on subsequent updates. Root changes are synchronized with the grid
worker. An unchanged root preserves existing cells.

Backend selections are remapped to preserve their world-space volume, using the
same clipping and bounded boundary refinement as the browser picker. The browser
remaps its own selections when the new stream header arrives. The backend replies
with `{"cmd":"setGridConfig","module":"octGrid","bSuccess":true}` after applying
the change; invalid requests return `bSuccess: false` and leave the grid unchanged.
The form displays the result. Update changes runtime configuration; the picker's
**Send** continues to save the current root and selections through `fConfig`.

### Grid drawing mode

The **Wire-frame / Solid** switch beneath Grid changes occupied-cell drawing
immediately. Wire-frame is the default. Solid uses unlit filled box faces with
each cell's RGBA color and alpha multiplied by object opacity. Both modes use
instancing and the same level range. The mode persists through stream updates
and reconnects; selected cells keep their red wire outlines in either mode.

Transparent solid cells are sorted by camera depth when the geometry or camera
changes. Sorting entire boxes approximates blending; nested or intersecting
surfaces can still show overlap artifacts. A narrow level range reduces both
overlap and rendering cost. Opaque boxes use depth writes. The stream remains
20 bytes per cell in either mode.

### Grid level range

The **Grid levels** Min and Max sliders below Point scale select an inclusive
range from 0 to 40. The default is 0–40. Changes apply immediately to occupied
cells in every grid and persist through stream updates and reconnects. Moving
one endpoint past the other moves the other endpoint to keep a valid interval.
Cells outside the interval are omitted from drawing and picking. Selected red
boxes remain visible and can still be deselected, regardless of their level.

### Grid cell picker

The **Grid cell picker** panel below the camera controls shows the picked-cell
count and **Load** / **Clear** / **Send** buttons. Left-click a cell's volume to toggle its
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
command received by `_SelectableOctGrid::console()` through `_WSconsole` is:

```json
{
  "cmd": "octGridCellSelect",
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
was submitted to the socket. The backend replies with
`{"cmd":"octGridCellSelect","bSuccess":true}` after replacing `m_vSelectedCells`.
It decodes the decimal header strings back to float32 and requires the root
origin and size to match the current grid; a missing, malformed or different
header clears the backend selection and returns `bSuccess: false`. Invalid ID
lists also return false, leaving the previous selection intact. IDs must be
canonical and within the grid's maximum level, but need not remain occupied.
An empty ID array with a matching header clears the backend selection.

**Load** is enabled when the command socket is connected and a grid header has
arrived, even if the grid has no occupied or selected cells. It calls `wsSendCmd()`
once per known grid module:

```json
{"cmd":"loadCellSelect","module":"octGrid"}
```

The backend snapshots its current root and `m_vSelectedCells` and replies:

```json
{
  "cmd": "cellSelect",
  "module": "octGrid",
  "nMaxLevel": 40,
  "vPorigin": ["0", "0", "0"],
  "vRootCellSize": ["2", "2", "2"],
  "cellIDs": ["00000000000000000000000000000000"]
}
```

`wsCmdHandler.js::handleCmd()` routes this reply to the picker. Valid IDs are
added to that module's existing selection, ignoring duplicates. An empty reply
keeps existing selections. If the returned root differs from the current stream
header, the same volume-remapping rules above apply before merging; IDs deeper
than the current maximum level are also remapped. Invalid replies leave the
selection unchanged. Loaded cells retain red outlines even when unoccupied or
outside the displayed level range.

### Saving selected cells

`_SelectableOctGrid::saveConfig(j, fName)` writes the root and selected IDs under
`_SelectableOctGrid`, preserving other top-level sections already present in `j`:

```json
{
  "_SelectableOctGrid": {
    "vPorigin": [0, 0, 0],
    "vRootCellSize": [2, 2, 2],
    "vSelectedCells": ["00000000000000000000000000000000"]
  }
}
```

The file uses numeric coordinate arrays and the same exact hexadecimal ID byte
order as picker commands. `loadConfig(pJ, fName)` restores this state and optionally
returns the complete parsed document through `pJ`. Both methods use the module's
`fConfig` setting when `fName` is empty. Initialization loads that file after the
grid's root and maximum level are configured.
Only the `_SelectableOctGrid` section is accepted for saved selections.

The `octGridCellSelect` handler calls `saveConfig()` after updating the selection.
Configure a writable `octGrid.fConfig` path to retain it across backend restarts.
After reopening the page, **Load** retrieves the backend's current selection,
including selections restored from that file during startup. The button does
not reread the file or change the running backend's root or occupancy.

Load explicitly after initialization with grid updates stopped. A restored root
change clears occupancy tied to the previous root and updates the published grid
header. All fields and IDs are validated before applying changes; a failed load
returns false and preserves the current root and selections. IDs must fit the
configured `nMaxLevel`. An empty `vSelectedCells` array is saved and restored as
an empty selection. File read/write failures also return false.

### Viewer sources

Configure two separate source arrays. `vGeometry` entries retain `_GeometryBase*`
for point/line collection; `vSelectableOctGrid` entries retain `_SelectableOctGrid*`
for cell snapshots. Both viewers use the same source parser and settings:

```json
{
  "viewer": {
    "class": "_WebSelectableOctGrid",
    "thread": { "FPS": 30 },
    "host": "0.0.0.0",
    "port": 8080,
    "nPbuf": 200000,
    "nLbuf": 100000,
    "nCbuf": 100000,
    "vCamEye": [0, -8, 4],
    "vCamLookAt": [0, 0, 0],
    "vCamUp": [0, 0, 1],
    "vGeometry": [
      { "_GeometryBase": "points", "nP": 200000, "nL": 0, "matPointSize": 2 },
      { "_GeometryBase": "lines", "nP": 0, "nL": 100000, "matCol": [0.3, 0.8, 1] }
    ],
    "vSelectableOctGrid": [
      { "_SelectableOctGrid": "octGrid", "nC": 100000, "matCol": [1, 1, 1, 0.5] }
    ]
  }
}
```

Each source appears once. Wrong provider types, duplicate names, unsupported
entry settings, and missing modules fail linking. Explicitly disabled modules
are skipped. Per-source limits are capped by the viewer's corresponding buffer
limit; zero disables that output. A zero cell limit still publishes its root header.
The removed `vReferenceFrame`, viewer `vGeometryBase`, `geometry`, and
`_ReferenceFrame` entry aliases are rejected. Per-source caps use `nP`, `nL`,
and `nC`, with no `nPbuf`/`nLbuf`/`nCbuf` entry aliases.

A calculation-only `_OctreeGrid` does not publish viewer snapshots; use
`_SelectableOctGrid`. The grid's own `vGeometryBase` input list still selects
its point clouds.

| Setting | Default | Meaning |
| --- | --- | --- |
| `host`, `port` | `0.0.0.0`, `8080` | HTTP and WebSocket bind address and shared port |
| `webRoot` | `html/viewer/_SelectableOctGrid` | Directory of browser resources |
| `nClientMax` | `8` | Maximum clients per geometry endpoint, configurable from 1 to 64 |
| `thread.FPS` | framework default | Maximum geometry collection/publication rate |
| `nCbuf` | `100000` | Maximum occupied cells collected per grid source |
| `vSelectableOctGrid[].nC` | `nCbuf` | Per-grid cell limit; zero sends an empty grid |
| `nPbuf`, `nLbuf` | `200000`, `100000` | Scratch capacities per source; zero disables collection of that type |
| `dTexpire` | `0` | Maximum geometry age in nanoseconds; zero disables expiry |
| `bAutoBound`, `bShowGrid` | `true`, `true` | Fit the first nonempty frame of each type; show reference grid |
| `vBgCol` | `[0.035,0.045,0.065,1]` | Background color |
| `vGeometry[].nP`, `.nL` | scratch capacities | Per-object limits; zero omits that primitive type |
| `matPointSize`, `matCol` | `2`, `[1,1,1,1]` | Point size in pixels, fallback RGB; fourth component controls cell opacity only |
| `bVisible` | `true` | Exclude an object from streaming when false |

Camera configuration uses the base keys `camProjType` (0 perspective, 1
orthographic), `camFov`, `vCamNF`, `vCamLR`, `vCamBT`, `vCamEye`, `vCamLookAt`, and
`vCamUp`. It is sent when a browser connects. Camera movement, reset, visibility,
grid, and point scale are local to each browser. Automatic fitting runs once
on the first nonempty frame of each type after Start, fitting the union of available
geometry. Individual stream reconnects preserve the camera. **Fit scene** can be
used subsequently.

World-origin pointers mark `(0, 0, 0)` with an O label and three arrows, each
exactly 1 metre long: X in red, Y in green, and Z in blue. They remain visible
when the reference grid is hidden and stay fixed to the world coordinate axes.

Positions are float32. Point and line colors are normalized RGB8 and render
opaque; black source colors use the material RGB fallback, matching the ImGui
viewer. PLY alpha is ignored, including the alpha byte in packed `rgba` colors.
Invalid timestamps, expired geometry, and nonfinite positions are omitted.
Lines use native WebGL line segments at one pixel wide; thick-line materials
are not implemented. Grid cells retain RGBA8 (including black), with per-cell
alpha multiplied by cell object opacity (`matCol[3]`). Alpha 0 is invisible,
1 is opaque, and intermediate values blend with the scene. Their boxes use
shared wire/solid geometry and one GPU instance per cell.

## Structure and streaming behavior

| File | Responsibility |
| --- | --- |
| `src/Net/HttpServer.*` | Static HTTP files, MIME types, request deadlines, path containment, WebSocket upgrade hook |
| `src/IO/WebSocketStream.*` | Asynchronous WebSocket sessions, shared snapshots, bounded delivery, lifecycle |
| `src/UI/Viewer/Web/WebSelectableOctGridProtocol.h` | Versioned little-endian binary encoding |
| `src/UI/Viewer/SelectableOctGridSources.*` | Shared source configuration and separate typed geometry/grid lists |
| `src/UI/Viewer/Web/_WebSelectableOctGrid.*` | Framework configuration, collection, filtering and snapshot publication |
| `html/viewer/_SelectableOctGrid/js/launcher.js` | Local-file launcher and endpoint validation |
| `html/viewer/_SelectableOctGrid/js/wsStreamBase.js` | Geometry connection, protocol greeting, automatic reconnect and stream acknowledgements |
| `html/viewer/_SelectableOctGrid/js/wsCmdBase.js` | Independent JSON command socket, sending and console status |
| `html/viewer/_SelectableOctGrid/js/wsCmdHandler.js` | Reply buffering and page-specific JSON command handlers |
| `html/viewer/_SelectableOctGrid/js/protocol.js` | Frame validation and typed-array views |
| `html/viewer/_SelectableOctGrid/js/viewer3D.js` | Three.js scene, GPU buffers, camera and rendering |
| `html/viewer/_SelectableOctGrid/js/gridCellPicker.js` | Click picking, persistent red selections and command payloads |
| `html/viewer/_SelectableOctGrid/js/gridSelection.js` | Exact ID encoding and volume remapping between root headers |
| `html/viewer/_SelectableOctGrid/js/main.js` | UI, animation loop and render acknowledgements |

HTTP and WebSocket IO run on one asynchronous worker. Geometry collection runs
on a separate owned thread, stopped and joined before its resources are released.
The network thread never calls geometry providers. Providers retain responsibility
for synchronizing concurrent writes with their `get()` methods, as with ImGui.

Geometry uses three endpoints on the HTTP port:

| Endpoint | Binary contents |
| --- | --- |
| `/stream/points` | Point positions and RGB8 |
| `/stream/lines` | Line endpoints and RGB8 |
| `/stream/cells` | Octree root headers and 20-byte cell records |

Each endpoint owns its latest snapshot, sequence counter, buffer pool and clients.
The frontend owns a separate connection, pending frame and reconnect timer per
type. All three update the same scene and canvas. Geometry sources can contribute
points and lines; selectable grids contribute cells. Object IDs are consistent
across connections. A failed,
missing, or empty stream does not prevent the other types from drawing. Stream
status is shown separately when their connection states differ.

Each connection receives a JSON `hello` with `version: 5`, its `stream` name
(`points`, `lines`, or `cells`), all object names and camera settings. Any type's
hello can initialize the page; later greetings do not reset the scene or camera.
Each object also has a `selectableGrid` boolean, true for `_SelectableOctGrid`
sources. The browser enables picking and routes grid commands only to those sources.
The client sends `start` to subscribe. One complete snapshot of that type is then
sent; the browser sends `next` on that same socket after uploading and rendering
it. Each socket has its own credit, so withholding point ACKs does not block cell
or line delivery. While a browser is busy, the backend replaces the latest
snapshot and discards intermediate versions. There is no growing FIFO of old
frames. Each socket has at most one active write; clients of the same type share
immutable frame storage. `pause` disables that socket's delivery; `start` resumes
it. Beast handles WebSocket framing, ping/pong and close.

Collection of a type is skipped when its endpoint has no clients. Scratch vectors,
snapshot buffers and browser GPU arrays are reused. Position/color sections
decode as views into the received ArrayBuffer; the renderer copies them into
reusable GPU upload arrays and updates only occupied ranges. Empty objects clear
that type's geometry; absent objects clear that component. A disconnect clears
only its type. The geometry source's other component and retained grid selections remain.
The browser allocates point/line objects for geometry sources and box instances
for grid sources. It rejects a stream whose type conflicts with its source.
Resources are released when the source's last stream disappears; source visibility
and selected grid cells survive reconnects.
Each type's configured worst-case snapshot is limited to 64 MiB; the viewer supports
1024 sources. The byte lengths below apply independently to each stream.

For a cells-only viewer, set `nPbuf` and `nLbuf` to zero and include the grid source:

```json
"nPbuf": 0,
"nLbuf": 0,
"nCbuf": 100000,
"vSelectableOctGrid": [{ "_SelectableOctGrid": "octGrid", "nC": 100000 }]
```

The point/line endpoints then send empty snapshots. The grid can still consume
point data from its own sources; these viewer limits affect only output.

Geometry transport uses `WebSocketStream` under `src/IO`, independently of
the command socket and the `USE_WSSERVER` option.

The embedded server provides plain HTTP/WS for local or trusted-network use. It
does not implement TLS or authentication. To expose it through HTTPS, use a TLS
endpoint that also forwards `/stream/points`, `/stream/lines` and `/stream/cells`
WebSocket upgrades. The browser selects
`wss://` when the page is served over HTTPS.

## Binary protocol, version 6

All integers and IEEE float32 values are little-endian. Each WebSocket binary
message contains one complete snapshot of exactly one geometry type. An empty
stream is a 32-byte frame with an object count of zero. Reserved fields are zero.

| Frame header offset | Type | Value |
| --- | --- | --- |
| 0 | uint32 | `0x36443357` (`W3D6`) |
| 4 | uint32 | Version `6` |
| 8 | uint32 | Type: 1 points, 2 lines, 3 cells; must match the endpoint |
| 12 | uint32 | Sequence number for this stream, wraps at 2^32 |
| 16 | uint32 | Object count |
| 20 | uint32 | Entire message length in bytes |
| 24 | uint64 | Backend monotonic-clock timestamp in nanoseconds |

Each object starts with a 40-byte header:

| Object header offset | Type | Value |
| --- | --- | --- |
| 0 | uint32 | Source object ID, shared across the three streams |
| 4 | uint32 | Record count N for this geometry type |
| 8, 12 | float32 × 2 | Point size (1 for lines/cells), cell opacity (must be 1 for points/lines) |
| 16 | float32 × 6 | Axis-aligned bounds: min XYZ, max XYZ |

The payload immediately follows the object header:

- **Points:** XYZ float32 (`12N` bytes), then RGB8 (`3N` bytes).
- **Lines:** endpoint XYZ float32 (`24N` bytes, A then B), then endpoint RGB8
  (`6N` bytes, A then B).
- Point/line payloads end with 0–3 zero padding bytes to reach a four-byte
  boundary. Padding is per object, not per vertex, including the last object.
- **Cells:** the 40-byte grid header below, then N interleaved 20-byte cell records.
  The grid header is always present, including when N is zero, so an empty grid
  can update its root information and remap retained selections.

There are no mixed-type object sections, flags or optional geometry headers.
All sections and subsequent object headers remain four-byte aligned.

| Grid header offset | Type | Value |
| --- | --- | --- |
| 0 | float32 × 3 | Root cell center XYZ (`vPorigin`) |
| 12 | float32 × 3 | Full root cell extents XYZ (`vRootCellSize`) |
| 24 | uint32 | Maximum depth, 0–40 |
| 28 | uint32 | Reserved |
| 32 | uint64 | Grid snapshot publication timestamp in nanoseconds |

Next come C interleaved records: 16 ID bytes, three RGB8 bytes, and one alpha byte
(0–255). The ID is
little-endian, low 64-bit word first. Its top two bits are zero; bits 125–6
hold up to 40 child indices, three bits per level, starting at bit 123. Bits
5–0 hold the cell depth. Depth 0 identifies the root (ID zero); only `depth`
child segments are used and all remaining path bits are zero. A child index
uses X/Y/Z masks 4/2/1, with a set bit choosing the positive half of that axis.

Each cell costs exactly **20 bytes**. Cell records and sections are four-byte
aligned without padding.

Only version 6 is supported, with an exact version match required for both
the JSON greeting and binary frames. There is no protocol negotiation or
conversion. Update the backend and browser assets together.
Point and line payloads use 15 and 30 bytes per record respectively, plus
per-object alignment padding. Full snapshots preserve reconnect and object
removal behavior. Picker commands send 16-byte IDs as hexadecimal strings.

## Occupied cell interface

`_SelectableOctGrid::get(OCTGRID_CELLS*, tExpire, nMaxCells)` copies one coherent header
and its occupied cell records. It includes occupied ancestors, as the previous
wireframe did, in root-first traversal order. `nMaxCells` in the grid config sets
the publication cap (default 8333). The old `nMaxLines` setting is rejected. The viewers have independent `nCbuf` and per-object `nC` caps.
`_OctreeBase` derives from `_ReferenceFrame`. Neither it nor its grid subclasses
expose `_GeometryBase::get()` or a geometry type. `_OctreeGrid` still consumes
point clouds through its `vGeometryBase` input list; `_SelectableOctGrid` publishes
only cell IDs, colors, and the root header. Viewers construct the boxes.

Cells average point RGB, initializing cell alpha to 1, then publish their retained
RGBA color clamped and rounded to RGBA8.
An explicitly configured `vColCellOcc` keeps the previous uniform color override.
It accepts `[r,g,b,a]`; a three-component override defaults to alpha 1.
The sample omits this setting so cell colors follow the point cloud. Remove it
from existing configurations to enable per-cell colors. For grid cells, `matCol`
scales the per-cell alpha; its RGB does not tint cells. The box shader uses the same
output color conversion as the point-cloud material.
`dTexpireCell` still removes old occupancy; viewer expiry uses the shared snapshot
publication time. An empty snapshot replaces previous boxes.

Both viewers retain the full ID and box geometry. Browser `GridBoxes.getCell(i)`
returns the ID bytes, RGBA, and a `THREE.Box3` for an instance; ImGui keeps an
`IMGUI_VIEWER_BOX` with UUID, center, size, and color. The browser picker uses
these records for selection and command sending. Rendering positions remain float32, so very deep cells can become
visually indistinguishable even though their 128-bit IDs remain exact.

## Verification

The standalone transport suite needs Boost headers, Eigen 5, glog, CMake,
a C++17 compiler, and Python 3. It exercises independent typed channels,
endpoint validation, real loopback sockets, path traversal (including
symlinks), large and fragmented frames, multiple peers, ACK flow control,
pause/resume, ping/pong, and shutdown with live connections.

```bash
cmake -S test/webViewer3D -B /tmp/openkai-webviewer-tests
cmake --build /tmp/openkai-webviewer-tests -j2
ctest --test-dir /tmp/openkai-webviewer-tests --output-on-failure
```

An optional Chrome/Chromium test also checks the local HTML launcher, real WebGL2
rendering, cells-only startup, per-stream disconnect/error isolation, protocol
rejection, camera/visibility controls, alpha, solid boxes, picking and Stop/Start. A
separate command fixture emulates `_WSconsole` framing to check JSON + EOJ sends,
split replies, bounded console history and isolation between command and stream
connections:

```bash
python3 test/webViewer3D/browser.py /tmp/openkai-webviewer-tests/viewer_fixture html/viewer/_SelectableOctGrid
```

The browser test writes `/tmp/openkai-webviewer.png`. There are no npm or Python
package dependencies. Vendor provenance and hashes are recorded in
`html/viewer/_SelectableOctGrid/vendor/README.md`.

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

When deliberately retaining a temporary cell-alpha override in the grid publication path,
pass `--cell-alpha 0.5` (or its actual value) to the backend, octree and native
test scripts. Their defaults expect alpha 1 for cells built from RGB points.
