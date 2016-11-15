#ifndef BASE_COMMON_H_
#define BASE_COMMON_H_

#include "Module.h"
#include "cv.h"
#include "platform.h"
#include <glog/logging.h>
#include "../Utility/util.h"
#include "../Base/BASE.h"

#define MAVLINK_DEBUG

//Common include
#define PI 3.141592653589793
#define OneOvPI 0.31830988618
#define DEG_RADIAN 0.0174533
#define USEC_1SEC 1000000
#define DEFAULT_FPS 30

#define CR '\x0d'
#define LF '\x0a'

using namespace std;

#define F_FATAL_F(x) if(x==false){LOG(FATAL);return false;}
#define F_ERROR_F(x) if(x==false){LOG(ERROR);return false;}
#define F_INFO(x) if(x==false){LOG(INFO);}
#define F_(x) if(x==false){return;}

#define CHECK_T(x) if(x){return true;}
#define CHECK_F(x) if(x){return false;}
#define CHECK_N(x) if(x){return NULL;}
#define CHECK_(x) if(x){return;}

#define NULL_(x) if(x==NULL){return;}
#define NULL_F(x) if(x==NULL){return false;}
#define NULL_N(x) if(x==NULL){return NULL;}
#define NULL_T(x) if(x==NULL){return true;}

#define DEL(x) if(x){delete x;}
#define SWAP(x,y,t) t=x;x=y;y=t;

namespace kai
{

struct vDouble4
{
	double m_x;
	double m_y;
	double m_z;
	double m_w;
};

struct vDouble3
{
	double m_x;
	double m_y;
	double m_z;
};

struct vDouble2
{
	double m_x;
	double m_y;
};

struct vInt2
{
	int m_x;
	int m_y;
};

struct vInt3
{
	int m_x;
	int m_y;
	int m_z;
};

struct vInt4
{
	int m_x;
	int m_y;
	int m_z;
	int m_w;
};

struct MOUSE
{
	int m_event;
	int m_x;
	int m_y;
	int m_flags;
};

}

#endif



