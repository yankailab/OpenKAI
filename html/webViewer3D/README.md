# OpenKAI webViewer3D

Open `index.html`, enter the C++ backend's IP and port, then click **Start**.
The backend serves this directory itself. No npm, CDN, or extra web server is
required. The full setup, configuration and wire format are documented in
[`docs/3D/WebViewer3D.md`](../../docs/3D/WebViewer3D.md).

- `css/`: viewer styles
- `js/`: launcher, connection, binary decoder, renderer, application
- `vendor/`: pinned three.js and OrbitControls, with their license
