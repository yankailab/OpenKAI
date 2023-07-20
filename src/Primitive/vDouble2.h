#ifndef OpenKAI_src_Primitive_vDouble2_H_
#define OpenKAI_src_Primitive_vDouble2_H_

#include "../Base/platform.h"

namespace kai
{

struct vDouble2
{
	double x;
	double y;

	vDouble2(void)
	{
		x = 0.0;
		y = 0.0;
	}

	vDouble2(double v)
	{
		x = v;
		y = v;
	}

	vDouble2(double a, double b)
	{
		x = a;
		y = b;
	}

	void clear(void)
	{
		x = 0.0;
		y = 0.0;
	}

	void set(double v)
	{
		x = v;
		y = v;
	}

	void set(double a, double b)
	{
		x = a;
		y = b;
	}

	inline vDouble2 operator+(vDouble2& r)
    {
		vDouble2 v;
		v.x = x + r.x;
		v.y = y + r.y;
		return v;
	}

	inline vDouble2 operator-(vDouble2& r)
    {
		vDouble2 v;
		v.x = x - r.x;
		v.y = y - r.y;
		return v;
	}

	inline vDouble2 operator*(double r)
    {
		vDouble2 v;
		v.x = x * r;
		v.y = y * r;
		return v;
	}

	inline vDouble2 operator/(double r)
    {
		vDouble2 v;
		v.x = x / r;
		v.y = y / r;
		return v;
	}

	inline void operator=(double r)
    {
		x = r;
		y = r;
	}

	inline void operator+=(vDouble2& r)
    {
		x += r.x;
		y += r.y;
	}

	inline void operator-=(vDouble2& r)
    {
		x -= r.x;
		y -= r.y;
	}

	inline void operator*=(double r)
    {
		x *= r;
		y *= r;
	}

	inline void operator/=(double r)
    {
		x /= r;
		y /= r;
	}

	double mid(void)
	{
		return (x + y) * 0.5;
	}

	double len(void)
	{
		return sqrt(x*x + y*y);
	}

	bool bEmpty(void)
	{
		return ((x+y)==0);
	}

	bool bInside(double v)
    {
        IF_F(v < x);
        IF_F(v >= y);
        return true;
    }
};

}
#endif
