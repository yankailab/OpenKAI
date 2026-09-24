# Scepter depth-only GLIM profile

Run `build/OpenKAI jsonCfg/GLIM_scepter.json` and open
`http://localhost:8080/`. The application uses the shared `html/viewer/_GLIM`
frontend, WSconsole controls, incremental submap stream, trajectory, and PLY export.

Scepter cameras have no IMU. This profile loads GLIM's
`libodometry_estimation_ct.so` for geometric odometry and disables IMU factors in
both submapping and global mapping. It keeps depth frames as global-shutter
samples with an identity sensor transform. The map starts in the first camera's
optical frame (X right, Y down, Z forward); there is no gravity alignment. The
viewer uses the same camera axes.

Depth points are converted from millimeters to meters with `dScale: 0.001`.
`pclStride: 2` samples every second image row and column. Color capture/alignment
and local image windows are disabled in this profile to reduce capture overhead.

Build with GLIM, universe, and Scepter support enabled. For an already configured
GLIM build, enable the installed SDK and retain compiler optimization:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo \
  -DUSE_SCEPTER_SDK=ON \
  -DScepterSDK_root=/path/to/ScepterSDK/BaseSDK/Ubuntu
cmake --build build --parallel 4
```

The GLIM installation must include `libodometry_estimation_ct.so`. Both camera
support options can be enabled together; the Orbbec application and its profile
remain separate.

Tune parameters in the shared viewer while SLAM is stopped. **Save parameters**
writes `jsonCfg/glim_scepter/GLIM.controls.json`; Scepter camera snapshots use
`jsonCfg/glim_scepter/Scepter.controls.json`. Those optional snapshots override
startup defaults when present. PLY exports go to `data/glim_scepter/`.

`config_odometry_ct.json` contains the geometric registration settings. Initial
values use a 0.25 m target voxel size, 0.03 m minimum point separation, a 0.5 m
correspondence limit, and eight optimizer iterations. Slow motion and overlapping
views with varied geometry help tracking; an IMU-free camera cannot use inertial
constraints through featureless scenes. Adjust these values for the camera,
scene scale, and motion. All profile JSON files and saved controls here are
independent of `jsonCfg/glim_orbbec` and the Orbbec saved controls.
