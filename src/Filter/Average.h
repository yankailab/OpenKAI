/*
 * Average.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Filter_Average_H_
#define OpenKAI_src_Filter_Average_H_

#include "../Base/common.h"
#include "FilterBase.h"

namespace kai
{

class Average: public FilterBase
{
public:
	Average();
	virtual ~Average();

	bool init(int wLen, int nTraj);
	void input(double v);

private:
	int m_windowLen;

};

}
#endif
