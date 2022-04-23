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

	void init(void)
	{
		x = 0;
		y = 0;
	}

	void set(int a, int b)
	{
		x = a;
		y = b;
	}

	int len(void)
	{
		return y-x;
	}

	int area(void)
	{
		return abs(x * y);
	}
};

}
#endif
