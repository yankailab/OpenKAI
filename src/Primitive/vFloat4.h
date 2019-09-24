#ifndef OpenKAI_src_Primitive_vFloat4_H_
#define OpenKAI_src_Primitive_vFloat4_H_

#include "data.h"
#include "../Base/platform.h"

namespace kai
{

struct vFloat4
{
	float x;
	float y;
	float z;
	float w;

	float midX(void)
	{
		return (x + z) * 0.5;
	}

	float midY(void)
	{
		return (y + w) * 0.5;
	}

	vFloat2 center(void)
	{
		vFloat2 vC;
		vC.x = midX();
		vC.y = midY();
		return vC;
	}

	float width(void)
	{
		return z - x;
	}

	float height(void)
	{
		return w - y;
	}

	float area(void)
	{
		return abs((z - x) * (w - y));
	}

	void init(void)
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
		w = 0.0;
	}

	void init(float v)
	{
		x = v;
		y = v;
		z = v;
		w = v;
	}

	void init(float a, float b, float c, float d)
	{
		x = a;
		y = b;
		z = c;
		w = d;
	}

	inline vFloat4 operator+(vFloat4& r)
    {
		vFloat4 v;
		v.x = r.x + x;
		v.y = r.y + y;
		v.z = r.z + z;
		v.w = r.w + w;
		return v;
	}

	inline vFloat4 operator-(vFloat4& r)
    {
		vFloat4 v;
		v.x = x - r.x;
		v.y = y - r.y;
		v.z = z - r.z;
		v.w = w - r.w;
		return v;
	}

	inline vFloat4 operator*(float r)
    {
		vFloat4 v;
		v.x = x * r;
		v.y = y * r;
		v.z = z * r;
		v.w = w * r;
		return v;
	}

	inline vFloat4 operator/(float r)
    {
		vFloat4 v;
		v.x = x / r;
		v.y = y / r;
		v.z = z / r;
		v.w = w / r;
		return v;
	}

	inline void operator=(float r)
    {
		x = r;
		y = r;
		z = r;
		w = r;
	}

	inline void operator+=(vFloat4& r)
    {
		x += r.x;
		y += r.y;
		z += r.z;
		w += r.w;
	}

	inline void operator-=(vFloat4& r)
    {
		x -= r.x;
		y -= r.y;
		z -= r.z;
		w -= r.w;
	}

	inline void operator*=(float r)
    {
		x *= r;
		y *= r;
		z *= r;
		w *= r;
	}

	inline void operator/=(float r)
    {
		x /= r;
		y /= r;
		z /= r;
		w /= r;
	}

	float len(void)
	{
		return sqrt(x*x + y*y + z*z + w*w);
	}

	void constrain(float a, float b)
	{
		x = (x<a)?a:x;
		x = (x>b)?b:x;
		y = (y<a)?a:y;
		y = (y>b)?b:y;
		z = (z<a)?a:z;
		z = (z>b)?b:z;
		w = (w<a)?a:w;
		w = (w>b)?b:w;
	}
};

}
#endif
