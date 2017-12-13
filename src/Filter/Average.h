/*
 * Average.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef SRC_ALGORITHM_FILTER_AVERAGE_H_
#define SRC_ALGORITHM_FILTER_AVERAGE_H_

#include "../Base/common.h"
#include "FilterBase.h"

namespace kai
{

class Average: public FilterBase
{
public:
	Average();
	virtual ~Average();

	bool init(void* pKiss);
	void input(double v);
	void reset(void);

private:
	int m_windowLen;

};

}

#endif
