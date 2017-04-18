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
	double x;
	double y;
	double z;
	double w;

	double midX(void)
	{
		return (x + z) * 0.5;
	}

	double midY(void)
	{
		return (y + w) * 0.5;
	}

	double area(void)
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
};

struct vDouble2
{
	double x;
	double y;

	void init(void)
	{
		x = 0.0;
		y = 0.0;
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

	double len(void)
	{
		return sqrt(x*x + y*y);
	}
};

struct vInt2
{
	int x;
	int y;

	int area(void)
	{
		return abs(x * y);
	}

	void init(void)
	{
		x = 0;
		y = 0;
	}
};

struct vInt3
{
	int x;
	int y;
	int z;

	void init(void)
	{
		x = 0;
		y = 0;
		z = 0;
	}
};

struct vInt4
{
	int x;
	int y;
	int z;
	int w;

	int midX(void)
	{
		return (x + z) / 2;
	}

	int midY(void)
	{
		return (y + w) / 2;
	}

	int area(void)
	{
		return abs((z - x) * (w - y));
	}

	int width(void)
	{
		return z - x;
	}

	int height(void)
	{
		return w - y;
	}

	void init(void)
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
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

