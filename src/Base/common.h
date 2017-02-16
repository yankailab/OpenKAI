#ifndef BASE_COMMON_H_
#define BASE_COMMON_H_

#include "../Config/switch.h"
#include "cv.h"
#include "tensorRT.h"
#include "platform.h"
#include "macro.h"
#include "constant.h"
#include <glog/logging.h>

//Common structures
namespace kai
{

struct vDouble4
{
	double m_x;
	double m_y;
	double m_z;
	double m_w;

	double midX(void)
	{
		return (m_x + m_z) * 0.5;
	}

	double midY(void)
	{
		return (m_y + m_w) * 0.5;
	}

	double area(void)
	{
		return abs((m_z - m_x) * (m_w - m_y));
	}

	void init(void)
	{
		m_x = 0.0;
		m_y = 0.0;
		m_z = 0.0;
		m_w = 0.0;
	}

	inline vDouble4 operator-(vDouble4& r)
    {
		vDouble4 v;
		v.m_x = m_x - r.m_x;
		v.m_y = m_y - r.m_y;
		v.m_z = m_z - r.m_z;
		v.m_w = m_w - r.m_w;
		return v;
	}

	inline vDouble4 operator+(vDouble4& r)
    {
		vDouble4 v;
		v.m_x = r.m_x + m_x;
		v.m_y = r.m_y + m_y;
		v.m_z = r.m_z + m_z;
		v.m_w = r.m_w + m_w;
		return v;
	}

	inline void operator-=(vDouble4& r)
    {
		m_x -= r.m_x;
		m_y -= r.m_y;
		m_z -= r.m_z;
		m_w -= r.m_w;
	}

	inline void operator+=(vDouble4& r)
    {
		m_x += r.m_x;
		m_y += r.m_y;
		m_z += r.m_z;
		m_w += r.m_w;
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

	inline vDouble3 operator-(vDouble3& r)
    {
		vDouble3 v;
		v.m_x = m_x - r.m_x;
		v.m_y = m_y - r.m_y;
		v.m_z = m_z - r.m_z;
		return v;
	}

	inline vDouble3 operator+(vDouble3& r)
    {
		vDouble3 v;
		v.m_x = m_x + r.m_x;
		v.m_y = m_y + r.m_y;
		v.m_z = m_z + r.m_z;
		return v;
	}

	inline void operator-=(vDouble3& r)
    {
		m_x -= r.m_x;
		m_y -= r.m_y;
		m_z -= r.m_z;
	}

	inline void operator+=(vDouble3& r)
    {
		m_x += r.m_x;
		m_y += r.m_y;
		m_z += r.m_z;
	}

	double len(void)
	{
		return sqrt(m_x*m_x + m_y*m_y + m_z*m_z);
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

	inline vDouble2 operator-(vDouble2& r)
    {
		vDouble2 v;
		v.m_x = m_x - r.m_x;
		v.m_y = m_y - r.m_y;
		return v;
	}

	inline vDouble2 operator+(vDouble2& r)
    {
		vDouble2 v;
		v.m_x = m_x + r.m_x;
		v.m_y = m_y + r.m_y;
		return v;
	}

	inline void operator-=(vDouble2& r)
    {
		m_x -= r.m_x;
		m_y -= r.m_y;
	}

	inline void operator+=(vDouble2& r)
    {
		m_x += r.m_x;
		m_y += r.m_y;
	}
};

struct vInt2
{
	int m_x;
	int m_y;

	int area(void)
	{
		return abs(m_x * m_y);
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
		return (m_x + m_z) / 2;
	}

	int midY(void)
	{
		return (m_y + m_w) / 2;
	}

	int area(void)
	{
		return abs((m_z - m_x) * (m_w - m_y));
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

