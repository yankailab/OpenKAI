# DataObject ownership and interfaces

Images, depth, points, lines, maps, and IMU samples are owned by independent DataObjects. Image and geometry producers publish complete frames, while IMU producers append samples. Consumers resolve a stream name through `InstanceMgr::findDataObject` instead of calling a producer module. `OCTREE_CELL` and selectable-grid cell interfaces remain grid-owned.

Declare each stream as a top-level launch object. Streams have no worker thread:

```json
{
  "image": { "type": "dataObject", "class": "RGBframe" },
  "rgbd": { "type": "dataObject", "class": "RGBDframe" },
  "points": { "type": "dataObject", "class": "PCLframe" },
  "lines": { "type": "dataObject", "class": "LineFrame" },
  "imu": { "type": "dataObject", "class": "IMUstream" },
  "map": { "type": "dataObject", "class": "PCLmap" }
}
```

A stream can remain enabled when its producer is disabled: readers then copy an empty initial payload. Stream names must be unique, and configured readers require the matching stream type.

## Module configuration

| Module | Input | Output |
| --- | --- | --- |
| Camera/image producers | Device or file | `RGBframe` |
| Image filters | `RGBframeIn` (depth filter: `DframeIn`) | `RGBframe` |
| `_WindowCV`, `_GstOutput` | `RGBframeIn` | Window or video output |
| Image detectors and trackers | `RGBframeIn` | Existing detection/tracking outputs |
| `_PCfile`, `_PCrecv` | File or transport as configured | `PCLframe` |
| `_PCtransform` | `PCLframeIn` | `PCLframe` |
| `_PCmerge` | `vPCLframes` array | `PCLframe` |
| `_PCsend` | `PCLframeIn` | Existing transport module |
| `_PCregistCol` | `PCLframeIn` | `PCLframe` |
| `_LCalign` | `PCLframeIn`, `RGBframeIn` | `PCLframe` |
| `_PCregistICP`, `_PCregistGlobal` | `PCLframeSrc`, `PCLframeTgt` | Existing registration result |
| `_OctreeGrid`, `_SelectableOctGrid` | `vPCLframes` array | Existing cell interface |
| `_Livox2` | Existing UDP modules | `PCLframe`; optional `IMUstream` |
| `_RoboSenseAiry` | Existing UDP modules | `PCLframe` |
| `_SLAMbase`, `_GLIM` | `PCLframeIn`; optional `IMUstream` | `_GLIM`: optional `PCLframe` and `PCLmap` |
| RGBD camera modules | Device | `PCLframe`; IMU output key is `IMUframe` |
| `_WebGLIM` | `PCLmapIn` | Browser map stream |

Point pipeline inputs and outputs must use different streams. Replace old `_PointCloud`, `_GeometryBase`, `globalMapPCL`, and `vGeometryBase` data references with the applicable keys above. A former `_PointCloud` object used only for storage becomes a `PCLframe` DataObject declaration; remove its `thread` and `nP` settings.

The `_PointCloud`, `_GeometryBase`, and `_Line` module classes have been removed.
Remaining former geometry subclasses inherit directly from `_ReferenceFrame`. Point-cloud producers
each declare their own `PCLframe *m_pPCL` output pointer,
resolved from the `PCLframe` configuration key. The referenced DataObject owns
the point data and remains managed by `InstanceMgr`. Use a `LineFrame` DataObject
declaration for line storage. Geometry viewers retain their existing typed input
streams and camera settings.

For example, a file, grid, and viewer can share one cloud without referring to the file module:

```json
{
  "filePoints": { "type": "dataObject", "class": "PCLframe" },
  "pcFile": {
    "class": "_PCfile",
    "PCLframe": "filePoints",
    "vfName": ["data/PointCloud/StanfordBunny/bun000.ply"]
  },
  "octGrid": {
    "class": "_SelectableOctGrid",
    "vPCLframes": ["filePoints"],
    "dTexpireCell": 0
  },
  "viewer": {
    "class": "_WebSelectableOctGrid",
    "port": 8080,
    "vGeometry": [{ "PCLframe": "filePoints", "nP": 400000 }],
    "vSelectableOctGrid": [{ "_SelectableOctGrid": "octGrid" }]
  }
}
```

Both web and ImGUI geometry viewers retain `vGeometry`. Each entry names `PCLframe`, `LineFrame`, or both, with an optional display `name`. Existing `nP`, `nL`, visibility, and material settings remain viewer limits and styling. The old `_GeometryBase` entry and `bFrame` setting are removed. `vSelectableOctGrid` remains unchanged. `_WebGLIM` reads the stream named by `PCLmapIn`; its source `_GLIM` writes that same stream through `PCLmap`.

## Frame ownership and timing

RGBframe, RGBDframe, PCLframe, LineFrame, PCLmap, and IMUstream use copying
`set(...)` and `get(...)` interfaces. `set` copies caller-owned data into the
stream; `get` copies it into caller-owned output buffers. Later writes to either
copy do not change the other. Streams have no shared snapshot objects or
revision counters.

`getTstamp()` cheaply reads the stream's current timestamp. Consumers can skip
`get` when it matches their last timestamp. Because a writer can run between
these calls, record the timestamp returned by `get`: it is read under the same
lock as the copied payload. Timestamp zero denotes the initial empty stream.
A zero/default timestamp passed to image or geometry `set` uses the current
backend time.

```cpp
Mat image;
uint64_t lastTimestamp = 0;
if (imageStream->getTstamp() != lastTimestamp)
{
    lastTimestamp = imageStream->get(image);
    // image owns a copy; drawing on it does not change the stream.
}
```

`RGBframe::get(Mat &image)` copies pixels. `RGBDframe::get(Mat &rgb, Mat &depth)`
copies both images from the same update. Image dimensions come from the copied
matrices. `PCLframe::get(vector<GEOMETRY_POINT> &points)` and
`LineFrame::get(vector<GEOMETRY_LINE> &lines)` copy the record vectors.
An empty `set` clears the stored payload; give it a changed timestamp so
polling consumers observe the clear.

`IMUstream::addGyro(value, stamp)` and
`addAcc(value, stamp)` append samples in constant time.
`get(deque<IMU_DATA> &gyro, deque<IMU_DATA> &acc)` copies both bounded histories,
each capped at 1000 samples, and returns their common update timestamp. Sample
pairing and timestamp cursors belong to consumers such as SLAM. Gyro and
accelerometer samples can arrive separately with the same timestamp, so an IMU
reader must inspect both copied histories and track each channel's sample
timestamps. Do not skip an entire IMU read merely because `getTstamp()` matches
the previous value.

These are latest-frame streams, not queues: a slow consumer can skip updates.
Change detection uses timestamps only. Repeated timestamps are indistinguishable
even when the payload changed. Producers must supply changed timestamps for
updates that readers should observe. SLAM additionally requires increasing
capture times. `_PCtransform` preserves the input capture timestamp and
`_PCmerge` uses the largest input timestamp. Transform configuration or expiry
changes with the same timestamp therefore remain invisible to timestamp-gated
consumers until a new capture timestamp arrives. These modules do not substitute
a host clock or synthetic update counter. Point records retain their individual
timestamps; zero is invalid. Expiration settings use nanoseconds in the
consumer's clock domain.

Grids ingest each changed stream timestamp once. Re-reading an unchanged frame
does not increase occupancy counts or refresh cell lifetime. Set `dTexpireCell`
to zero for persistent occupancy from a static file. Changing the grid root
clears occupancy and allows the current frame to be consumed once under the
new root.

Livox assembles packets with the same device `frame_cnt`, then copies the completed cloud into its `PCLframe` when the next frame starts. `nMaxFramePoints` bounds that assembly buffer (default 100000); excess points in that frame are discarded. Clearing the driver also discards its pending unpublished frame. Its `IMUstream` output is independent of the cloud frame.

`PCLmap::get(vector<Submap> &submaps, uint64_t &session)` copies each submap's
local point vector and pose, copies the SLAM session ID, and returns the map
timestamp. Session identifies an estimator lifecycle; it is not an update
counter. Viewers retain their own copied map and send only changed poses or new
geometry over the network. Replacing geometry under an existing submap ID
requires a changed submap timestamp. Geometry viewers use v6; the GLIM viewer
uses GLM3 with map timestamps. Legacy wire decoders are not supported.

## Examples and scope

Updated launches include `WebViewer3D.json`, `ImGUI.json`, `APmav_drive.json`, `Orbbec.json`, `Scepter.json`, `GLIM_orbbec.json`, `GLIM_scepter.json`, and `_GLIM.json`. `Livox2.json` is a standalone LiDAR example; configure its host/device IP addresses for the local network. Build LiDAR with `WITH_SENSOR`, `WITH_UNIVERSE`, and `WITH_IO` enabled; Open3D is not required. `_GLIM.json` declares input streams but needs a producer to populate them.

The old `_IMUbase` preview modules are absent from the migrated examples; camera and LiDAR IMU data go to `IMUstream`. The `_LCalign.json` launch now uses point and image DataObjects but retains legacy viewer and IMU settings. Legacy `.kiss` launches still reference older Tools/Open3D/ROS interfaces and are not migrated examples. Open3D point registration now uses the point stream keys above, while its registration and visualization algorithms still require `USE_OPEN3D`. Other `USE_OPEN3D` interfaces, `Universe/Object`, and `Universe/Surface` remain outside this migration.

## Point transport and validation

`_PCsend` and `_PCrecv` use only PCL2. Every packet is little-endian and has a 24-byte header: four-byte `PCL2` magic, packet byte count (`u32`), frame timestamp (`u64`), total point count (`u32`), and first point index (`u32`). Each 32-byte record contains XYZRGB (`6 × float32`) and its timestamp (`u64`). There is no native-struct padding. The receiver publishes only a complete frame; an empty frame is represented by its header. Older point transport layouts are unsupported.

Run the independent stream, viewer, geometry, grid, LiDAR, and SLAM-input checks from the repository root:

```sh
cmake -S test/DataObject -B /tmp/openkai-stream-tests
cmake --build /tmp/openkai-stream-tests --parallel 2
ctest --test-dir /tmp/openkai-stream-tests --output-on-failure

cmake -S test/Universe -B /tmp/openkai-geometry-test
cmake --build /tmp/openkai-geometry-test --parallel 2
ctest --test-dir /tmp/openkai-geometry-test --output-on-failure

cmake -S test/Vision -B /tmp/openkai-vision-tests
cmake --build /tmp/openkai-vision-tests --parallel 2
ctest --test-dir /tmp/openkai-vision-tests --output-on-failure

cmake -S test/Tracker -B /tmp/openkai-tracker-tests
cmake --build /tmp/openkai-tracker-tests --parallel 2
ctest --test-dir /tmp/openkai-tracker-tests --output-on-failure

cmake -S test/UI -B /tmp/openkai-ui-tests
cmake --build /tmp/openkai-ui-tests --parallel 2
ctest --test-dir /tmp/openkai-ui-tests --output-on-failure

python3 html/viewer/tests/run.py /tmp/openkai-stream-tests/viewer_cache_test
```

These checks use synthetic data and do not start hardware devices.
