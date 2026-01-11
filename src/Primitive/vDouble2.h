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

		inline double &operator[](int i)
		{
			return (&x)[i];
		}

		inline vDouble2 operator+(vDouble2 &r)
		{
			vDouble2 v;
			v.x = x + r.x;
			v.y = y + r.y;
			return v;
		}

		inline vDouble2 operator-(vDouble2 &r)
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

		inline void operator=(Eigen::Vector2d r)
		{
			x = r[0];
			y = r[1];
		}

		inline void operator=(const vector<double>& v)
		{
			int n = v.size();

			if (n >= 1)
				x = v[0];

			if (n >= 2)
				y = v[1];
		}

		inline void operator+=(vDouble2 &r)
		{
			x += r.x;
			y += r.y;
		}

		inline void operator-=(vDouble2 &r)
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

		inline bool operator!=(vDouble2 r)
		{
			IF__(x != r.x, true);
			IF__(y != r.y, true);

			return false;
		}

		inline bool operator==(vDouble2 r)
		{
			IF_F(x != r.x);
			IF_F(y != r.y);

			return true;
		}

		double mid(void)
		{
			return (x + y) * 0.5;
		}

		double length(void)
		{
			return sqrt(x * x + y * y);
		}

		double constrain(double v)
		{
			if (v > y)
				v = y;
			else if (v < x)
				v = x;

			return v;
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
