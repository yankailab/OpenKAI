# OpenKAI _SelectableOctGrid viewer

Open `index.html`, enter the C++ backend's IP and stream port, then click **Start**.
Streaming uses port `8080` by default; the independent command connection uses
port `7890`, editable in the geometry panel. **Start** connects all geometry and command sockets; **Stop**
disconnects them. The Commands panel also has independent Connect/Disconnect
buttons and a bounded `cmdState` console.
The backend serves this directory and mounts `../../common/` at `/common/`.
Keep that relative layout when copying the viewer. No npm, CDN, or extra web server is
required. The full setup, configuration and wire format are documented in
[`docs/3D/WebViewer3D.md`](../../../docs/3D/WebViewer3D.md).

- `css/`: viewer-specific styles; shared theme and layout are in `../../common/css/`
- `../../common/js/geometryConnection.js`: one independent connection and flow control per geometry type
- `../../common/js/wsCmdBase.js`: command connection, `wsSendCmd()` and console status
- `js/wsCmdHandler.js`: JSON reply assembly and page-specific `handleCmd()`
- `../../common/js/viewerLauncher.js`: local-file launcher and endpoint validation
- Other `js/` files: binary decoder, renderer and application wiring
- `vendor/`: pinned three.js and OrbitControls, with their license

Keep application commands out of the stream transport. Add UI controls that call
`wsSendCmd({ cmd: 'test', module: 'tester', v: 0 })` and handle their replies in
`handleCmd(jCmd)`. This preserves `_JSONcmd`'s `JSON + EOJ` request scheme.
The command server and stream wiring are included in `jsonCfg/WebViewer3D.json`;
the grid instance must appear in the command module's `vBASE` list.

Use `"class": "_SelectableOctGrid"` for grids displayed by this viewer.
Octree grids derive from `_ModuleBase`, retain an optional `_TransformRigid` link,
and publish cell snapshots only.
Configure separate viewer lists:

```json
"vGeometry": [{ "PCLframeIn": "points", "nP": 200000, "nL": 0 }],
"vSelectableOctGrid": [{ "_SelectableOctGrid": "octGrid", "nC": 100000 }]
```

Generic reference-frame entries and old viewer name lists are no longer accepted.
A grid's `vPCLframesIn` list names the independent point-cloud streams it consumes.
Viewer geometry entries use `PCLframeIn` and/or `LineFrameIn` stream names; they
never resolve producer modules. Each publication replaces the corresponding
geometry, and an empty publication clears it. Encoded point/line snapshots are
reused until their source changes or records expire.
It inherits `_OctreeGrid` calculations and adds cell snapshots, selection persistence,
and interaction commands. `_OctreeGrid` itself only calculates occupancy.
The cells handshake identifies interactive sources with `selectableGrid: true`;
the picker and Grid config controls target those sources.

Occupied octree cells stream as 16-byte IDs plus RGBA8 (20 bytes per cell). The browser reconstructs
instanced wire or solid boxes and retains IDs and bounds for cell picking. The
Wire-frame / Solid switch below Grid enables filled faces with per-cell alpha;
selected cells keep their red outlines. See the
[version 7 format](../../../docs/3D/WebViewer3D.md#binary-protocol-version-7).
IDs place two zero reserved bits first, then six depth bits, then forty
three-bit child indices. Stream and saved-JSON IDs remain little-endian, so
their final byte holds the reserved bits and depth. Saved selections must
already use this layout.
The backend and browser must both use version 7; there is no protocol negotiation
or conversion. Points and lines use opaque RGB8. Only cell alpha multiplies
object opacity.

The **Grid config** form below Point scale loads the connected grid's root origin
and size. **Grid levels config** adds `dTexpireCell`, `dTexpirePCL`, `nMaxCells`,
`nMaxLevel`, and `nPminBuild`, using the same names and units as the launch JSON.
Expiration durations are nanoseconds; zero disables expiration. `nPminBuild` is
an exclusive point-count threshold: `1` publishes cells with at least two points.
Choose the source when more than one grid is connected. **Load** reloads its
current runtime settings through `getGridConfig`; stream frames leave edits alone.
**Apply** sends all seven settings together through `setGridConfig` and changes
only the running grid. **Save** sends `saveConfig` to persist its current settings
and selections in the original launch JSON. Apply edits before saving; Save leaves
unapplied form values untouched. Root or depth changes rebuild occupancy and
remap running selections within the new root. Only the complete command format is accepted.
The **Grid levels show range** sliders filter drawing and picking locally; they
do not change the configured `nMaxLevel` or saved JSON.

The **Transform** panel targets a `_TransformRigid` instance by its backend name;
the default `scepterTransform` matches `samples/AProver_LiDAR/AProver_LiDAR.json`.
The target must be listed in the command module's `vBASE`, as it is in that sample.
Connecting the command socket or changing the target loads its local translation
and roll/pitch/yaw. Translation is in metres and rotation is displayed in degrees;
commands send angles in radians. **Load** reloads the current backend values.
**Apply** changes all six running values together. **Save** separately writes the
applied `vT` and quaternion `vR` to that instance's original JSON file, leaving
unapplied form edits alone. A save error leaves the running transform unchanged
and reports the failure; the JSON must be writable. The panel uses the
existing `getConfig`, `setConfig`, and `saveConfig` commands on the command socket.
Linked RGBD producers apply the combined transform, including any parent, to new
point clouds before publishing them.

The **Grid cell picker** panel provides **Load**, **Clear**, **Apply**, and **Save**.
Load replaces each source's local selections with its current backend selections,
remapping volumes when root headers differ. Clear removes the local selections;
Apply replaces the backend selections, including an empty list after Clear,
without writing JSON. Save persists the currently applied selections together
with that grid's current settings; it does not apply local picks. Commands use
`getCellSelection`, `setCellSelection`, and `saveConfig`, with correlated replies.
The file must be writable. Startup reads the saved settings and selections on
the next launch. See the
[picker behavior and JSON contract](../../../docs/3D/WebViewer3D.md#grid-cell-picker).

Geometry uses `/stream/points`, `/stream/lines` and `/stream/cells` on the same
HTTP port, with independent snapshots, acknowledgements and reconnects. All
available types render on the same canvas; missing or failed streams affect only
their own geometry. Cells-only viewers can set both `nPbuf` and `nLbuf` to zero.

## Checks

The native grid regression covers dense 640×480 clouds with a 300 ms cell expiry
to catch processing delays, cell IDs and colors, repeated-frame handling, and
stale-input rejection:

```sh
python3 test/run_octree_grid_tests.py build
python3 test/run_transform_rigid_tests.py build
```

With Chrome/Chromium installed, exercise separate Load/Apply/Save behavior for
grid settings, transforms and selections, exact 64-bit expiration values,
validation, and saved-config restart without sensor hardware:

```sh
python3 test/UI/SelectableGridConfig.py build/OpenKAI
```

With the Scepter connected and other camera applications stopped, check the
actual browser's occupied cells, mouse picking, Apply/Save, and Clear/Load:

```sh
python3 test/UI/SelectableGridLive.py build/OpenKAI --ip 192.168.16.101
```

The live check requires Chrome/Chromium. It uses temporary ports and a camera/grid
launch configuration without rover modules; Save writes only the temporary JSON.
Results are saved to `/tmp/openkai-grid-live.log` and `/tmp/openkai-grid-live.png`.
