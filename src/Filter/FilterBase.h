/*
 * FilterBase.h
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#ifndef SRC_ALGORITHM_FILTERBASE_H_
#define SRC_ALGORITHM_FILTERBASE_H_

#include "../Base/common.h"
#include "../Base/BASE.h"
#include "../Script/Kiss.h"

namespace kai
{

class FilterBase: public BASE
{
public:
	FilterBase();
	virtual ~FilterBase();

	virtual bool init(void* pKiss);
	virtual void input(double v);
	virtual double v(void);
	virtual void reset(void);
	virtual double diff(void);

public:
	double	m_v;
	std::deque<double> m_data;

	unsigned int m_nTraj;
	std::deque<double> m_traj;

	double	m_diff;
	double	m_variance;
};

}

#endif
