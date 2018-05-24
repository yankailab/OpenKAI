/*
 * FilterBase.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef OpenKAI_src_Filter_FilterBase_H_
#define OpenKAI_src_Filter_FilterBase_H_

#include "../Base/common.h"

namespace kai
{

class FilterBase
{
public:
	FilterBase();
	virtual ~FilterBase();

	virtual bool init(int nTraj);
	virtual void input(double v);
	virtual double v(void);
	virtual void reset(void);

public:
	double	m_v;
	std::deque<double> m_data;

	unsigned int m_nTraj;
	std::deque<double> m_traj;

	double	m_variance;
};

}
#endif
