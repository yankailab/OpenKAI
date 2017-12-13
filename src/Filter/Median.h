/*
 * Median.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef SRC_ALGORITHM_FILTER_MEDIAN_H_
#define SRC_ALGORITHM_FILTER_MEDIAN_H_

#include "../Base/common.h"
#include "FilterBase.h"

namespace kai
{

class Median: public FilterBase
{
public:
	Median();
	virtual ~Median();

	bool init(void* pKiss);
	void input(double v);
	void reset(void);

private:
	int m_windowLen;
	int m_iMid;
	std::deque<double> m_sort;


};

}

#endif
