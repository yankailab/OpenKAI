#ifndef OpenKAI_src_Primitive_vFloat3_H_
#define OpenKAI_src_Primitive_vFloat3_H_

namespace kai
{

struct vFloat3
{
	float x;
	float y;
	float z;

	vFloat3(void)
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}

	vFloat3(float v)
	{
		x = v;
		y = v;
		z = v;
	}

	vFloat3(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}

	void init(void)
	{
		x = 0.0;
		y = 0.0;
		z = 0.0;
	}

	void init(float v)
	{
		x = v;
		y = v;
		z = v;
	}

	void init(float a, float b, float c)
	{
		x = a;
		y = b;
		z = c;
	}

	inline vFloat3 operator+(vFloat3 r)
    {
		vFloat3 v;
		v.x = r.x + x;
		v.y = r.y + y;
		v.z = r.z + z;
		return v;
	}

	inline vFloat3 operator-(vFloat3 r)
    {
		vFloat3 v;
		v.x = x - r.x;
		v.y = y - r.y;
		v.z = z - r.z;
		return v;
	}

	inline vFloat3 operator*(float r)
    {
		vFloat3 v;
		v.x = x * r;
		v.y = y * r;
		v.z = z * r;
		return v;
	}

	inline vFloat3 operator/(float r)
    {
		float rb = 1.0/r;
		vFloat3 v;
		v.x = x * rb;
		v.y = y * rb;
		v.z = z * rb;
		return v;
	}

	inline void operator=(float r)
    {
		x = r;
		y = r;
		z = r;
	}

	inline void operator=(Eigen::Vector3d& r)
    {
		x = r.x();
		y = r.y();
		z = r.z();
	}

	inline void operator+=(vFloat3 r)
    {
		x += r.x;
		y += r.y;
		z += r.z;
	}

	inline void operator-=(vFloat3 r)
    {
		x -= r.x;
		y -= r.y;
		z -= r.z;
	}

	inline void operator*=(float r)
    {
		x *= r;
		y *= r;
		z *= r;
	}

	inline void operator/=(float r)
    {
		x /= r;
		y /= r;
		z /= r;
	}

	inline bool operator!=(vFloat3 r)
    {
		IF_T(x != r.x);
		IF_T(y != r.y);
		IF_T(z != r.z);

		return false;
	}

	inline bool operator==(vFloat3 r)
    {
		IF_F(x != r.x);
		IF_F(y != r.y);
		IF_F(z != r.z);

		return true;
	}

	float len(void)
	{
		return sqrt(x*x + y*y + z*z);
	}

	void constrain(float a, float b)
	{
		x = (x<a)?a:x;
		x = (x>b)?b:x;
		y = (y<a)?a:y;
		y = (y>b)?b:y;
		z = (z<a)?a:z;
		z = (z>b)?b:z;
	}

	float v(int i)
	{
		IF__(i>=3, 0);
		IF__(i<0, 0);

		float* pv[3];
		pv[0] = &x;
		pv[1] = &y;
		pv[2] = &z;

		return *pv[i];
	}

	bool bEqual(vFloat3& v, float d)
	{
		IF_F(abs(x - v.x) > d);
		IF_F(abs(y - v.y) > d);
		IF_F(abs(z - v.z) > d);

		return true;
	}

	float r(void)
	{
		return sqrt((x*x) + (y*y) + (z*z));
	}
};

}
#endif
