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

	void init(void)
	{
		x = 0;
		y = 0;
		z = 0;
	}
};

}
#endif
