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

	int len(void)
	{
		return y-x;
	}

	int area(void)
	{
		return abs(x * y);
	}

	int constrain(int v)
	{
		if(v < x)v=x;
		else if(v > y)v=y;

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
