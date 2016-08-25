INCLUDES="-I/home/ubuntu/src/caffe_ssd/caffe/include -I/home/ubuntu/src/caffe_ssd/caffe/build/src -I/usr/local/cuda/include -I/usr/local/include -I/usr/local/opencv3/include"
OPTIONS="-O3 -w -Wall -c -fmessage-length=0 -MMD -MP"
LINKERS="-L/usr/local/lib -L/usr/local/opencv3/lib -L/home/ubuntu/src/caffe_ssd/caffe/build/lib -L/home/ubuntu/src/caffe_ssd/caffe/build"

rm OpenKAI
rm -rf build
mkdir build

g++ $INCLUDES $OPTIONS -MF"build/_Caffe.d" -MT"build/_Caffe.d" -o "build/_Caffe.o" "src/AI/_Caffe.cpp"
g++ $INCLUDES $OPTIONS -MF"build/_FCN.d" -MT"build/_FCN.d" -o "build/_FCN.o" "src/AI/_FCN.cpp"
g++ $INCLUDES $OPTIONS -MF"build/_SSD.d" -MT"build/_SSD.d" -o "build/_SSD.o" "src/AI/_SSD.cpp"

g++ $INCLUDES $OPTIONS -MF"build/Filter.d" -MT"build/Filter.d" -o "build/Filter.o" "src/Algorithm/Filter.cpp"

g++ $INCLUDES $OPTIONS -MF"build/AppBase.d" -MT"build/AppBase.d" -o "build/AppBase.o" "src/Application/AppBase.cpp"
g++ $INCLUDES $OPTIONS -MF"build/VisualFollow.d" -MT"build/VisualFollow.d" -o "build/VisualFollow.o" "src/Application/VisualFollow.cpp"
g++ $INCLUDES $OPTIONS -MF"build/Navigator.d" -MT"build/Navigator.d" -o "build/Navigator.o" "src/Application/Navigator.cpp"

g++ $INCLUDES $OPTIONS -MF"build/_AutoPilot.d" -MT"build/_AutoPilot.d" -o "build/_AutoPilot.o" "src/Autopilot/_AutoPilot.cpp"

g++ $INCLUDES $OPTIONS -MF"build/_ThreadBase.d" -MT"build/_ThreadBase.d" -o "build/_ThreadBase.o" "src/Base/_ThreadBase.cpp"

g++ $INCLUDES $OPTIONS -MF"build/_Stream.d" -MT"build/_Stream.d" -o "build/_Stream.o" "src/Camera/_Stream.cpp"
g++ $INCLUDES $OPTIONS -MF"build/Frame.d" -MT"build/Frame.d" -o "build/Frame.o" "src/Camera/Frame.cpp"
g++ $INCLUDES $OPTIONS -MF"build/Camera.d" -MT"build/Camera.d" -o "build/Camera.o" "src/Camera/Camera.cpp"
g++ $INCLUDES $OPTIONS -MF"build/FrameGroup.d" -MT"build/FrameGroup.d" -o "build/FrameGroup.o" "src/Camera/FrameGroup.cpp"

g++ $INCLUDES $OPTIONS -MF"build/_BgFgDetector.d" -MT"build/_BgFgDetector.d" -o "build/_BgFgDetector.o" "src/Detector/_BgFgDetector.cpp"
g++ $INCLUDES $OPTIONS -MF"build/_CascadeDetector.d" -MT"build/_CascadeDetector.d" -o "build/_CascadeDetector.o" "src/Detector/_CascadeDetector.cpp"
g++ $INCLUDES $OPTIONS -MF"build/DetectorBase.d" -MT"build/DetectorBase.d" -o "build/DetectorBase.o" "src/Detector/DetectorBase.cpp"
g++ $INCLUDES $OPTIONS -MF"build/_MarkerDetector.d" -MT"build/_MarkerDetector.d" -o "build/_MarkerDetector.o" "src/Detector/_MarkerDetector.cpp"
g++ $INCLUDES $OPTIONS -MF"build/_DepthDetector.d" -MT"build/_DepthDetector.d" -o "build/_DepthDetector.o" "src/Detector/_DepthDetector.cpp"

g++ $INCLUDES $OPTIONS -MF"build/_Flow.d" -MT"build/_Flow.d" -o "build/_Flow.o" "src/Vision/_Flow.cpp"

g++ $INCLUDES $OPTIONS -MF"build/_Mavlink.d" -MT"build/_Mavlink.d" -o "build/_Mavlink.o" "src/Interface/_Mavlink.cpp"
g++ $INCLUDES $OPTIONS -MF"build/_RC.d" -MT"build/_RC.d" -o "build/_RC.o" "src/Interface/_RC.cpp"

g++ $INCLUDES $OPTIONS -MF"build/FileIO.d" -MT"build/FileIO.d" -o "build/FileIO.o" "src/IO/FileIO.cpp"
g++ $INCLUDES $OPTIONS -MF"build/JSON.d" -MT"build/JSON.d" -o "build/JSON.o" "src/IO/JSON.cpp"
g++ $INCLUDES $OPTIONS -MF"build/SerialPort.d" -MT"build/SerialPort.d" -o "build/SerialPort.o" "src/IO/SerialPort.cpp"

g++ $INCLUDES $OPTIONS -MF"build/_Universe.d" -MT"build/_Universe.d" -o "build/_Universe.o" "src/Navigation/_Universe.cpp"

g++ $INCLUDES $OPTIONS -MF"build/_ROITracker.d" -MT"build/_ROITracker.d" -o "build/_ROITracker.o" "src/Tracker/_ROITracker.cpp"

g++ $INCLUDES $OPTIONS -MF"build/demo.d" -MT"build/demo.d" -o "build/demo.o" "src/demo.cpp"

g++ $LINKERS -o "OpenKAI" ./build/_Caffe.o ./build/_FCN.o ./build/_SSD.o ./build/Filter.o ./build/AppBase.o ./build/VisualFollow.o ./build/Navigator.o ./build/_AutoPilot.o ./build/_ThreadBase.o ./build/_Stream.o ./build/Frame.o ./build/Camera.o ./build/FrameGroup.o ./build/_BgFgDetector.o ./build/_CascadeDetector.o ./build/DetectorBase.o ./build/_MarkerDetector.o ./build/_DepthDetector.o ./build/_Flow.o ./build/_Mavlink.o ./build/_RC.o ./build/FileIO.o ./build/JSON.o ./build/SerialPort.o ./build/_Universe.o ./build/_ROITracker.o ./build/demo.o -lpthread -lboost_system -lopencv_imgproc -lopencv_cudawarping -lopencv_cudaobjdetect -lglog -lcaffe -lopencv_videoio -lopencv_core -lopencv_cudaarithm -lopencv_cudabgsegm -lopencv_cudaimgproc -lopencv_calib3d -lopencv_cudaoptflow -lopencv_imgcodecs -lopencv_highgui -lopencv_cudastereo -lopencv_saliency -lopencv_objdetect -lopencv_xobjdetect -lopencv_tracking -lopencv_features2d

