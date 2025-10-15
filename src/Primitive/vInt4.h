#ifndef OpenKAI_src_Primitive_vInt4_H_
#define OpenKAI_src_Primitive_vInt4_H_

#include "../Base/platform.h"

namespace kai
{

	struct vInt4
	{
		int x;
		int y;
		int z;
		int w;

		vInt4(void)
		{
			x = 0;
			y = 0;
			z = 0;
			w = 0;
		}

		vInt4(int v)
		{
			x = v;
			y = v;
			z = v;
			w = v;
		}

		vInt4(int a, int b, int c, int d)
		{
			x = a;
			y = b;
			z = c;
			w = d;
		}

		void clear(void)
		{
			x = 0;
			y = 0;
			z = 0;
			w = 0;
		}

		void set(int v)
		{
			x = v;
			y = v;
			z = v;
			w = v;
		}

		void set(int a, int b, int c, int d)
		{
			x = a;
			y = b;
			z = c;
			w = d;
		}

		inline int &operator[](int i)
		{
			return (&x)[i];
		}

		inline vInt4 operator+(vInt4 &r)
		{
			vInt4 v;
			v.x = x + r.x;
			v.y = y + r.y;
			v.z = z + r.z;
			v.w = w + r.w;
			return v;
		}

		inline vInt4 operator-(vInt4 &r)
		{
			vInt4 v;
			v.x = x - r.x;
			v.y = y - r.y;
			v.z = z - r.z;
			v.w = w - r.w;
			return v;
		}

		inline vInt4 operator*(float r)
		{
			vInt4 v;
			v.x = x * r;
			v.y = y * r;
			v.z = z * r;
			v.w = w * r;
			return v;
		}

		inline vInt4 operator/(float r)
		{
			vInt4 v;
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

		inline void operator=(Eigen::Vector4i r)
		{
			x = r[0];
			y = r[1];
			z = r[2];
			w = r[3];
		}

		inline void operator+=(vInt4 &r)
		{
			x += r.x;
			y += r.y;
			z += r.z;
			w += r.w;
		}

		inline void operator-=(vInt4 &r)
		{
			x -= r.x;
			y -= r.y;
			z -= r.z;
			w -= r.w;
		}

		inline void operator*=(int r)
		{
			x *= r;
			y *= r;
			z *= r;
			w *= r;
		}

		inline void operator/=(int r)
		{
			x /= r;
			y /= r;
			z /= r;
			w /= r;
		}

		inline bool operator!=(vInt4 r)
		{
			IF__(x != r.x, true);
			IF__(y != r.y, true);
			IF__(z != r.z, true);
			IF__(w != r.w, true);

			return false;
		}

		inline bool operator==(vInt4 r)
		{
			IF_F(x != r.x);
			IF_F(y != r.y);
			IF_F(z != r.z);
			IF_F(w != r.w);

			return true;
		}

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
	};

}
#endif
