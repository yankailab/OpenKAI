#ifndef OpenKAI_src_Primitive_vFloat2_H_
#define OpenKAI_src_Primitive_vFloat2_H_

namespace kai
{

struct vFloat2
{
	float x;
	float y;

	void init(void)
	{
		x = 0.0;
		y = 0.0;
	}

	void init(float v)
	{
		x = v;
		y = v;
	}

	void init(float a, float b)
	{
		x = a;
		y = b;
	}

	inline vFloat2 operator+(vFloat2& r)
    {
		vFloat2 v;
		v.x = x + r.x;
		v.y = y + r.y;
		return v;
	}

	inline vFloat2 operator-(vFloat2& r)
    {
		vFloat2 v;
		v.x = x - r.x;
		v.y = y - r.y;
		return v;
	}

	inline vFloat2 operator*(float r)
    {
		vFloat2 v;
		v.x = x * r;
		v.y = y * r;
		return v;
	}

	inline vFloat2 operator/(float r)
    {
		vFloat2 v;
		v.x = x / r;
		v.y = y / r;
		return v;
	}

	inline void operator=(float r)
    {
		x = r;
		y = r;
	}

	inline void operator+=(vFloat2& r)
    {
		x += r.x;
		y += r.y;
	}

	inline void operator-=(vFloat2& r)
    {
		x -= r.x;
		y -= r.y;
	}

	inline void operator*=(float r)
    {
		x *= r;
		y *= r;
	}

	inline void operator/=(float r)
    {
		x /= r;
		y /= r;
	}

	float mid(void)
	{
		return (x + y) * 0.5;
	}

	float len(void)
	{
		return abs(y-x);
	}

	float d(void)
	{
		return y-x;
	}

	float r(void)
	{
		return sqrt((x*x) + (y*y));
	}

	float constrain(float v)
	{
		if (v > y)
			v = y;
		else if (v < x)
			v = x;

		return v;
	}

};

}
#endif
