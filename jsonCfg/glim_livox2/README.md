# Livox Mid-360 LiDAR-inertial GLIM profile

From the repository root, run:

```sh
build/OpenKAI jsonCfg/GLIM_livox2.json
```

Open `http://localhost:8080/` for the GLIM map and SLAM controls, and
`http://localhost:8081/` for the raw Livox point cloud, IMU, and sensor settings.
Both use the existing WSconsole service on port 7890 (`GLIM` and `Lvx1`).
Keep the sensor still for initialization, then click **Start** in the GLIM viewer.
SLAM waits for IMU samples covering the complete scan before processing it.
Stop SLAM before changing the sensor format, timing, work mode, or IMU settings.

This profile uses the Mid-360 at `192.168.16.120`. `_Livox2` owns its UDP sockets
and selects the PC's Ethernet address from the route to `Lvx1.lvxIP`. Host ports
are configured directly on `Lvx1`: `lvxHostPortCmd: 56101`,
`lvxHostPortState: 56201`, `lvxHostPortPCL: 56301`, and `lvxHostPortIMU: 56401`.
If the sensor address changes, update only `Lvx1.lvxIP`; an optional `lvxHostIP`
overrides automatic host selection. Restart OpenKAI after changing network
settings. Run one Livox application at a time because the standalone and SLAM
profiles use the same sensor and UDP ports.
If a firewall is enabled, allow UDP traffic from the sensor to those host ports
and broadcast discovery replies to the driver's automatically assigned query
port; see the [Livox networking notes](../../html/viewer/_Livox2/README.md).

`lvxIMUdataEn: 1` enables the built-in 200 Hz IMU. `_Livox2` converts acceleration
from g to m/s² once, using 9.80665; gyroscope values are rad/s. No ROS acceleration
scale is needed. `bIMUstab: false` keeps points in the original LiDAR coordinates
for GLIM's inertial motion compensation. `frameDurationMs: 100` groups the
non-repeating scan into approximately 10 Hz point clouds using sensor timestamps;
the Mid-360 packet frame counter does not delimit these scans.

`config_sensors.json` sets `global_shutter_lidar: false`. The frame timestamp is
the scan start and each point retains its capture time in the same nanosecond
clock as the IMU. `_GLIM` supplies relative point times in seconds for deskewing.
After a sensor restart or clock reset, stop and restart SLAM.

The nominal transform is `T_lidar_imu = [0.011, 0.02329, -0.04412, 0, 0, 0, 1]`
in metres and quaternion XYZW order. It maps IMU coordinates into LiDAR
coordinates, with aligned axes. The translation comes from the **IMU Data**
section on printed page 15 of the [official Mid-360 manual](https://terra-1-g.djicdn.com/851d20f7b9f64838a34cd02351370894/Livox/Livox_Mid-360_User_Manual_EN.pdf),
using [GLIM's transform convention](https://github.com/koide3/glim/blob/master/config/config_sensors.json).
These are the manufacturer's nominal extrinsics; the profile does not perform
per-unit calibration. Keep additional point transforms disabled unless the
LiDAR–IMU extrinsic is updated to match them.

The CPU pipeline uses LiDAR-inertial odometry, IMU-enabled submapping and global
mapping. Initial preprocessing retains 0.5–100 m points and targets 6000 points
per scan; adjust the limits and resolution for the scene. IMU noise values are
GLIM starting values, not a measured calibration of this sensor. The scan and
IMU rates follow the [GLIM sensor setup guide](https://github.com/koide3/glim/wiki/Sensor-setup-guide).

Build with `WITH_SENSOR=ON`, `WITH_SLAM=ON`, `USE_GLIM=ON`, and
`WITH_UNIVERSE=ON`, keeping the existing networking/viewer options enabled.
Use `RelWithDebInfo` or `Release` for live SLAM. The GLIM installation must
include `libodometry_estimation_cpu.so`, `libsub_mapping.so`, and
`libglobal_mapping.so`; no CUDA backend is required.

GLIM parameter snapshots are saved to `jsonCfg/glim_livox2/GLIM.controls.json`.
Point-cloud exports go to `data/glim_livox2/`. This folder is independent of the
depth-camera GLIM profiles. Run the hardware-independent timing regression with
`python3 test/run_glim_timing_tests.py build` after building OpenKAI.

For the connected sensor, stop other Livox backends and run
`python3 test/Sensor/LiDAR/live.py build/OpenKAI --glim --seconds 20`.
This uses a temporary launch file, exercises sensor controls, initializes GLIM,
checks tracking and exports a temporary PLY before shutting down. On 2026-10-22,
the stationary test processed 204 scans in approximately 20 seconds, retained a
fresh pose, and exported 8,156 points. Moving-map accuracy and loop closure have
not been validated by this stationary test.
