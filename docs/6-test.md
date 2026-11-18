# Testing and debugging

## Tests without an OpenKAI build

The Airy and E1R wire parsers require only Python 3 and a C++11 compiler. Add
`--sanitize` to enable AddressSanitizer and UndefinedBehaviorSanitizer.

```bash
python3 test/run_airy_protocol_tests.py
python3 test/run_e1r_protocol_tests.py
```

The viewer protocol and grid controls have Node.js tests:

```bash
node --test test/UI/*.test.mjs
```

The HTTP server has a standalone CMake project. It requires Boost headers,
threads, and permission to open loopback sockets:

```bash
cmake -S test/Net -B build/http-tests
cmake --build build/http-tests
ctest --test-dir build/http-tests --output-on-failure
```

## Tests against an OpenKAI build

Configure the required modules with `BUILD_EXECUTABLE=ON` and
`CMAKE_EXPORT_COMPILE_COMMANDS=ON`, then build before running a suite. The Python
runners reuse that build's compiler flags and library dependencies, with test
assertions enabled even for Release builds. Unix Makefiles and Ninja are supported.
Tests compile into temporary directories. Rebuild OpenKAI after changing its source
or headers so the tests and library match.

For example, to test typed streams, byte transports, JSON/binary parsers, and CAN:

```bash
cmake -S . -B build-tests -DBUILD_EXECUTABLE=ON \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DWITH_IO=ON -DWITH_PROTOCOL=ON
cmake --build build-tests --parallel 4
python3 test/run_byte_packet_tests.py build-tests --byte-only
```

Use `--streams-only` for typed stream timestamp, filtering, wraparound, clearing,
snapshot ownership, and concurrent byte packet access checks. `--can-only` selects
CAN codecs and worker regressions. With `USE_MAVLINK=ON`, the default run also tests
MAVLink; `--mavlink-only` selects those cases. `WITH_ARDUPILOT=ON` and
`WITH_NAVIGATION=ON` add MAVLink consumer coverage. WebSocket transport checks run
when `USE_WSSERVER=ON`.

Other runners accept the build directory as their first argument:

| Runner under `test/` | Required build features / coverage |
| --- | --- |
| `run_livox1_tests.py`, `run_livox2_tests.py`, `run_airy_tests.py`, `run_e1r_tests.py` | `WITH_SENSOR`; synthetic packets, configuration, UDP retry and restart |
| `run_distance_sensor_tests.py` | `WITH_SENSOR`, `WITH_IO`, `WITH_PROTOCOL`; add `USE_LIBMODBUS` for Leddar |
| `run_spor_sensor_tests.py` | `WITH_SENSOR`, `USE_SPOR_SENSOR_SDK`; SDK capture fixtures, configurable with `--fixtures-dir` |
| `run_realsense_preview_tests.py` | `WITH_VISION`, `USE_OPENCV`, `USE_REALSENSE`; preview controls and IMU |
| `run_scepter_color_tests.py` | `WITH_VISION`, `USE_OPENCV`, `USE_SCEPTER_SDK`; mocked capture and color |
| `run_ftservo_tests.py` | `WITH_ACTUATOR`, `WITH_IO`, `USE_FTSERVO`; pseudo-terminal motion, feedback and shutdown |
| `run_modbus_lifecycle_tests.py` | `WITH_IO`, `WITH_PROTOCOL`, `USE_LIBMODBUS`; disconnected stop and destruction |
| `run_geofence_tests.py` | `WITH_NAVIGATION`; prediction and bounce |
| `run_octree_grid_tests.py`, `run_transform_rigid_tests.py` | `WITH_UNIVERSE`; grids, transforms and point pipelines |
| `run_glim_timing_tests.py` | `WITH_SLAM`, `WITH_UNIVERSE`, `USE_GLIM`; preprocessing and IMU gating |
| `run_web_geometry_tests.py` | `WITH_UI`, `WITH_UNIVERSE`; point and line serialization |
| `run_web_mavlink_tests.py` | `WITH_UI`, `USE_MAVLINK`; telemetry and stream cursors |
| `run_http_startup_tests.py` | `WITH_NET`, `WITH_UI`, `WITH_UNIVERSE`, `USE_MAVLINK`; HTTP bind retries |
| `run_aprover_lidar_tests.py` | `WITH_AUTOPILOT`, `WITH_ARDUPILOT`, `WITH_UNIVERSE`, `USE_MAVLINK`; rover avoidance with synthetic grids |

These suites do not need sensor hardware. Transport and startup cases use local
sockets; FTServo uses a pseudo-terminal. See each runner's docstring and the
corresponding module documentation for optional dependencies.

## Live and browser checks

The Python scripts under `test/Sensor/` and `test/UI/` are integration checks.
Read their `--help` before running: live sensor cases can change device settings,
and browser cases require Chrome/Chromium. The E1R script defaults to synthetic
UDP replay; Airy accepts a capture directory for replay. Hardware-free grid
configuration checks are available with:

```bash
python3 test/UI/SelectableGridConfig.py build/OpenKAI
```

## GDB
```bash
gdb --args executablename arg1 arg2 arg3
run
bt
```

## Turn off GDB downloading separate debug info
```bash
echo "unset DEBUGINFOD_URLS" >> ~/.bashrc
```


## Test gstreamer
```bash
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480,framerate=20/1 ! x264enc ! matroskamux ! filesink location=/home/pi/ssd/test.mkv
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! fbdevsink
```
