/*
 * Filter.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#include "../Base/common.h"

#ifndef SRC_ALGORITHM_FILTER_H_
#define SRC_ALGORITHM_FILTER_H_

namespace kai
{

class Filter
{
public:
	Filter();
	virtual ~Filter();

	bool startMedian(int windowLength);
	void input(double v);
	double v(void);

private:
	unsigned int m_windowLength;
	unsigned int m_iMid;
	double m_v;
	std::deque<double> m_data;
	std::deque<double> m_sort;
};

} /* namespace kai */

#endif /* SRC_ALGORITHM_FILTER_H_ */
