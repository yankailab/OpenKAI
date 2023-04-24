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
