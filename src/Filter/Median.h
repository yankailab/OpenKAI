/*
 * Median.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Filter_Median_H_
#define OpenKAI_src_Filter_Median_H_

#include "FilterBase.h"

namespace kai
{

class Median: public FilterBase
{
public:
	Median();
	virtual ~Median();

	bool init(int wLen, int nTraj);
	void input(double v);
	void reset(void);

private:
	int m_windowLen;
	int m_iMid;
	std::deque<double> m_sort;

};

}
#endif
