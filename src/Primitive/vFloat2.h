#ifndef OpenKAI_src_Primitive_vFloat2_H_
#define OpenKAI_src_Primitive_vFloat2_H_

namespace kai
{

	struct vFloat2
	{
		float x;
		float y;

		vFloat2(void)
		{
			x = 0.0;
			y = 0.0;
		}

		vFloat2(float v)
		{
			x = v;
			y = v;
		}

		vFloat2(float a, float b)
		{
			x = a;
			y = b;
		}

		void clear(void)
		{
			x = 0.0;
			y = 0.0;
		}

		void set(float v)
		{
			x = v;
			y = v;
		}

		void set(float a, float b)
		{
			x = a;
			y = b;
		}

		inline float &operator[](int i)
		{
			return (&x)[i];
		}

		inline vFloat2 operator+(vFloat2 &r)
		{
			vFloat2 v;
			v.x = x + r.x;
			v.y = y + r.y;
			return v;
		}

		inline vFloat2 operator-(vFloat2 &r)
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

		inline void operator=(Eigen::Vector2f r)
		{
			x = r[0];
			y = r[1];
		}

		inline void operator+=(vFloat2 &r)
		{
			x += r.x;
			y += r.y;
		}

		inline void operator-=(vFloat2 &r)
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

		inline bool operator!=(vFloat2 r)
		{
			IF__(x != r.x, true);
			IF__(y != r.y, true);

			return false;
		}

		inline bool operator==(vFloat2 r)
		{
			IF_F(x != r.x);
			IF_F(y != r.y);

			return true;
		}

		float mid(void)
		{
			return (x + y) * 0.5;
		}

		float len(void)
		{
			return sqrt((x * x) + (y * y));
		}

		float constrain(float v)
		{
			if (v > y)
				v = y;
			else if (v < x)
				v = x;

			return v;
		}

		bool bInside(float v)
		{
			IF_F(v < x);
			IF_F(v >= y);
			return true;
		}
	};

}
#endif
