#ifndef OpenKAI_src_Primitive_vInt4_H_
#define OpenKAI_src_Primitive_vInt4_H_

#include "../Base/platform.h"
#include "vDouble4.h"

namespace kai
{

struct vInt4
{
	int x;
	int y;
	int z;
	int w;

	inline void operator=(vDouble4& r)
    {
		x = r.x;
		y = r.y;
		z = r.z;
		w = r.w;
	}

	void init(void)
	{
		x = 0;
		y = 0;
		z = 0;
		w = 0;
	}

	void init(int v)
	{
		x = v;
		y = v;
		z = v;
		w = v;
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
