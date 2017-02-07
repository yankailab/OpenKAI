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

bool FilterBase::init(void* pKiss)
{
	CHECK_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	F_INFO(pK->v("nTraj", (int*)&m_nTraj));
	if(m_nTraj<2)
		m_nTraj=2;

	reset();

	return true;
}

void FilterBase::input(double v)
{
	while(m_traj.size() >= m_nTraj)
	{
		m_traj.pop_front();
	}
	m_traj.push_back(m_v);
	CHECK_(m_traj.size() < m_nTraj);

	m_diff = abs(m_traj.at(m_nTraj-1) - m_traj.at(0));

	double nT = (double)m_nTraj;
	double baseT = 1.0/nT;
	double dV = abs(v - m_v);
	m_variance = m_variance*(nT-1)*baseT + dV*baseT;
}

double FilterBase::v(void)
{
	return m_v;
}

void FilterBase::reset(void)
{
	m_v = 0.0;
	m_variance = 0.0;
	m_diff = 0.0;
	m_data.clear();
	m_traj.clear();
}

double FilterBase::variance(void)
{
	return m_variance;
}

double FilterBase::diff(void)
{
	return m_diff;
}

}
