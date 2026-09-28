# DataStream ownership and interfaces

Images, depth, points, lines, maps, and IMU samples are owned by independent DataStreams. Image and geometry producers publish complete frames, while IMU producers append samples. Consumers resolve a stream name through `InstanceMgr::findDataStream` instead of calling a producer module. `OCTREE_CELL` and selectable-grid cell interfaces remain grid-owned.

Declare each stream as a top-level launch object. Streams have no worker thread:

```json
{
  "image": { "type": "dataStream", "class": "RGBframe" },
  "rgbd": { "type": "dataStream", "class": "RGBDframe" },
  "points": { "type": "dataStream", "class": "PCLframe" },
  "lines": { "type": "dataStream", "class": "LineFrame" },
  "imu": { "type": "dataStream", "class": "IMUstream" },
  "map": { "type": "dataStream", "class": "PCLmap" }
}
```

A stream can remain enabled when its producer is disabled: readers then see an empty initial snapshot. Stream names must be unique, and configured readers require the matching stream type.

## Module configuration

| Module | Input | Output |
| --- | --- | --- |
| Camera/image producers | Device or file | `RGBframe` |
| Image filters | `RGBframeIn` (depth filter: `DframeIn`) | `RGBframe` |
| `_WindowCV`, `_GstOutput` | `RGBframeIn` | Window or video output |
| Image detectors and trackers | `RGBframeIn` | Existing detection/tracking outputs |
| `_PointCloud`, `_PCfile`, `_PCrecv` | File or transport as configured | `PCLframe` |
| `_PCtransform` | `PCLframeIn` | `PCLframe` |
| `_PCmerge` | `vPCLframes` array | `PCLframe` |
| `_PCsend` | `PCLframeIn` | Existing transport module |
| `_Line` | Producer-defined | `LineFrame` |
| `_OctreeGrid`, `_SelectableOctGrid` | `vPCLframes` array | Existing cell interface |
| `_Livox2` | Existing UDP modules | `PCLframe`; optional `IMUstream` |
| `_SLAMbase`, `_GLIM` | `PCLframeIn`; optional `IMUstream` | `_GLIM`: optional `PCLframe` and `PCLmap` |
| RGBD camera modules | Device | `PCLframe`; IMU output key is `IMUframe` |
| `_WebGLIM` | `PCLmapIn` | Browser map stream |

Point pipeline inputs and outputs must use different streams. Replace old `_PointCloud`, `_GeometryBase`, `globalMapPCL`, and `vGeometryBase` data references with the applicable keys above. A former `_PointCloud` object used only for storage becomes a `PCLframe` DataStream declaration; remove its `thread` and `nP` settings.

For example, a file, grid, and viewer can share one cloud without referring to the file module:

```json
{
  "filePoints": { "type": "dataStream", "class": "PCLframe" },
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

RGBframe, RGBDframe, PCLframe, LineFrame, PCLmap, and IMUstream use `set(...)` and `get()`. The former
`copyFrom`, `copyTo`, image `getSize`, and separate timestamp accessors are removed.
A snapshot holds its data together with `m_tStamp` and `m_revision`.

`RGBframe::set(const Mat &, stamp)` and
`RGBDframe::set(const Mat &, const Mat &, stamp)` copy incoming pixels once,
so camera SDK buffers and reusable scratch images can be released or reused as
soon as publication returns. `get()` shares the owned pixels without
copying. RGB snapshots expose `m_mRGB`; RGBD snapshots expose `m_mRGB` and `m_mD`
from one coherent publication. Image dimensions come from those matrices.

Hold the snapshot while reading its matrices, and use `const Mat &` for input.
OpenCV matrix headers are shallow copies, so a mutable header does not grant
ownership of the pixels. Call `clone()` before drawing or editing:

```cpp
const auto frame = imageStream->get();
const Mat &input = frame->m_mRGB;
Mat annotated = input.clone(); // private pixels for drawing
```

Empty image publications clear prior pixels and advance the revision. The
snapshot itself is always valid, including the initial revision-zero snapshot.

`IMUstream::set(Type::Gyro, value, stamp)` and
`set(Type::Acc, value, stamp)` append samples in constant time. `get()`
returns immutable `m_dqGyro` and `m_dqAcc` histories, each capped at 1000 samples,
plus the common timestamp and revision. Each call to `get()` copies this bounded history. SLAM retains a snapshot
while draining a batch; pairing and sequence cursors belong to the consumer.


`PCLframe::get()` and `LineFrame::get()` return shared immutable storage containing the records, publication timestamp, and revision. Revision zero means nothing has been published. `set(std::move(records), stamp)` transfers a complete vector without copying its records; publishing an empty vector clears the frame and advances the revision. Readers may retain a snapshot while another thread publishes its replacement.

These are latest-frame streams, not queues: a slow consumer can skip intermediate publications. Consumers compare revisions instead of assuming capture timestamps always increase. Point records retain their individual timestamps; zero is invalid. Expiration settings are in nanoseconds and require timestamps in the same clock domain as the consumer.

Grids ingest each stream revision once. Re-reading an unchanged frame does not increase occupancy counts or refresh cell lifetime. Set `dTexpireCell` to zero for persistent occupancy from a static file. Changing the grid root clears occupancy and allows the current frame to be consumed once under the new root.

Livox assembles packets with the same device `frame_cnt`, then moves the completed cloud into its `PCLframe` when the next frame starts. `nMaxFramePoints` bounds that assembly buffer (default 100000); excess points in that frame are discarded. Clearing the driver also discards its pending unpublished frame. Its `IMUstream` output is independent of the cloud frame.

`PCLmap` stores shared local submap points and separate poses so loop closure can update poses without copying every point. It also carries a session identifier for map resets. Geometry web viewers use the current v6 protocol; the GLIM viewer uses GLM2. Legacy wire decoders are not supported.

## Examples and scope

Updated launches include `WebViewer3D.json`, `ImGUI.json`, `APmav_drive.json`, `Orbbec.json`, `Scepter.json`, `GLIM_orbbec.json`, `GLIM_scepter.json`, and `_GLIM.json`. `Livox2.json` is a standalone LiDAR example; configure its host/device IP addresses for the local network. Build LiDAR with `WITH_SENSOR`, `WITH_UNIVERSE`, and `WITH_IO` enabled; Open3D is not required. `_GLIM.json` declares input streams but needs a producer to populate them.

The old `_IMUbase` preview modules are absent from the migrated examples; camera and LiDAR IMU data go to `IMUstream`. The mixed `_LCalign.json` launch and legacy `.kiss` launches still reference excluded Tools/Open3D/ROS interfaces and are not migrated examples. Classes guarded by `USE_OPEN3D`, `Universe/Object`, and `Universe/Surface` remain outside this migration.

## Point transport and validation

`_PCsend` and `_PCrecv` use only PCL1. Every packet is little-endian and has a 32-byte header: four-byte `PCL1` magic, packet byte count (`u32`), frame revision (`u64`), frame timestamp (`u64`), total point count (`u32`), and first point index (`u32`). Each 32-byte record contains XYZRGB (`6 × float32`) and its timestamp (`u64`). There is no native-struct padding. The receiver publishes only a complete frame; an empty frame is represented by its header. Older point transport layouts are unsupported.

Run the independent stream, viewer, geometry, grid, LiDAR, and SLAM-input checks from the repository root:

```sh
cmake -S test/DataStream -B /tmp/openkai-stream-tests
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

python3 html/viewer/tests/run.py /tmp/openkai-stream-tests/viewer_snapshot_test
```

These checks use synthetic data and do not start hardware devices.
