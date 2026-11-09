# Testing and debugging

## DataObject initialization regression test

Loads the detection stream from `jsonCfg/Detectors.json` without starting the
camera, window, or inference. Checks `nBuf=1`, DataObject initialization before
modules, and failure propagation. Requires Eigen 5, glog, and ncurses.

```bash
cmake -S test/instance -B build/instance-tests
cmake --build build/instance-tests
ctest --test-dir build/instance-tests --output-on-failure
```

## Typed stream and byte transport regression tests

Build OpenKAI with `WITH_IO=ON`, `WITH_PROTOCOL=ON`, `BUILD_EXECUTABLE=ON`, and
`CMAKE_EXPORT_COMPILE_COMMANDS=ON`, then run:

```bash
python3 test/run_byte_packet_tests.py build --streams-only
```

Checks all four `DataObjStream<T>` subclasses for timestamp preservation,
filtering, wraparound, clearing, snapshot ownership, and concurrent byte packet
access. Use `--byte-only` for the wider suite, including duplex transports,
JSON/binary parsers, and CAN codecs. WebSocket checks run when
`USE_WSSERVER=ON`. Both modes exclude MAVLink tests.

## Socket initialization retry tests

With `WITH_SENSOR=ON`, the LiDAR suites exercise occupied UDP ports, automatic
recovery after release, partial socket cleanup, and stop/restart during retries:

```bash
python3 test/run_livox1_tests.py build
python3 test/run_livox2_tests.py build
python3 test/run_airy_tests.py build
python3 test/run_e1r_tests.py build
```

For HTTP servers and web viewers, also enable `WITH_NET`, `WITH_UI`,
`WITH_UNIVERSE`, and `USE_MAVLINK`, then run:

```bash
python3 test/run_http_startup_tests.py build
```

These suites need loopback sockets but no sensor hardware. Like the transport
suite, they require the executable target and compilation database. Startup
success means the worker is running; network readiness can follow after retries.

## Eigen vector regression tests

These checks cover vector configuration loading, geometry defaults, and bounding-box operations. They require Eigen 5 and glog.

```bash
cmake -S test/eigenVectors -B build/eigenVectors
cmake --build build/eigenVectors
ctest --test-dir build/eigenVectors --output-on-failure
```

## Console shutdown regression test

This uses a pseudo-terminal to check that Ctrl+C and initialization failures restore
the terminal settings and leave the ncurses alternate screen.

```bash
python3 test/console/shutdown.py build/OpenKAI
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
