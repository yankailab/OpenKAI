# RoboSense E1R LiDAR-only GLIM profile

From the repository root, run:

```sh
build/OpenKAI jsonCfg/GLIM_E1R.json
```

Open `http://localhost:8080/` for the GLIM map and SLAM controls. Connect the
viewer, then click **Start**. Open `http://localhost:8081/` for the raw E1R point
cloud and sensor controls. Both use the shared command service on port 7890
(`GLIM` and `E1R`). Keep only one viewer's command connection open at a time:
disconnect commands in the current viewer before connecting them in the other.
The geometry streams can remain connected. **Stop** finalizes unfinished
submaps; the map can remain empty until a submap completes.

The sensor address is `192.168.1.200`, with MSOP on UDP 6699 and DIFOP on UDP
7788. The tested sensor sends to `192.168.1.102`, so assign that address to the
PC's connected Ethernet interface. The driver binds `0.0.0.0` and filters the
sender by `E1R.deviceIP`. These settings do not reconfigure the sensor's network.
Run one E1R application at a time because the standalone and SLAM profiles use
the same UDP ports.

The OpenKAI E1R driver exposes point clouds without an IMU stream. This profile
uses `libodometry_estimation_ct.so` for continuous-time geometric odometry and
disables IMU factors in submapping and global mapping. The map starts in the
initial LiDAR frame, without gravity alignment. Keep the driver transform fixed;
the supplied the linked `_TransformRigid` module's `vT` and `vR` are identity.

`bUseSensorTime: true` retains sensor capture timestamps and per-point offsets.
`config_sensors.json` sets `global_shutter_lidar: false` to preserve those offsets
for motion compensation: recorded E1R frames contain points spanning about
96 ms of each 100 ms frame. Restart OpenKAI after a sensor clock reset, power
cycle, or change of timestamp mode. Stop SLAM before changing sensor filtering
or mounting transforms.

Preprocessing starts with a 0.3–100 m software range, 0.1 m cells and a target of
6000 points per scan. CT odometry uses a 0.5 m target voxel size, 0.05 m minimum
point separation, a 1 m correspondence limit and eight optimizer iterations.
These are starting values to tune for the scene and motion. Slow motion and
overlapping views with varied geometry help geometric tracking.

Use an optimized OpenKAI build with `WITH_SENSOR`, `WITH_UNIVERSE`, `WITH_SLAM`,
`USE_GLIM`, `WITH_IO`, `WITH_PROTOCOL` and `USE_WSSERVER` enabled. For an existing
GLIM-capable build:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DWITH_SENSOR=ON -DWITH_UNIVERSE=ON -DWITH_SLAM=ON -DUSE_GLIM=ON \
  -DWITH_IO=ON -DWITH_PROTOCOL=ON -DUSE_WSSERVER=ON
cmake --build build --parallel 4
```

**Save parameters** in the GLIM viewer and **Save config** in the E1R viewer
update their respective modules in `jsonCfg/GLIM_E1R.json`. Profile files in this
folder supply the GLIM defaults; restart OpenKAI after editing them. Point-cloud
exports go to `data/glim_e1r/`.
