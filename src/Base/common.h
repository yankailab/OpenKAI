#ifndef BASE_COMMON_H_
#define BASE_COMMON_H_

#include "../Config/switch.h"
#include "cv.h"
#include "tensorRT.h"
#include "platform.h"
#include "macro.h"
#include "constant.h"
#include <glog/logging.h>
#include "../include/UTM.h"

//Common structures
namespace kai
{

struct GPS_POSITION
{
	double m_lng;
	double m_lat;

	double m_UTMeasting;
	double m_UTMnorthing;
	string m_UTMzone;

	void init(void)
	{
		m_lng = 0.0;
		m_lat = 0.0;
		m_UTMeasting = 0.0;
		m_UTMnorthing = 0.0;
		m_UTMzone = "";
	}

	void LL2UTM(void)
	{
		char pUTMzone[8];
		UTM::LLtoUTM(m_lat,m_lng,m_UTMnorthing,m_UTMeasting,pUTMzone);
		m_UTMzone = pUTMzone;
	}

	void UTM2LL(void)
	{
		UTM::UTMtoLL(m_UTMnorthing, m_UTMeasting, m_UTMzone.c_str(), m_lat, m_lng);
	}
};

struct vDouble4
{
	double m_x;
	double m_y;
	double m_z;
	double m_w;

	double midX(void)
	{
		return (m_x+m_z)*0.5;
	}

	double midY(void)
	{
		return (m_y+m_w)*0.5;
	}

	double area(void)
	{
		return abs((m_z-m_x)*(m_w-m_y));
	}

	void init(void)
	{
		m_x = 0.0;
		m_y = 0.0;
		m_z = 0.0;
		m_w = 0.0;
	}
};

struct vDouble3
{
	double m_x;
	double m_y;
	double m_z;

	void init(void)
	{
		m_x = 0.0;
		m_y = 0.0;
		m_z = 0.0;
	}
};

struct vDouble2
{
	double m_x;
	double m_y;

	void init(void)
	{
		m_x = 0.0;
		m_y = 0.0;
	}
};

struct vInt2
{
	int m_x;
	int m_y;

	int area(void)
	{
		return abs(m_x*m_y);
	}

	void init(void)
	{
		m_x = 0;
		m_y = 0;
	}
};

struct vInt3
{
	int m_x;
	int m_y;
	int m_z;

	void init(void)
	{
		m_x = 0;
		m_y = 0;
		m_z = 0;
	}
};

struct vInt4
{
	int m_x;
	int m_y;
	int m_z;
	int m_w;

	int midX(void)
	{
		return (m_x+m_z)/2;
	}

	int midY(void)
	{
		return (m_y+m_w)/2;
	}

	int area(void)
	{
		return abs((m_z-m_x)*(m_w-m_y));
	}

	void init(void)
	{
		m_x = 0;
		m_y = 0;
		m_z = 0;
		m_w = 0;
	}

};

struct MOUSE
{
	int m_event;
	int m_x;
	int m_y;
	int m_flags;
};

}

//Common utility functions
#include "../Utility/util.h"

#endif



