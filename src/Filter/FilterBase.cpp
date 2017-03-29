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
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	F_INFO(pK->v("nTraj", (int*)&m_nTraj));
	m_nTraj = (m_nTraj < 2) ? 2 : m_nTraj;

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
	IF_(m_traj.size() < m_nTraj);

	m_accumulatedDiff += m_traj.at(m_nTraj - 1) - m_traj.at(m_nTraj - 2);
}

double FilterBase::v(void)
{
	return m_v;
}

void FilterBase::reset(void)
{
	m_v = 0.0;
	m_variance = 0.0;
	m_accumulatedDiff = 0.0;
	m_data.clear();
	m_traj.clear();
}

double FilterBase::accumlatedDiff(void)
{
	double d = m_accumulatedDiff;
	m_accumulatedDiff = 0.0;
	return d;
}

}
