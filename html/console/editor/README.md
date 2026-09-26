# OpenKAI config editor

Open **`index.html` directly in a modern browser**. The editor runs entirely in the browser, offline, without installing packages, starting a server, or changing OpenKAI's C++ code. Its colors, typography, controls, and logo follow `html/viewer/`.

## Create and edit

1. Find a class in the left library. Categories follow the directories below `src/`. Drag a class onto the canvas or click its **+** button. Enable **Include base / helper classes** to browse classes that are not registered in the runtime factory; these are reference entries rather than addable modules.
2. Select an instance. Edit its name and parameters in the right panel. Inherited and embedded thread parameters are included. **Available parameters** lists omitted settings; **×** omits a value so the runtime can use its default. Arrays and objects accept JSON. **Parameters inside collections** exposes fields in existing array/map entries. **Instance JSON / custom parameters** supports additional fields.
3. Click or drag a dependency port on the right of a node to a compatible provider. Arrows run **from the dependent instance to the instance it needs**. The inspector also provides compatible target lists, supports multiple references, and removes individual connections. Array-based dependencies append a row; map-based dependencies provide an object-key field in the inspector (for example, `motors.left`).
4. Use **Arrange**, **Fit**, or the zoom buttons to organize the graph. Drag a node by its header; drag empty canvas space to pan. Scroll to zoom. **Escape** cancels a pending connection.
5. **Copy** or **Download** exports the usual OpenKAI JSON object. No graph positions, editor settings, or schema metadata are added. Launch it using your compiled executable, for example `./OpenKAI /path/to/CameraCrop.json`.

The welcome canvas includes a small camera/crop/window example. Supply hardware-appropriate parameters and use an OpenKAI build containing the selected classes.

## Existing configs

Use **Open JSON** to choose a file, **Paste JSON** to import clipboard text, or **Edit JSON** to edit the whole current document. Root metadata, unknown classes, custom parameters, slash-prefixed keys, nested objects, and `APP.vInclude` are retained. JSON formatting and block comments are not retained. Legacy `.kiss` syntax is not supported.

The graph shows objects containing a `class` string, including embedded objects. Embedded nodes are editable but cannot be used as top-level dependency providers. Configs without class-tagged objects can still be edited using **Edit JSON**.

The current `ModuleMgr::findModule` matches exact top-level runtime names. A module's explicit `name` overrides its root key. Thus a reference such as `/cam` does not resolve to `cam`; it is preserved and reported as unresolved. Renaming an instance updates references that resolved to it. Deleting one leaves its references visible for correction. References in unknown/custom fields are preserved but cannot be automatically interpreted.

Include files are retained as external paths; a browser cannot automatically read arbitrary local include paths. Edit each included file separately. Links into other files appear unresolved. Relative asset and include paths are exported unchanged and remain relative to OpenKAI's working directory.

Use **Config checks** for missing required connections, incompatible providers, unknown classes, and parameter type issues. Availability also depends on the build flags shown in the inspector. These checks do not replace runtime initialization or hardware validation. For the current runtime, use integer `bON: 0` to disable a module and `bON: 1` to enable it; older example configs containing boolean `bON` are retained and flagged.

## History and local drafts

**Undo / Redo** covers config changes, imports, New, and node moves. Shortcuts outside text fields: **Ctrl/Cmd+Z**, **Ctrl/Cmd+Shift+Z**, **Ctrl/Cmd+Y**, and **Ctrl/Cmd+S** to download. A recent draft and its layout are saved to browser local storage when available. Opening the same page restores that draft. Use **New** to start over; export files for durable backups. Local drafts are specific to this browser and page location. The editor does not send config data to a server.

When clipboard permissions are unavailable, Copy uses a browser fallback; if copying is still blocked, the JSON is selected for manual **Ctrl/Cmd+C**.

## Class schema

`OpenKAI.json` is the source-derived catalog. See [SCHEMA.md](SCHEMA.md) for its format, extraction audit, extension points, and limitations.

Browsers generally block fetching neighboring JSON from `file://`. The generated `js/schema-data.js` contains the same catalog for direct opening. When served over HTTP, the editor automatically reads `OpenKAI.json`, with the bundle as fallback. **Load schema** lets you use an updated JSON catalog immediately without a server.

Regenerate both catalog files after updating the sources:

```sh
python3 html/console/editor/tools/generate-schema.py
python3 html/console/editor/tools/generate-schema.py --check
```

All runtime assets are within this directory. Python is only used to maintain the catalog and run development checks; the editor itself uses plain HTML, CSS, and JavaScript with no third-party libraries or CDN requests.

## Verification

Run the browser suite from the repository root with Python 3 and an installed Chrome/Chromium:

```sh
python3 html/console/editor/tests/browser-smoke.py
```

It opens the editor via `file://` with network access disabled, checks the generated catalog bundle, exercises UI actions, verifies existing config round trips, and runs the model regression suite. It uses Python's standard library and Chrome's local debugging protocol; no npm or Python packages are required. To also save a screenshot:

```sh
python3 html/console/editor/tests/browser-smoke.py --screenshot /tmp/openkai-editor.png
```

The model tests can alternatively run with Node:

```sh
node --test html/console/editor/tests/model.test.js
```

## Files

| Path | Purpose |
| --- | --- |
| `index.html`, `css/editor.css`, `img/OK.png` | Layout and viewer theme |
| `js/editor.js` | Library, graph, inspector, history, import/export |
| `js/model.js` | Config preservation, names, dependencies, validation |
| `OpenKAI.json` | Class catalog with source evidence |
| `js/schema-data.js` | Generated offline catalog bundle |
| `tools/generate-schema.py`, `SCHEMA.md` | Catalog maintenance and format |
| `tests/` | Browser and model regression checks |
