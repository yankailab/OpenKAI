/*
 * FilterBase.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#include "FilterBase.h"

namespace kai
{

FilterBase::FilterBase()
{
	m_nTraj = 2;
	reset();
}

FilterBase::~FilterBase()
{
}

bool FilterBase::init(int nTraj)
{
	m_nTraj = (nTraj < 2) ? 2 : nTraj;

	reset();
	return true;
}

void FilterBase::input(double v)
{
	while (m_traj.size() >= m_nTraj)
	{
		m_traj.pop_front();
	}
	m_traj.push_back(m_v);
}

double FilterBase::v(void)
{
	return m_v;
}

void FilterBase::reset(void)
{
	m_v = 0.0;
	m_variance = 0.0;
	m_data.clear();
	m_traj.clear();
}

}
