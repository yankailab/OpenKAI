#ifndef OpenKAI_src_Primitive_vFloat4_H_
#define OpenKAI_src_Primitive_vFloat4_H_

#include "vFloat2.h"
#include "vFloat3.h"

namespace kai
{

	struct vFloat4
	{
		float x;
		float y;
		float z;
		float w;

		vFloat4(void)
		{
			x = 0.0;
			y = 0.0;
			z = 0.0;
			w = 0.0;
		}

		vFloat4(float v)
		{
			x = v;
			y = v;
			z = v;
			w = v;
		}

		vFloat4(float a, float b, float c, float d)
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

		void set(float v)
		{
			x = v;
			y = v;
			z = v;
			w = v;
		}

		void set(float a, float b, float c, float d)
		{
			x = a;
			y = b;
			z = c;
			w = d;
		}

		inline float &operator[](int i)
		{
			return (&x)[i];
		}

		inline vFloat4 operator+(vFloat4 &r)
		{
			vFloat4 v;
			v.x = r.x + x;
			v.y = r.y + y;
			v.z = r.z + z;
			v.w = r.w + w;
			return v;
		}

		inline vFloat4 operator-(vFloat4 &r)
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

		inline void operator=(float *pR)
		{
			x = pR[0];
			y = pR[1];
			z = pR[2];
			w = pR[3];
		}

		inline void operator=(Eigen::Vector4f r)
		{
			x = r[0];
			y = r[1];
			z = r[2];
			w = r[3];
		}

		inline void operator+=(vFloat4 &r)
		{
			x += r.x;
			y += r.y;
			z += r.z;
			w += r.w;
		}

		inline void operator-=(vFloat4 &r)
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

		inline bool operator!=(vFloat4 r)
		{
			IF__(x != r.x, true);
			IF__(y != r.y, true);
			IF__(z != r.z, true);
			IF__(w != r.w, true);

			return false;
		}

		inline bool operator==(vFloat4 r)
		{
			IF_F(x != r.x);
			IF_F(y != r.y);
			IF_F(z != r.z);
			IF_F(w != r.w);

			return true;
		}

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

		float len(void)
		{
			return sqrt(x * x + y * y + z * z + w * w);
		}

		float sum(void)
		{
			return x + y + z + w;
		}

		void constrain(float a, float b)
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
