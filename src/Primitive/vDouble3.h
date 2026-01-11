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

		vDouble3(void)
		{
			x = 0.0;
			y = 0.0;
			z = 0.0;
		}

		vDouble3(double v)
		{
			x = v;
			y = v;
			z = v;
		}

		vDouble3(double a, double b, double c)
		{
			x = a;
			y = b;
			z = c;
		}

		void clear(void)
		{
			x = 0.0;
			y = 0.0;
			z = 0.0;
		}

		void set(double v)
		{
			x = v;
			y = v;
			z = v;
		}

		void set(double a, double b, double c)
		{
			x = a;
			y = b;
			z = c;
		}

		inline double &operator[](int i)
		{
			return (&x)[i];
		}

		inline vDouble3 operator+(vDouble3 &r)
		{
			vDouble3 v;
			v.x = x + r.x;
			v.y = y + r.y;
			v.z = z + r.z;
			return v;
		}

		inline vDouble3 operator-(vDouble3 &r)
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

		inline void operator=(Eigen::Vector3d r)
		{
			x = r[0];
			y = r[1];
			z = r[2];
		}

		inline void operator=(const vector<double>& v)
		{
			int n = v.size();

			if (n >= 1)
				x = v[0];

			if (n >= 2)
				y = v[1];

			if (n >= 3)
				z = v[2];
		}

		inline void operator+=(vDouble3 &r)
		{
			x += r.x;
			y += r.y;
			z += r.z;
		}

		inline void operator-=(vDouble3 &r)
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

		inline bool operator!=(vDouble3 r)
		{
			IF__(x != r.x, true);
			IF__(y != r.y, true);
			IF__(z != r.z, true);

			return false;
		}

		inline bool operator==(vDouble3 r)
		{
			IF_F(x != r.x);
			IF_F(y != r.y);
			IF_F(z != r.z);

			return true;
		}

		Eigen::Vector3d v3d(void)
		{
			return Eigen::Vector3d(x, y, z);
		}

		double len(void)
		{
			return sqrt(x * x + y * y + z * z);
		}

		bool bEqual(vDouble3 &v, float d)
		{
			IF_F(abs(x - v.x) > d);
			IF_F(abs(y - v.y) > d);
			IF_F(abs(z - v.z) > d);

			return true;
		}
	};

}
#endif
