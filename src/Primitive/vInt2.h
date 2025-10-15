#ifndef OpenKAI_src_Primitive_vInt2_H_
#define OpenKAI_src_Primitive_vInt2_H_

#include "../Base/platform.h"

namespace kai
{

	struct vInt2
	{
		int x;
		int y;

		vInt2(void)
		{
			x = 0;
			y = 0;
		}

		vInt2(int v)
		{
			x = v;
			y = v;
		}

		vInt2(int a, int b)
		{
			x = a;
			y = b;
		}

		void clear(void)
		{
			x = 0;
			y = 0;
		}

		void set(int a, int b)
		{
			x = a;
			y = b;
		}

		inline int &operator[](int i)
		{
			return (&x)[i];
		}

		inline vInt2 operator+(vInt2 &r)
		{
			vInt2 v;
			v.x = x + r.x;
			v.y = y + r.y;
			return v;
		}

		inline vInt2 operator-(vInt2 &r)
		{
			vInt2 v;
			v.x = x - r.x;
			v.y = y - r.y;
			return v;
		}

		inline vInt2 operator*(float r)
		{
			vInt2 v;
			v.x = x * r;
			v.y = y * r;
			return v;
		}

		inline vInt2 operator/(float r)
		{
			vInt2 v;
			v.x = x / r;
			v.y = y / r;
			return v;
		}

		inline void operator=(float r)
		{
			x = r;
			y = r;
		}

		inline void operator=(Eigen::Vector2i r)
		{
			x = r[0];
			y = r[1];
		}

		inline void operator+=(vInt2 &r)
		{
			x += r.x;
			y += r.y;
		}

		inline void operator-=(vInt2 &r)
		{
			x -= r.x;
			y -= r.y;
		}

		inline void operator*=(int r)
		{
			x *= r;
			y *= r;
		}

		inline void operator/=(int r)
		{
			x /= r;
			y /= r;
		}

		inline bool operator!=(vInt2 r)
		{
			IF__(x != r.x, true);
			IF__(y != r.y, true);

			return false;
		}

		inline bool operator==(vInt2 r)
		{
			IF_F(x != r.x);
			IF_F(y != r.y);

			return true;
		}

		int len(void)
		{
			return y - x;
		}

		int area(void)
		{
			return abs(x * y);
		}

		int constrain(int v)
		{
			if (v < x)
				v = x;
			else if (v > y)
				v = y;

			return v;
		}

		bool bInside(int v)
		{
			IF_F(v < x);
			IF_F(v >= y);
			return true;
		}

		bool bContain(int v)
		{
			IF_F(v < x);
			IF_F(v > y);
			return true;
		}
	};

}
#endif
