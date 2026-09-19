# GLIM in OpenKAI

Build against the installed GLIM CMake package (the source checkout is not compiled into OpenKAI):

```bash
cmake -S . -B build-glim -DWITH_SLAM=ON -DUSE_GLIM=ON
cmake --build build-glim -j4
```

Use `-DCMAKE_PREFIX_PATH=/your/install/prefix` for a nonstandard installation. `glim::glim` supplies GTSAM, gtsam_points, Eigen, OpenMP, spdlog and Boost dependencies. GLIM is only required when both `WITH_SLAM` and `USE_GLIM` are enabled. SLAM also builds the shared navigation, geometry and IMU base classes if their wider module groups are disabled.

Include `jsonCfg/_GLIM.json` using the application's `APP.vInclude`, or copy its three module definitions into your existing camera configuration. Connect the camera's `_PointCloud` and `_IMUbase` outputs to `slamPoints` and `slamIMU`. Alternatively, change the SLAM module's input names to your existing modules. The example supplies input buffers; it needs a sensor producer to feed them. Paths are relative to the process working directory.

`jsonCfg/glim` contains CPU configuration templates adapted from GLIM 1.2.2, with its MIT license. You can also point `configPath` at a copy of `/home/kai/dev/glim/config`. In that copy, select the CPU odometry, sub-mapping and global-mapping JSON files in `config.json`; the upstream configuration selects GPU modules by default. Every `_GLIM` instance in one process must use the same configuration directory because GLIM configuration is global. Restart the process to reload configuration changes.

Before using a real sensor, set `T_lidar_imu` in `config_sensors.json` to its calibrated IMU-to-point-cloud transform. The example's identity transform is a placeholder. Point coordinates must be in meters; acceleration in m/s² (including gravity); angular velocity in rad/s. Point-cloud and IMU timestamps must increase in microseconds and share one clock. Keep the input point cloud in a fixed sensor frame, and calibrate extrinsics against that frame. Do not feed the estimated pose back into its input `_PointCloud` transform.

The frame interface treats each cloud as a single exposure, with zero per-point time offsets. It suits depth-camera clouds; it does not deskew a scanning LiDAR. Each producer calls `frameStart()`, `add(..., sensorTimestampUs)` for its points, then `frameStop()`. A completed frame remains available while the next one is built, even when its size exceeds the point cloud's history ring. IMU samples use `addGyro(value, timestampUs)` and `addAcc(value, timestampUs)`; pairing consumes those queues, so use one SLAM consumer per IMU buffer.

The CPU odometry example requires an IMU. For IMU-free operation, choose GLIM's `config_odometry_ct.json` and disable `enable_imu` in both mapping configs, or set `bMapping` to false. The adapter checks the selected odometry backend's IMU requirement at session startup. Sensor rates and preprocessing parameters must be tuned for your scene and hardware.

`_SLAMbase` owns input linking, timestamp deduplication, the worker, pose publication and the session lifecycle. `_GLIM` owns preprocessing, odometry, submaps and global mapping. Processing is sequential on the OpenKAI worker; if processing falls behind, it takes the newest completed cloud instead of building an unbounded frame queue. IMU-required odometry waits for a paired IMU sample later than the cloud timestamp.

- `start()` starts the worker and, by default, a session. Set `bAutoStart: false` to start tracking explicitly.
- `startTracking()` creates a new session, or returns success if one is already active. Initialization may need several seconds of IMU and point-cloud data.
- `stopTracking()` finishes mapping and retains the last pose and map. Confidence becomes zero.
- `saveMap(path)` saves the finished GLIM map after `stopTracking()`. The saved map includes global optimization; the navigation pose is continuous local odometry in GLIM's world frame, expressed as `T_world_lidar` for the input cloud frame.
- `reset()` stops tracking, discards the map, and resets position and orientation. Call `startTracking()` to begin again.

`_NavBase::setConfidence()` uses a 0–100 scale and optionally expires stale updates. SLAM defaults to a one-second `tConfidenceTimeoutUs`; zero disables expiry. GLIM supplies no scalar tracking-quality score, so this adapter reports 100 for an available finite pose and 0 for initialization, insufficient points, stopped tracking or expired updates. It is a pose-availability signal, not an accuracy estimate. `bTracking()` reports whether the session is active, including initialization.

Run the hardware-independent tests with:

```bash
cmake -S test/slam -B /tmp/openkai-slam-tests
cmake --build /tmp/openkai-slam-tests -j4
ctest --test-dir /tmp/openkai-slam-tests --output-on-failure
```

These exercise completed-frame snapshots, IMU pairing, confidence expiry, real GLIM CPU estimation on a synthetic stationary scene, map saving, and session restart/reset. Live sensor calibration and trajectory accuracy still require a hardware run.

# Installing the dependencies
```bash
sudo apt update
sudo apt install --no-install-recommends \
    libomp-dev libmetis-dev \
    libfmt-dev libspdlog-dev \
    libboost-serialization-dev \
    libboost-filesystem-dev \
    libboost-thread-dev \
    libboost-program-options-dev \
    libboost-date-time-dev \
    libboost-timer-dev \
    libboost-chrono-dev \
    libboost-regex-dev \
    libboost-random-dev \
    libboost-graph-dev

unset ROS_VERSION
```

```bash
git clone --depth 1 --branch 4.3a1 https://github.com/borglab/gtsam.git
```

Edit
```bash
nano gtsam/gtsam/geometry/SL4.cpp
```
add near the top

```Cpp
#include <cassert>
```

Edit
```bash
nano gtsam/gtsam/slam/FrobeniusFactor.h
```

Around lines 200–225, replace the evaluateError() implementation with this:
```Cpp
Vector evaluateError(const T& T1, const T& T2, OptionalMatrixType H1,
                     OptionalMatrixType H2) const override {
  const bool computeJacobians = H1 || H2;

  // predict T2*T1
  typename T::Jacobian H_T21_T2;
  const T hatT21 =
      traits<T>::Between(T2, T1,
                         computeJacobians ? &H_T21_T2 : nullptr);

  // Calculate \hat T21 * T12_, which is predicted to be I_NxN
  typename T::Jacobian H_pred_hat = T::Jacobian::Zero();
  const T pred =
      traits<T>::Compose(hatT21, T12_,
                         computeJacobians ? &H_pred_hat : nullptr);

  // Move to constructor
  const MatrixN I = MatrixN::Identity();
  const VectorD vecI = Eigen::Map<const VectorD>(I.data());

  // Calculate error
  Eigen::Matrix<double, Dim, T::dimension> H_vec_pred;
  Vector error =
      traits<T>::Vec(pred, computeJacobians ? &H_vec_pred : nullptr) - vecI;

  // Do chain rule only when Jacobians were actually requested
  if (computeJacobians) {
    const auto H_error_hat21 = H_vec_pred * H_pred_hat;
    if (H1) *H1 = H_error_hat21;
    if (H2) *H2 = H_error_hat21 * H_T21_T2;
  }

  return error;
}
```

Build
```bash
cd gtsam
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DGTSAM_BUILD_TESTS=OFF \
      -DGTSAM_BUILD_EXAMPLES_ALWAYS=OFF \
      -DGTSAM_BUILD_TIMING_ALWAYS=OFF \
      -DGTSAM_BUILD_UNSTABLE=ON \
      -DGTSAM_BUILD_PYTHON=OFF \
      -DGTSAM_INSTALL_MATLAB_TOOLBOX=OFF \
      -DGTSAM_WITH_TBB=OFF \
      -DGTSAM_USE_SYSTEM_EIGEN=ON \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
      -DGTSAM_BUILD_WITH_MARCH_NATIVE=OFF ../

make -j$(nproc)
sudo make install
sudo ldconfig
```

gtsam_points
```bash
git clone --depth 1 --branch v1.2.2 https://github.com/koide3/gtsam_points.git
cd gtsam_points
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
      -DBUILD_WITH_CUDA=OFF \
      -DBUILD_WITH_TBB=OFF \
      -DBUILD_WITH_OPENMP=ON \
      -DBUILD_WITH_MARCH_NATIVE=OFF \
      -DBUILD_DEMO=OFF \
      -DBUILD_EXAMPLE=OFF \
      -DBUILD_TOOLS=OFF \
      -DBUILD_TESTS=OFF \
      -DBUILD_TESTS_PCL=OFF ../

make -j$(nproc)
sudo make install
sudo ldconfig
```


GLIM
```bash
git clone --depth 1 --branch v1.2.2 https://github.com/koide3/glim.git
cd glim
mkdir build
cd build

unset ROS_VERSION
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr/local \
      -DCMAKE_INSTALL_LIBDIR=lib \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DEigen3_DIR=/usr/local/share/eigen3/cmake \
      -DBUILD_WITH_CUDA=OFF \
      -DBUILD_WITH_VIEWER=OFF \
      -DBUILD_WITH_OPENCV=OFF \
      -DBUILD_WITH_MARCH_NATIVE=OFF ../

make -j$(nproc)
sudo make install
sudo ldconfig
```
