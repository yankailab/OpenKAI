#ifndef OpenKAI_src_Primitive_vInt3_H_
#define OpenKAI_src_Primitive_vInt3_H_

#include "../Base/platform.h"

namespace kai
{

	struct vInt3
	{
		int x;
		int y;
		int z;

		vInt3(void)
		{
			x = 0;
			y = 0;
			z = 0;
		}

		vInt3(int v)
		{
			x = v;
			y = v;
			z = v;
		}

		vInt3(int a, int b, int c)
		{
			x = a;
			y = b;
			z = c;
		}

		void clear(void)
		{
			x = 0;
			y = 0;
			z = 0;
		}

		void set(int v)
		{
			x = v;
			y = v;
			z = v;
		}

		void set(int a, int b, int c)
		{
			x = a;
			y = b;
			z = c;
		}

		inline int &operator[](int i)
		{
			return (&x)[i];
		}

		inline vInt3 operator+(vInt3 &r)
		{
			vInt3 v;
			v.x = x + r.x;
			v.y = y + r.y;
			v.z = z + r.z;
			return v;
		}

		inline vInt3 operator-(vInt3 &r)
		{
			vInt3 v;
			v.x = x - r.x;
			v.y = y - r.y;
			v.z = z - r.z;
			return v;
		}

		inline vInt3 operator*(float r)
		{
			vInt3 v;
			v.x = x * r;
			v.y = y * r;
			v.z = z * r;
			return v;
		}

		inline vInt3 operator/(float r)
		{
			vInt3 v;
			v.x = x / r;
			v.y = y / r;
			v.z = z / r;
			return v;
		}

		inline void operator=(float r)
		{
			x = r;
			y = r;
			z = r;
		}

		inline void operator=(Eigen::Vector3i r)
		{
			x = r[0];
			y = r[1];
			z = r[2];
		}

		inline void operator+=(vInt3 &r)
		{
			x += r.x;
			y += r.y;
			z += r.z;
		}

		inline void operator-=(vInt3 &r)
		{
			x -= r.x;
			y -= r.y;
			z -= r.z;
		}

		inline void operator*=(int r)
		{
			x *= r;
			y *= r;
			z *= r;
		}

		inline void operator/=(int r)
		{
			x /= r;
			y /= r;
			z /= r;
		}

		inline bool operator!=(vInt3 r)
		{
			IF__(x != r.x, true);
			IF__(y != r.y, true);
			IF__(z != r.z, true);

			return false;
		}

		inline bool operator==(vInt3 r)
		{
			IF_F(x != r.x);
			IF_F(y != r.y);
			IF_F(z != r.z);

			return true;
		}
	};

}
#endif
