#pragma once
#include "platform.h"
#include "defaultparam.h"
#include <glog/logging.h>
#include "../IO/FileIO.h"
#include "../IO/JSON.h"
#include "ThreadBase.h"
#include "../Algorithm/Filter.h"

//Demo App

//#define MARKER_COPTER
#define OBJECT_DETECT


//Common include

#define MAX_FRAME_ID 0xffffffff

using namespace std;

#define CHECK_FATAL(x) if(x==false){LOG(FATAL);return false;}
#define CHECK_ERROR(x) if(x==false){LOG(ERROR);return false;}
#define CHECK_INFO(x) if(x==false){LOG(INFO);}

#define RELEASE(x) if(x){delete x;}

namespace kai
{

struct fVector4
{
	double m_x;
	double m_y;
	double m_z;
	double m_w;
};

struct fVector3
{
	double m_x;
	double m_y;
	double m_z;
};

struct fVector2
{
	double m_x;
	double m_y;
};

struct iVector2
{
	int m_x;
	int m_y;
};

struct iVector3
{
	int m_x;
	int m_y;
	int m_z;
};

struct iVector4
{
	int m_x;
	int m_y;
	int m_z;
	int m_w;
};

struct CAMERA_DATA
{
	bool m_bBall;



};

struct SYSTEM
{
	char m_mode;
};

}



