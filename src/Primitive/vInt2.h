#ifndef OpenKAI_src_Primitive_vInt2_H_
#define OpenKAI_src_Primitive_vInt2_H_

#include "../Base/platform.h"

namespace kai
{

struct vInt2
{
	int x;
	int y;

	int area(void)
	{
		return abs(x * y);
	}

	void init(void)
	{
		x = 0;
		y = 0;
	}

	int len(void)
	{
		return y-x;
	}
};

}
#endif
