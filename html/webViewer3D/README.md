# OpenKAI webViewer3D

Open `index.html`, enter the C++ backend's IP and stream port, then click **Start**.
Streaming uses port `8080` by default; the independent command connection uses
port `7890`, editable in the geometry panel. **Start** connects all geometry and command sockets; **Stop**
disconnects them. The Commands panel also has independent Connect/Disconnect
buttons and a bounded `cmdState` console.
The backend serves this directory itself. No npm, CDN, or extra web server is
required. The full setup, configuration and wire format are documented in
[`docs/3D/WebViewer3D.md`](../../docs/3D/WebViewer3D.md).

- `css/`: viewer styles
- `js/wsStreamBase.js`: one independent connection and flow control per geometry type
- `js/wsCmdBase.js`: command connection, `wsSendCmd()` and console status
- `js/wsCmdHandler.js`: JSON reply assembly and page-specific `handleCmd()`
- Other `js/` files: launcher, binary decoder, renderer and application wiring
- `vendor/`: pinned three.js and OrbitControls, with their license

Keep application commands out of the stream transport. Add UI controls that call
`wsSendCmd({ cmd: 'test', module: 'tester', v: 0 })` and handle their replies in
`handleCmd(jCmd)`. This preserves `_WSconsole`'s `JSON + EOJ` request scheme.
An optional backend configuration is in `jsonCfg/WebViewer3D_commands.json`;
include it from your application's `APP.vInclude` when using commands.

Occupied octree cells stream as 16-byte IDs plus RGBA8 (20 bytes per cell). The browser reconstructs
instanced wire or solid boxes and retains IDs and bounds for cell picking. The
Wire-frame / Solid switch below Grid enables filled faces with per-cell alpha;
selected cells keep their red outlines. See the
[version 4 format](../../docs/3D/WebViewer3D.md#binary-protocol-version-4). Only
version 4 is supported. Point, line, and cell alpha multiplies object opacity.

The **Grid config** form below Point scale sends root origin and size through
`setGridConfig` on the command socket. Its defaults are 0/0/0 m and 5/5/5 m, and
stream updates leave the inputs unchanged. Updating the root rebuilds occupancy
and preserves selected volumes within the new root.

The **Grid cell picker** panel supports persistent red selections, Load, Clear,
and Send. Load retrieves backend selections over the independent command socket
and merges them without duplicate IDs, remapping volumes when root headers differ.
Set the grid module's `fConfig` to persist selections across backend restarts. See the
[picker behavior and JSON contract](../../docs/3D/WebViewer3D.md#grid-cell-picker).

Geometry uses `/stream/points`, `/stream/lines` and `/stream/cells` on the same
HTTP port, with independent snapshots, acknowledgements and reconnects. All
available types render on the same canvas; missing or failed streams affect only
their own geometry. The old combined `/stream` endpoint is removed. Cells-only
viewers can set both `nPbuf` and `nLbuf` to zero.
