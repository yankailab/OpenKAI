# OpenKAI editor catalog

`OpenKAI.json` is the editor's class and configuration catalog. It is **editor metadata, not a JSON Schema validation document** and is never passed to the C++ application. The editor exports the usual OpenKAI configuration format. Schema version 1 is documented below.

The browser loads `OpenKAI.json` when served over HTTP. For opening `index.html` directly from disk, `js/schema-data.js` provides the identical object as `window.OPENKAI_SCHEMA`; browsers normally prohibit fetching adjacent JSON over `file:`. Regenerate both files together. All assets and data stay local; neither generation nor editing needs an online service.

## Regenerate after source changes

From the repository root, with Python 3 and no third-party packages:

```sh
python3 html/console/editor/tools/generate-schema.py
python3 html/console/editor/tools/generate-schema.py --check
```

The first command writes the JSON catalog and JavaScript mirror. The second regenerates them in memory and fails if either checked-in artifact differs. Generation is deterministic: no clock timestamps, absolute source paths, network access, C++ build, or SDK installation are used. `sourceDigest` hashes the relative paths and bytes of every scanned source file, so even a source-only change causes `--check` to request regeneration.

The current tree contains 346 scanned `.h`/`.cpp` files, 202 explicit class declarations representing 201 distinct names, and 128 factory-creatable classes. Other supported C++ extensions are `.hpp`, `.hh`, `.cc`, and `.cxx`. `src/Dependencies` is excluded. The two declarations of `_APmav_base` are recorded in `declarations`; the active `Autopilot/FC/ArduPilot` definition matches `Module.h`. `_TestJSON` and `_TestWebSocket` remain factory registrations without source declarations and are listed in the audit, rather than offered as fabricated classes.

## Top-level fields

| Field | Meaning |
| --- | --- |
| `schemaVersion` | Integer format version, currently `1`. |
| `title`, `description` | Human-readable catalog information. |
| `generator` | Relative location of the generator. |
| `sourceDigest` | SHA-256 of scanned source names and contents. |
| `categories` | Sorted folder paths relative to `src`, such as `Vision` or `Vision/Pipeline`. |
| `classes` | Array of class records described below. |
| `application` | Special `APP` / `ModuleMgr` settings: `bStdErr` and `vInclude`. |
| `audit` | File inventory, extraction counts, diagnostics, missing factory definitions, and limitations. |

## Class records

```json
{
  "name": "_Crop",
  "category": "Vision/Pipeline",
  "source": "src/Vision/Pipeline/_Crop.h:16",
  "baseClasses": ["_VisionBase"],
  "creatable": true,
  "buildConditions": [["ifdef WITH_VISION", "ifdef USE_OPENCV"]],
  "configurable": true,
  "parameters": [],
  "dependencies": [],
  "containers": [],
  "diagnostics": []
}
```

The example shows the record shape; generated records contain the actual parameters and dependencies. `source` references a declaration, usually `repository/path:line`. Audited adapters may use `repository/path:method`. `declarations`, when present, preserves duplicate declaration locations.

`baseClasses` lists direct C++ base types. Parameters, dependencies, and container shapes are **already flattened** through inheritance, with a derived path replacing the inherited path. Consumers should not merge inherited field arrays again. Embedded `_Thread`, `StateBase`, and ROS node configuration is flattened at its actual JSON path. Fields retain `declaredIn`, and embedded fields also carry `embeddedClass`.

`creatable` means the class is registered by `ADD_MODULE` in `Module::createInstance`. Helper classes, base classes, and templates remain discoverable but cannot be instantiated by the current launch factory. `ModuleMgr` is the special application block, not a normal factory module. `buildConditions` records alternative preprocessor stacks at factory registration; it does **not** say that a user's binary enabled those features. A class can be creatable in the catalog and unavailable in a particular build.

## Parameters and paths

```json
{
  "path": ["thread", "FPS"],
  "type": "number",
  "default": 30,
  "declaredIn": "_Thread",
  "embeddedClass": "_Thread",
  "source": "src/Base/_Thread.cpp:60"
}
```

`path` is an array of literal JSON property names, relative to the instance object. Do not split a component on dots. A `"*"` component means each array element or each property of an object map. Examples:

| Path | Configuration value |
| --- | --- |
| `["deviceID"]` | `instance.deviceID` |
| `["thread", "FPS"]` | `instance.thread.FPS` |
| `["vGeometry", "*", "matPointSize"]` | A material value in every `vGeometry` array entry. |
| `["motors", "*", "kSpd"]` | A value in each named `motors` object entry. |
| `["states", "*", "next"]` | The next-state name in each embedded state. |

`containers` records the shapes needed to interpret wildcards, for example `{"path":["motors"],"type":"object"}` versus `{"path":["vGeometry"],"type":"array"}`. The same shape records are included on a dependency for convenience.

Parameter `type` is `boolean`, `integer`, `number`, `string`, `array`, `object`, or `json`. `json` deliberately accepts arbitrary JSON where static C++ type resolution is incomplete. `nullable: true` permits explicit `null`, particularly for optional Orbbec hardware overrides. Imported fields that are absent from the catalog must still be retained.

Optional metadata includes:

- `default`: a safely parsed source literal, vector initializer, or simple constant expression. It is a convenience for adding instances, not proof of an SDK/device's runtime value. Inherited constructor assignments override known source defaults. Dynamic expressions are not executed.
- `defaultExpression`: an unresolved C++ initializer or a value assigned from runtime state; never evaluate this as JavaScript.
- `cppType`, `cppVariable`, `typeEvidence`: extraction provenance. `typeEvidence: "key-convention"` is weaker than an actual declaration.
- `description`, `group`: help text and source-defined control grouping.
- `dependency: true`: the value also has a dependency record and should be edited as a module reference.
- `extraction: "audited-adapter"`: a composed or dynamic C++ reader handled explicitly by the generator.

Absence of a default does not mean a field is required. Avoid filling every unknown field with zero or an empty string; omission preserves the application's behavior. Hardware overrides are intentionally left absent until explicitly configured. `name` and `class` identify the instance and should not receive the empty strings found in base-class member initializers. Embedded threads get their runtime name/class from `createThread`; the catalog does not inject duplicate child identities.

`bON` is a boolean module switch (`false` disabled, `true` enabled), with a runtime default of `true`. `ModuleMgr` reads it as a C++ `bool`. Older integer values are preserved on import and flagged as type mismatches; use the inspector to replace them with JSON booleans.

## Dependencies

```json
{
  "path": ["_VisionBase"],
  "targetClass": "_VisionBase",
  "multiple": false,
  "required": true,
  "declaredIn": "_Crop",
  "source": "src/Vision/Pipeline/_Crop.cpp:58",
  "containers": []
}
```

A dependency means the owning instance looks up another module by name. It is stored as the ordinary instance-name string in the exported config. The graph draws an arrow from the provider to the dependent instance that retains its pointer (provider port to dependency socket). Compatible providers include `targetClass` and derived classes according to the catalog's inheritance tree.

`multiple: true` means the value at `path` is an array of module-name strings, such as `_Console.vBASE` or `_Mavlink.vRoutings`. A path containing a wildcard represents repeated fields; `multiple` still describes **each terminal value**, not the container:

```json
{
  "path": ["vGeometry", "*", "_GeometryBase"],
  "targetClass": "_GeometryBase",
  "multiple": false,
  "containers": [{"path": ["vGeometry"], "type": "array"}]
}
```

The resulting launch value is `"vGeometry": [{"_GeometryBase": "cloud"}]`. `_ApDrive.motors` uses an object map instead, for example `"motors": {"left": {"_ActuatorBase": "motor"}}`.

`required: true` is emitted only for a clear, immediate unconditional scalar-link failure check. Its absence does not guarantee an optional link; conditional validation and hardware state remain in C++. `_APmav_drive` supports `_SelectableOctGrid` with `_OctreeGrid` as a legacy fallback, so neither alias is independently marked required. Thread scheduling references describe the source as written; the editor cannot make an embedded `_Thread` factory-creatable.

## Extraction and audit boundaries

The scanner removes comments, identifies explicit class/struct declarations, reads factory registration guards, and scans `loadConfig`/`link` bodies. It resolves `jKv` reads, `jK` child aliases, iterator/range loops, immediate `findModule` calls, scalar/vector declarations, and embedded `createThread` calls. The composed `ROS_fastLio::init` reader is included explicitly. Runtime console command payloads are not launch configuration.

Audited adapters cover the shared selectable-grid viewer source reader, `_StateControl` embedded states, application/module switches, `_Canvas.vRoi`, selected grid UUIDs, grid-reference aliases, Mavlink routing type, and the source-defined Scepter/Orbbec control catalogs. The generator reads the hardware-control key lists directly from source so additions appear on regeneration. SDK-dependent ranges, supported controls, C++ enum conversions, custom object validators, and general arbitrary C++ execution are not reproduced. Complex hardware objects remain editable as JSON.

The current automatic pass resolves 674 of 684 `jKv` reads and 99 of 100 module lookups in these readers. The 11 diagnostic entries all have explicit `resolution` notes: the shared viewer helper's dynamic reads/lookups are covered by adapters, and the other reader inspects a referenced module's `bON` rather than the viewer's own config. These counts concern recognized function bodies, not a guarantee that future arbitrary C++ patterns will be inferred. All diagnostics, including struct-helper diagnostics, appear under `audit.diagnostics`; a class also carries its local diagnostics.

The source inventory is comprehensive for explicit `class` declarations; structs are inspected to infer member types and composed settings but are not all shown as independent catalog classes. Namespaces/templates are not a full C++ AST. New duplicate class names, macro-defined classes, unusual initializer syntax, helper readers, or runtime-generated keys need human review. Inspect source changes alongside the regenerated catalog, especially new `jKv` aliases, dependency lookups, and dynamic readers.

## Extending the catalog

Prefer adding ordinary parameters to `loadConfig`, dependencies to `link`, and factory modules to `Module.cpp`; regeneration handles the common patterns automatically. For a new helper or dynamic reader, extend `adapters()` in `tools/generate-schema.py` and cite the source reader. Keep the adapter small and avoid invented defaults. For a newly supported parser pattern, check representative flat, nested-array, and object-map paths after regeneration.

A project-specific override file can also be applied without changing the scanner:

```json
{
  "classes": {
    "_Camera": {
      "parameters": [
        {
          "path": ["deviceID"],
          "type": "integer",
          "default": 0,
          "description": "Local camera index",
          "source": "src/Vision/_Camera.cpp:28"
        }
      ]
    }
  }
}
```

```sh
python3 html/console/editor/tools/generate-schema.py --overrides path/to/overrides.json
python3 html/console/editor/tools/generate-schema.py --overrides path/to/overrides.json --check
```

Parameter/dependency/container entries replace generated entries at the same path; other class fields replace the corresponding value. Overrides apply to the final flattened class only, so use a generator adapter when an addition must propagate through inheritance. Keep the override file and generation command with the project. Do not hand-edit only the JSON file: the disk-open fallback would diverge.
