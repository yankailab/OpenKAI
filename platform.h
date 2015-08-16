//#define PLATFORM_WIN
//#define PLATFORM_ANDROID
#define PLATFORM_UBUNTU
//#define PLATFORM_MAC

#define USE_CUDA


#ifdef PLATFORM_WIN
#endif

#ifdef PLATFORM_UBUNTU
#include <sys/time.h>
#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif
