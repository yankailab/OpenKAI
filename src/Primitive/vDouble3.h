#ifndef OpenKAI_src_Primitive_vDouble3_H_
#define OpenKAI_src_Primitive_vDouble3_H_

#include "../Base/platform.h"

namespace kai
{

struct vDouble3
{
	double x;
	double y;
	double z;

	void init(void)
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}

	void init(double v)
	{
		x = v;
		y = v;
		z = v;
	}

	void init(double a, double b, double c)
	{
		x = a;
		y = b;
		z = c;
	}

	inline vDouble3 operator+(vDouble3& r)
    {
		vDouble3 v;
		v.x = x + r.x;
		v.y = y + r.y;
		v.z = z + r.z;
		return v;
	}

	inline vDouble3 operator-(vDouble3& r)
    {
		vDouble3 v;
		v.x = x - r.x;
		v.y = y - r.y;
		v.z = z - r.z;
		return v;
	}

	inline vDouble3 operator*(double r)
    {
		vDouble3 v;
		v.x = x * r;
		v.y = y * r;
		v.z = z * r;
		return v;
	}

	inline vDouble3 operator/(double r)
    {
		vDouble3 v;
		v.x = x / r;
		v.y = y / r;
		v.z = z / r;
		return v;
	}

	inline void operator=(double r)
    {
		x = r;
		y = r;
		z = r;
	}

	inline void operator+=(vDouble3& r)
    {
		x += r.x;
		y += r.y;
		z += r.z;
	}

	inline void operator-=(vDouble3& r)
    {
		x -= r.x;
		y -= r.y;
		z -= r.z;
	}

	inline void operator*=(double r)
    {
		x *= r;
		y *= r;
		z *= r;
	}

	inline void operator/=(double r)
    {
		x /= r;
		y /= r;
		z /= r;
	}

	double len(void)
	{
		return sqrt(x*x + y*y + z*z);
	}

	double sum(void)
	{
		return x+y+z;
	}

	Eigen::Vector3d v3d(void)
	{
		return Eigen::Vector3d(x,y,z);
	}

};

}
#endif
