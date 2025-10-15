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

		void clear(void)
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

		inline double &operator[](int i)
		{
			return (&x)[i];
		}

		inline vDouble4 operator+(vDouble4 &r)
		{
			vDouble4 v;
			v.x = x + r.x;
			v.y = y + r.y;
			v.z = z + r.z;
			v.w = w + r.w;
			return v;
		}

		inline vDouble4 operator-(vDouble4 &r)
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

		inline void operator=(Eigen::Vector4d r)
		{
			x = r[0];
			y = r[1];
			z = r[2];
			w = r[3];
		}

		inline void operator+=(vDouble4 &r)
		{
			x += r.x;
			y += r.y;
			z += r.z;
			w += r.w;
		}

		inline void operator-=(vDouble4 &r)
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

		inline bool operator!=(vDouble4 r)
		{
			IF__(x != r.x, true);
			IF__(y != r.y, true);
			IF__(z != r.z, true);
			IF__(w != r.w, true);

			return false;
		}

		inline bool operator==(vDouble4 r)
		{
			IF_F(x != r.x);
			IF_F(y != r.y);
			IF_F(z != r.z);
			IF_F(w != r.w);

			return true;
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

		double len(void)
		{
			return sqrt(x * x + y * y + z * z + w * w);
		}

		double sum(void)
		{
			return x + y + z + w;
		}

		void constrain(double a, double b)
		{
			x = (x < a) ? a : x;
			x = (x > b) ? b : x;
			y = (y < a) ? a : y;
			y = (y > b) ? b : y;
			z = (z < a) ? a : z;
			z = (z > b) ? b : z;
			w = (w < a) ? a : w;
			w = (w > b) ? b : w;
		}

	};

}
#endif
