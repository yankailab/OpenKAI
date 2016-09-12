#ifndef BASE_COMMON_H_
#define BASE_COMMON_H_

#include "platform.h"
#include <glog/logging.h>
#include "../IO/FileIO.h"
#include "../Script/Config.h"

#define MAVLINK_DEBUG


//Common include
#define PI 3.141592653589793
#define OneOvPI 0.31830988618
#define DEG_RADIAN 0.0174533

#define OBJ_VACANT 0
#define OBJ_ADDED 1
#define OBJ_CLASSIFYING 2
#define OBJ_COMPLETE 3

#define DEFAULT_FPS 30


using namespace std;

#define CHECK_FATAL(x) if(x==false){LOG(FATAL);return false;}
#define CHECK_ERROR(x) if(x==false){LOG(ERROR);return false;}
#define CHECK_INFO(x) if(x==false){LOG(INFO);}

#define RELEASE(x) if(x){delete x;}
#define STOP(x) if(x){x->stop();}
#define COMPLETE(x) if(x){x->complete();}

#define SWAP(x,y,t) t=x;x=y;y=t;

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

}

#endif



