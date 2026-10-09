# DataObject ownership and interfaces

Images, depth, points, lines, maps, detections, tracked boxes, and IMU samples are owned by independent DataObjects. `*Frame` classes replace complete payloads with `set(...)` and copy them with `get(...)`. `*Stream` classes append timestamped elements with `add(...)` and copy retained histories with `get(...)`. Detection and tracking outputs use `BBoxStream`; IMU channels use `IMUstream`. Consumers resolve a DataObject name through `InstanceMgr::findDataObject` instead of calling a producer module. `OCTREE_CELL` and selectable-grid cell interfaces remain grid-owned.

`BBoxStream`, `BytePacketStream`, `CANframeStream`, and `UGLIDcellStream` inherit
`DataObjStream<T>` with `BBOX_OBJ`, `BYTE_PACKET`, `CAN_FRAME`, and `UGLID_CELL_T`
elements, respectively. Their shared `add(const vector<T>&, tStamp = 0)` copies
elements into a bounded ring without changing element timestamps. The optional
timestamp updates only the stream's `m_tStamp`; zero uses the current backend
time. `get(vector<T>&, tStampFrom = 0)` copies retained elements whose `m_tStamp`
is strictly greater than `tStampFrom`, in arrival order, and returns the stream's
update timestamp under the same lock. Reads are non-destructive.

Producers supply element timestamps, including byte packet timestamps:

```cpp
BYTE_PACKET packet;
packet.set(bytes, getTns());
packetStream.add({packet});
vector<BYTE_PACKET> packets;
uint64_t streamTimestamp = packetStream.get(packets, lastPacketTimestamp);
```

The returned stream timestamp is separate from the element cursor. Consumers
advance that cursor from the elements they process. Producers that require
incremental delivery must coordinate increasing, nonzero element timestamps;
the stream preserves duplicate, zero, and out-of-order timestamps as supplied.

Declare each DataObject as a top-level launch object. DataObjects have no worker thread:

```json
{
  "image": { "type": "dataObject", "class": "RGBframe" },
  "rgbd": { "type": "dataObject", "class": "RGBDframe" },
  "points": { "type": "dataObject", "class": "PCLframe" },
  "lines": { "type": "dataObject", "class": "LineFrame" },
  "imu": { "type": "dataObject", "class": "IMUstream" },
  "detections": { "type": "dataObject", "class": "BBoxStream" },
  "map": { "type": "dataObject", "class": "PCLmap" }
}
```

A DataObject can remain enabled when its producer is disabled: readers then copy an empty initial payload. DataObject names must be unique, and configured readers require the matching type.

## Module configuration

DataObject link keys end in `In` for inputs and `Out` for outputs. The suffix describes the module's use of the object; DataObject class names stay unchanged. Corresponding C++ pointer names end in lowercase `in` or `out`, such as `m_pRGBin` and `m_pRGBout`.

| Module | Input | Output |
| --- | --- | --- |
| Camera/image producers | Device or file | `RGBframeOut` |
| Image filters | `RGBframeIn` (depth filter: `DframeIn`; mask: `RGBframeMaskIn`) | `RGBframeOut` |
| `_OCVwindow`, `_GstOutput` | `RGBframeIn` | Window or video output |
| `_Contour`, `_ArUco`, `_YOLO26detectONNX` | `RGBframeIn` | `BBoxStreamOut` |
| `_SingleTracker` | `RGBframeIn`, target commands through `_TrackerBase` | `BBoxStreamOut` |
| `_APmav_follow`, `_APmav_land` | `BBoxStreamIn`; optional `_TrackerBase` and `BBoxStreamTrackIn` | Existing autopilot controls |
| `_PCLfile`, `_PCrecv` | File or transport as configured | `PCLframeOut` |
| `_PCtransform` | `PCLframeIn` | `PCLframeOut` |
| `_PCmerge` | `vPCLframesIn` array | `PCLframeOut` |
| `_PCsend` | `PCLframeIn` | Existing transport module |
| `_PCregistCol` | `PCLframeIn` | `PCLframeOut` |
| `_LCalign` | `PCLframeIn`, `RGBframeIn` | `PCLframeOut` |
| `_PCregistICP`, `_PCregistGlobal` | `PCLframeSrcIn`, `PCLframeTgtIn` | Existing registration result |
| `_OctreeGrid`, `_SelectableOctGrid` | `vPCLframesIn` array | Existing cell interface |
| `_Livox2` | Existing UDP modules | `PCLframeOut`; optional `IMUstreamOut` |
| `_RoboSenseAiry` | Existing UDP modules | `PCLframeOut` |
| `_SLAMbase`, `_GLIM` | `PCLframeIn`; optional `IMUstreamIn` | `_GLIM`: optional `PCLframeOut` and `PCLmapOut` |
| RGBD camera modules | Device | `RGBframeOut`, `DframeOut`, `IRframeOut`, `RGBDframeOut`, `RGBDtRGBframeOut`, `RGBDtDframeOut`, `PCLframeOut`, `IMUstreamOut` |
| `_WebGLIM` | `PCLmapIn` | Browser map stream |

Point pipeline inputs and outputs must use different streams. Replace old `_PointCloud`, `_GeometryBase`, `globalMapPCL`, and `vGeometryBase` data references with the applicable keys above. A former `_PointCloud` object used only for storage becomes a `PCLframe` DataObject declaration; remove its `thread` and `nP` settings.

The `_PointCloud`, `_GeometryBase`, and `_Line` module classes have been removed.
Remaining former geometry subclasses inherit directly from `_ReferenceFrame`. Point-cloud producers
each declare their own `PCLframe *m_pPCLout` output pointer,
resolved from the `PCLframeOut` configuration key. The referenced DataObject owns
the point data and remains managed by `InstanceMgr`. Use a `LineFrame` DataObject
declaration for line storage. Geometry viewers retain their existing typed input
streams and camera settings.

For example, a file, grid, and viewer can share one cloud without referring to the file module:

```json
{
  "filePoints": { "type": "dataObject", "class": "PCLframe" },
  "pcFile": {
    "class": "_PCLfile",
    "PCLframeOut": "filePoints",
    "vfName": ["data/PointCloud/StanfordBunny/bun000.ply"]
  },
  "octGrid": {
    "class": "_SelectableOctGrid",
    "vPCLframesIn": ["filePoints"],
    "dTexpireCell": 0
  },
  "viewer": {
    "class": "_WebSelectableOctGrid",
    "port": 8080,
    "vGeometry": [{ "PCLframeIn": "filePoints", "nP": 400000 }],
    "vSelectableOctGrid": [{ "_SelectableOctGrid": "octGrid" }]
  }
}
```

Both web and ImGUI geometry viewers retain `vGeometry`. Each entry names `PCLframeIn`, `LineFrameIn`, or both, with an optional display `name`. Existing `nP`, `nL`, visibility, and material settings remain viewer limits and styling. The old `_GeometryBase` entry and `bFrame` setting are removed. `vSelectableOctGrid` remains unchanged. `_WebGLIM` reads the stream named by `PCLmapIn`; its source `_GLIM` writes that same stream through `PCLmapOut`.

## Bounding box and target streams

Replace detection canvas storage with a `BBoxStream` DataObject and remove its
worker-thread and drawing settings. Detectors and trackers resolve the stream
named by `BBoxStreamOut`; `_APmav_follow` and `_APmav_land` resolve detections
through `BBoxStreamIn`. DataObjects do not belong in a console's `vBASE` module
list. Each detector or tracker appends timestamped elements; existing history is
retained until the configured `nBuf` capacity evicts older elements.

`BBoxStream` owns copied `BBOX_OBJ` records. A record's `m_type` determines the
meaning of its position and dimensions:

| Type | `m_vPos` | `m_vDim` |
| --- | --- | --- |
| `obj_bbox` | Pixel coordinates `(left, top, 0)` | Pixel dimensions `(width, height, 0)` |
| `obj_tag` | Pixel center `(x, y)` and tag angle in degrees in `z` | Pixel radius in `x`; `y` and `z` are zero |

`m_vClass` stores class IDs (tag IDs for ArUco) and integer confidence percentages
from 0 to 100. `m_tStamp` records the element's capture or append time in
nanoseconds. Do not normalize detector output before appending it.

Producers use `add(objects, timestamp)` to append copies of detected or tracked
objects. The timestamp is assigned to the new elements; a zero/default timestamp
uses the current host clock. The stream keeps at most `nBuf` elements (default
1000), retaining the newest appended entries. Append order is preserved; element
timestamps may repeat or arrive out of order. An empty `add` does nothing and
preserves prior history and its timestamp. Missing detections, inference failure,
and stopped tracking therefore do not erase earlier records.

`get(objects)` copies the retained history and returns the timestamp from the
last nonempty append. Consumers determine validity, expiry, and selection from
each record's `m_tStamp`; the stream's update timestamp does not make older
elements fresh. Multiple appends can share one timestamp, and the last appended
element need not have the newest capture time. Consumers also own deduplication
or timestamp cursors when processing history and must not assume timestamp order.

Container dimensions are separate metadata:
`setContainerDim(Vector3f(width, height, 0))` sets them and
`getContainerDim()` returns them. Changing dimensions neither clears history nor
updates element timestamps. Target controllers use these image dimensions to
normalize pixel coordinates. The dimensions apply to all retained records;
keep a consistent image source and coordinate system for a stream's history.
Use separate streams for producers with different image dimensions or coordinate
systems. Reading dimensions and history is separate; they do not form an atomic
combined result.

Target control timestamps must use the host monotonic nanosecond clock returned
by `getTns()`. Detectors and trackers preserve image capture timestamps;
re-reading history cannot refresh an element's age. `_APmav_follow` and
`_APmav_land` reject records with future timestamps and records older than
`tOutTargetNotFound` (100 ms by default). A zero timeout disables the follower's
target-loss hold while retaining a 100 ms element freshness limit. Set this limit
with the expected camera and inference latency in mind.

Optional tracking uses a separate output stream. Configure a `_SingleTracker`
with `RGBframeIn` and `BBoxStreamOut`, then configure the follower's
`_TrackerBase` with that tracker module and `BBoxStreamTrackIn` with the same
output stream. The tracker stream must differ from `BBoxStreamIn`. Starting
and stopping a track remain tracker commands; target geometry is read from the
DataObject. Tracking boxes use the same pixel
convention as detection boxes.

For landing tags, normalized area is the radius bounding square's area,
`4 * radius * radius / (image width * image height)`. Review existing `vSize`
and `vKdist` tag calibration when migrating: the former generic dimension-area
calculation returned zero for radius-only tag dimensions.

Run the focused bounding-box stream, tracker, and target-control checks from the repository root:

```sh
cmake -S test/DataObject -B /tmp/openkai-bbox-tests
cmake --build /tmp/openkai-bbox-tests --parallel 2
ctest --test-dir /tmp/openkai-bbox-tests --output-on-failure

cmake -S test/Tracker -B /tmp/openkai-tracker-tests
cmake --build /tmp/openkai-tracker-tests --parallel 2
ctest --test-dir /tmp/openkai-tracker-tests --output-on-failure

cmake -S test/Autopilot -B /tmp/openkai-target-tests
cmake --build /tmp/openkai-target-tests --parallel 2
ctest --test-dir /tmp/openkai-target-tests --output-on-failure
```

## Frame ownership and timing

RGBframe, RGBDframe, PCLframe, LineFrame, and PCLmap use copying `set(...)` and
`get(...)` interfaces. `set` replaces the complete stored payload with a copy of
caller-owned data; `get` copies that payload into caller-owned output buffers.
Later writes to either copy do not change the other. BBoxStream and IMUstream
use append/get history interfaces instead. DataObjects have no shared snapshot
objects or revision counters.

For whole-frame DataObjects, `getTstamp()` cheaply reads the current timestamp.
Consumers can skip `get` when it matches their last timestamp. Because a writer can run between
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
each capped by `nBuf` (default 1000), and returns their common update timestamp.
Sample pairing, validity checks, and timestamp cursors belong to consumers such
as SLAM. A repeated or older timestamp appends without resetting the channel's
history; each consumer decides whether it can use the sample. Gyro and
accelerometer samples can arrive separately with the same timestamp, so an IMU
reader must inspect both copied histories and track each channel's sample
timestamps. Do not skip an entire IMU read merely because `getTstamp()` matches
the previous value.

Whole-frame DataObjects retain one complete payload, so a slow consumer can skip
updates. Their change detection uses timestamps only. Repeated timestamps are
indistinguishable even when the payload changed. Frame producers must supply
changed timestamps for updates that readers should observe. SLAM additionally
requires increasing capture times. `_PCtransform` preserves the input capture timestamp and
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

Updated launches include `Detectors.json`, `WebViewer3D.json`, `ImGUI.json`, `APmav_drive.json`, `Orbbec.json`, `Scepter.json`, `GLIM_orbbec.json`, `GLIM_scepter.json`, and `_GLIM.json`. `Livox2.json` is a standalone LiDAR example; configure its host/device IP addresses for the local network. Build LiDAR with `WITH_SENSOR`, `WITH_UNIVERSE`, and `WITH_IO` enabled; Open3D is not required. `_GLIM.json` declares input streams but needs a producer to populate them.

The old `_IMUbase` preview modules are absent from the migrated examples; camera and LiDAR IMU data go to `IMUstream`. The `_LCalign.json` launch now uses point and image DataObjects but retains legacy viewer and IMU settings. Legacy `.kiss` launches still reference older Tools/Open3D/ROS interfaces and are not migrated examples. The historical `test/apMavlinkFollow.kiss` has updated bounding-box stream links, but its older syntax and remaining module settings still need migration before use. Open3D point registration now uses the point stream keys above, while its registration and visualization algorithms still require `USE_OPEN3D`. Other `USE_OPEN3D` interfaces remain outside this migration.

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
