# Eigen vector regression tests

These checks cover vector configuration loading, geometry defaults, and bounding-box operations. They require Eigen 5 and glog.

```bash
cmake -S test/eigenVectors -B build/eigenVectors
cmake --build build/eigenVectors
ctest --test-dir build/eigenVectors --output-on-failure
```

# GDB
```bash
gdb --args executablename arg1 arg2 arg3
run
bt
```

# Turn off GDB downloading separate debug info
```bash
echo "unset DEBUGINFOD_URLS" >> ~/.bashrc
```


# Test gstreamer
```bash
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=640,height=480,framerate=20/1 ! x264enc ! matroskamux ! filesink location=/home/pi/ssd/test.mkv
sudo gst-launch-1.0 v4l2src device=/dev/video0 ! video/x-raw,width=1280,height=720,framerate=30/1 ! videoconvert ! fbdevsink
```
