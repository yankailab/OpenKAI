
#define USE_CUDA

//#define USE_OPENCV2X
//#define USE_OPENCV_CONTRIB
#define USE_DARKNET
//#define USE_TENSORRT
//#define USE_CAFFE
//#define USE_ZED
//#define USE_ORB_SLAM2
//#define USE_SSD
//#define USE_CASCADE
//#define USE_CUDASTEREO

#ifdef USE_TENSORRT
#define USE_CUDA
#endif
