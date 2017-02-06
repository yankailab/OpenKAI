/*
 * Median.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef SRC_ALGORITHM_FILTE_MEDIAN_H_
#define SRC_ALGORITHM_FILTE_MEDIAN_H_

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
	double v(void);
	double variance(void);
	double diff(void);
	void reset(void);

private:
	unsigned int m_windowLen;
	unsigned int m_diffInt;
	unsigned int m_iMid;
	std::deque<double> m_sort;


};

}

#endif
