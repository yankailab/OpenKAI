# RoboSense Airy LiDAR-only GLIM profile

From the repository root, run:

```sh
build/OpenKAI jsonCfg/GLIM_Airy.json
```

Open `http://localhost:8080/` for the GLIM map and SLAM controls. Connect the
viewer, then click **Start**. Open `http://localhost:8081/` for the raw Airy
point cloud, receiver settings and IMU telemetry. Both use command port 7890
(`GLIM` and `Airy`). Keep only one viewer's command connection open at a time:
disconnect commands in the current viewer before connecting them in the other.
Geometry streams can remain connected. **Stop** finalizes unfinished submaps;
the map can remain empty until a submap completes.

The factory sensor address is `192.168.1.200`, with MSOP on UDP 6699 and DIFOP on
UDP 7788. The connected sensor sends IMU data on UDP 6688 at 200 Hz. Its destination
is `192.168.1.102`, which the PC must own on the connected Ethernet interface
with a `/24` subnet. The driver binds `0.0.0.0` and filters the sender using
`Airy.deviceIP`. These settings do not reconfigure the sensor's network. Run
one Airy application at a time because the standalone and SLAM profiles use
the same UDP ports.

## Odometry and timing

This profile uses `libodometry_estimation_ct.so` for continuous-time geometric
odometry and disables IMU factors in submapping and global mapping. The map
starts in the initial LiDAR frame without gravity alignment. Keep the driver
mounting transform fixed; the linked `_TransformRigid` module's `vT` and `vR` are identity in the supplied profile.

The native driver publishes an optional `airyIMU` stream with SI angular
velocity/acceleration and sensor timestamps. It is deliberately not connected
to GLIM in this preset. The device DIFOP reports an IMU calibration quaternion
and translation, but the supplied Airy guide does not specify the transform
direction and translation units. LiDAR-inertial operation requires verifying
that transform and the IMU/LiDAR timestamp relationship before selecting the
CPU inertial odometry module and enabling its IMU factors. An identity transform
would not match the observed nonidentity calibration.

`bUseSensorTime: true` retains sensor capture timestamps and per-point firing
offsets. `global_shutter_lidar: false` preserves them for continuous-time motion
compensation. Airy scans are split at the azimuth wrap, and decoding waits for
DIFOP's per-unit angular calibration. Restart OpenKAI after a sensor clock reset,
power cycle or timestamp-mode change. Stop SLAM before changing sensor return
mode, filtering or mounting transforms.

## Starting parameters

Preprocessing keeps points at 0.3–60 m, uses 0.1 m cells and targets 6000 points
per scan. The far threshold matches the manual's nominal range; individual
channels and low-reflectivity targets have shorter useful ranges. CT odometry
uses a 0.5 m target voxel size, 0.05 m minimum point separation, a 1 m
correspondence limit and eight optimizer iterations. These are starting values
to tune for the scene and motion, not a measured Airy noise model. Slow motion
and overlapping scans with varied geometry help tracking.

The sensor/viewer buffers allow 200,000 points per frame, covering the manual's
nominal dual-return rate of about 171,000 points per 10 Hz scan. `nMapPoints`
limits the optional flat point-cloud output to 400,000 points; the accumulated
submaps streamed by `_WebGLIM` are not capped by that value. Map updates are
published every 200 ms.

Use an optimized build with `WITH_SENSOR`, `WITH_UNIVERSE`, `WITH_SLAM`,
`USE_GLIM`, `WITH_IO`, `WITH_PROTOCOL` and `USE_WSSERVER` enabled:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DWITH_SENSOR=ON -DWITH_UNIVERSE=ON -DWITH_SLAM=ON -DUSE_GLIM=ON \
  -DWITH_IO=ON -DWITH_PROTOCOL=ON -DUSE_WSSERVER=ON
cmake --build build --parallel 4
```

**Save parameters** in the GLIM viewer and **Save config** in the Airy viewer
update their respective modules in `jsonCfg/GLIM_Airy.json`. Files in this folder
supply GLIM defaults; restart OpenKAI after editing them. Point-cloud exports
go to `data/glim_airy/`. The matching VS Code launch entries are `Airy.json`
and `GLIM_Airy.json`.
