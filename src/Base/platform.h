//#define PLATFORM_WIN
//#define PLATFORM_ANDROID
#define PLATFORM_UBUNTU
//#define PLATFORM_MAC

#define USE_CUDA
//#define USE_OPENCL

#define USE_ZED


#ifdef PLATFORM_WIN
#endif

#ifdef PLATFORM_UBUNTU
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>   /* Standard types */
#include <fcntl.h>
#include <fstream>
#include <termios.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <getopt.h>

#include <pthread.h>
#include <string>
#include <string.h>

#endif
