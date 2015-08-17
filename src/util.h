#pragma once
#include "stdio.h"

inline int constrain(int x, int l, int h)
{
	int y;

	if (x < l)y = l;
	else y = x;

	if (y > h)return h;
	else return y;
}

inline double confineVal(double val, double max, double min)
{
	if (val>max)val = max;
	else if (val<min)val = min;

	return val;
}

