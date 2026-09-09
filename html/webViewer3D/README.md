# OpenKAI webViewer3D

Open `index.html`, enter the C++ backend's IP and stream port, then click **Start**.
Streaming uses port `8080` by default; the independent command connection uses
port `7890`, editable in the geometry panel. **Start** connects both; **Stop**
disconnects both. The Commands panel also has independent Connect/Disconnect
buttons and a bounded `cmdState` console.
The backend serves this directory itself. No npm, CDN, or extra web server is
required. The full setup, configuration and wire format are documented in
[`docs/3D/WebViewer3D.md`](../../docs/3D/WebViewer3D.md).

- `css/`: viewer styles
- `js/wsStreamBase.js`: geometry connection and stream flow control
- `js/wsCmdBase.js`: command connection, `wsSendCmd()` and console status
- `js/wsCmdHandler.js`: JSON reply assembly and page-specific `handleCmd()`
- Other `js/` files: launcher, binary decoder, renderer and application wiring
- `vendor/`: pinned three.js and OrbitControls, with their license

Keep application commands out of the stream transport. Add UI controls that call
`wsSendCmd({ cmd: 'test', module: 'tester', v: 0 })` and handle their replies in
`handleCmd(jCmd)`. This preserves `_WSconsole`'s `JSON + EOJ` request scheme.
An optional backend configuration is in `jsonCfg/WebViewer3D_commands.json`;
include it from your application's `APP.vInclude` when using commands.
