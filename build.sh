INCLUDES="-I/home/ubuntu/Downloads/caffe/include -I/home/ubuntu/Downloads/caffe/build/src -I/usr/local/cuda/include -I/usr/local/include"
OPTIONS="-O3 -w -Wall -c -fmessage-length=0 -MMD -MP"
LINKERS="-L/usr/local/lib -L/home/ubuntu/Downloads/caffe/build/lib"

g++ $INCLUDES $OPTIONS -MF"build/NNClassifier.d" -MT"build/NNClassifier.d" -o "build/NNClassifier.o" "src/AI/NNClassifier.cpp"

g++ $INCLUDES $OPTIONS -MF"build/MavlinkInterface.d" -MT"build/MavlinkInterface.d" -o "build/MavlinkInterface.o" "src/Interface/MavlinkInterface.cpp"

g++ $INCLUDES $OPTIONS -MF"build/VehicleInterface.d" -MT"build/VehicleInterface.d" -o "build/VehicleInterface.o" "src/Interface/VehicleInterface.cpp"

g++ $INCLUDES $OPTIONS -MF"build/FileIO.d" -MT"build/FileIO.d" -o "build/FileIO.o" "src/IO/FileIO.cpp"

g++ $INCLUDES $OPTIONS -MF"build/JSON.d" -MT"build/JSON.d" -o "build/JSON.o" "src/IO/JSON.cpp"

g++ $INCLUDES $OPTIONS -MF"build/SerialPort.d" -MT"build/SerialPort.d" -o "build/SerialPort.o" "src/IO/SerialPort.cpp"

g++ $INCLUDES $OPTIONS -MF"build/FastDetector.d" -MT"build/FastDetector.d" -o "build/FastDetector.o" "src/Detector/FastDetector.cpp"

g++ $INCLUDES $OPTIONS -MF"build/ObjectDetector.d" -MT"build/ObjectDetector.d" -o "build/ObjectDetector.o" "src/Detector/ObjectDetector.cpp"

g++ $INCLUDES $OPTIONS -MF"build/OpticalFlowDetector.d" -MT"build/OpticalFlowDetector.d" -o "build/OpticalFlowDetector.o" "src/Detector/OpticalFlowDetector.cpp"

g++ $INCLUDES $OPTIONS -MF"build/CamDenseFlow.d" -MT"build/CamDenseFlow.d" -o "build/CamDenseFlow.o" "src/Camera/CamDenseFlow.cpp"

g++ $INCLUDES $OPTIONS -MF"build/CamFrame.d" -MT"build/CamFrame.d" -o "build/CamFrame.o" "src/Camera/CamFrame.cpp"

g++ $INCLUDES $OPTIONS -MF"build/CamInput.d" -MT"build/CamInput.d" -o "build/CamInput.o" "src/Camera/CamInput.cpp"

g++ $INCLUDES $OPTIONS -MF"build/MarkerDetector.d" -MT"build/MarkerDetector.d" -o "build/MarkerDetector.o" "src/Detector/MarkerDetector.cpp"

g++ $INCLUDES $OPTIONS -MF"build/UIMonitor.d" -MT"build/UIMonitor.d" -o "build/UIMonitor.o" "src/UI/UIMonitor.cpp"

g++ $INCLUDES $OPTIONS -MF"build/CamSparseFlow.d" -MT"build/CamSparseFlow.d" -o "build/CamSparseFlow.o" "src/Camera/CamSparseFlow.cpp"

g++ $INCLUDES $OPTIONS -MF"build/CamStereo.d" -MT"build/CamStereo.d" -o "build/CamStereo.o" "src/Camera/CamStereo.cpp"

g++ $INCLUDES $OPTIONS -MF"build/CamStream.d" -MT"build/CamStream.d" -o "build/CamStream.o" "src/Camera/CamStream.cpp"

g++ $INCLUDES $OPTIONS -MF"build/ThreadBase.d" -MT"build/ThreadBase.d" -o "build/ThreadBase.o" "src/Base/ThreadBase.cpp"

g++ $INCLUDES $OPTIONS -MF"build/DetectorBase.d" -MT"build/DetectorBase.d" -o "build/DetectorBase.o" "src/Detector/DetectorBase.cpp"

g++ $INCLUDES $OPTIONS -MF"build/AutoPilot.d" -MT"build/AutoPilot.d" -o "build/AutoPilot.o" "src/Autopilot/AutoPilot.cpp"

g++ $INCLUDES $OPTIONS -MF"build/Filter.d" -MT"build/Filter.d" -o "build/Filter.o" "src/Algorithm/Filter.cpp"

g++ $INCLUDES $OPTIONS -MF"build/demo.d" -MT"build/demo.d" -o "build/demo.o" "src/demo.cpp"

g++ $LINKERS -o "LAB"  ./build/MavlinkInterface.o ./build/VehicleInterface.o ./build/FileIO.o ./build/JSON.o ./build/SerialPort.o  ./build/FastDetector.o ./build/ObjectDetector.o ./build/OpticalFlowDetector.o  ./build/CamDenseFlow.o ./build/CamFrame.o ./build/CamInput.o ./build/MarkerDetector.o ./build/UIMonitor.o ./build/CamSparseFlow.o ./build/CamStereo.o ./build/CamStream.o ./build/ThreadBase.o  build/DetectorBase.o  ./build/AutoPilot.o  ./build/Filter.o  ./build/NNClassifier.o  ./build/demo.o   -lpthread -lboost_system -lopencv_imgproc -lopencv_cudawarping -lopencv_cudaobjdetect -lglog -lcaffe -lopencv_videoio -lopencv_core -lopencv_cudaarithm -lopencv_cudabgsegm -lopencv_cudaimgproc -lopencv_calib3d -lopencv_cudaoptflow -lopencv_imgcodecs -lopencv_highgui -lopencv_cudastereo -lopencv_saliency

