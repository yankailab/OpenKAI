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

		inline bool operator==(vInt3 r)
		{
			IF_F(x != r.x);
			IF_F(y != r.y);
			IF_F(z != r.z);

			return true;
		}

		void clear(void)
		{
			x = 0;
			y = 0;
			z = 0;
		}

		void set(int a, int b, int c)
		{
			x = a;
			y = b;
			z = c;
		}
	};

}
#endif
