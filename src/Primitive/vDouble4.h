#ifndef OpenKAI_src_Primitive_vDouble4_H_
#define OpenKAI_src_Primitive_vDouble4_H_

#include "../Base/platform.h"
#include "vDouble2.h"

namespace kai
{

struct vDouble4
{
	double x;
	double y;
	double z;
	double w;

	vDouble4(void)
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
		w = 0.0;
	}

	vDouble4(double v)
	{
		x = v;
		y = v;
		z = v;
		w = v;
	}

	vDouble4(double a, double b, double c, double d)
	{
		x = a;
		y = b;
		z = c;
		w = d;
	}

	void init(void)
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
		w = 0.0;
	}

	void set(double v)
	{
		x = v;
		y = v;
		z = v;
		w = v;
	}

	void set(double a, double b, double c, double d)
	{
		x = a;
		y = b;
		z = c;
		w = d;
	}

	double midX(void)
	{
		return (x + z) * 0.5;
	}

	double midY(void)
	{
		return (y + w) * 0.5;
	}

	vDouble2 center(void)
	{
		vDouble2 vC;
		vC.x = midX();
		vC.y = midY();
		return vC;
	}

	double width(void)
	{
		return z - x;
	}

	double height(void)
	{
		return w - y;
	}

	double area(void)
	{
		return abs((z - x) * (w - y));
	}

	inline vDouble4 operator+(vDouble4& r)
    {
		vDouble4 v;
		v.x = r.x + x;
		v.y = r.y + y;
		v.z = r.z + z;
		v.w = r.w + w;
		return v;
	}

	inline vDouble4 operator-(vDouble4& r)
    {
		vDouble4 v;
		v.x = x - r.x;
		v.y = y - r.y;
		v.z = z - r.z;
		v.w = w - r.w;
		return v;
	}

	inline vDouble4 operator*(double r)
    {
		vDouble4 v;
		v.x = x * r;
		v.y = y * r;
		v.z = z * r;
		v.w = w * r;
		return v;
	}

	inline vDouble4 operator/(double r)
    {
		vDouble4 v;
		v.x = x / r;
		v.y = y / r;
		v.z = z / r;
		v.w = w / r;
		return v;
	}

	inline void operator=(double r)
    {
		x = r;
		y = r;
		z = r;
		w = r;
	}

	inline void operator+=(vDouble4& r)
    {
		x += r.x;
		y += r.y;
		z += r.z;
		w += r.w;
	}

	inline void operator-=(vDouble4& r)
    {
		x -= r.x;
		y -= r.y;
		z -= r.z;
		w -= r.w;
	}

	inline void operator*=(double r)
    {
		x *= r;
		y *= r;
		z *= r;
		w *= r;
	}

	inline void operator/=(double r)
    {
		x /= r;
		y /= r;
		z /= r;
		w /= r;
	}

	double len(void)
	{
		return sqrt(x*x + y*y + z*z + w*w);
	}
};


}
#endif
