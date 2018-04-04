/*
 * Median.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#include "Median.h"

namespace kai
{

Median::Median()
{
	m_windowLen = 3;
	m_iMid = 1;
	reset();
}

Median::~Median()
{
}

bool Median::init(int wLen, int nTraj)
{
	IF_F(!FilterBase::init(nTraj));

	m_windowLen = wLen;
	if(m_windowLen<3)
		m_windowLen = 3;

	m_iMid = m_windowLen/2;
	reset();

	return true;
}

void Median::input(double v)
{
	if(std::isnan(v))v = 0;
	m_data.push_back(v);
	if(m_data.size() < m_windowLen)
	{
		m_v = v;
		this->FilterBase::input(v);
		return;
	}

	while(m_data.size() > m_windowLen)
	{
		m_data.pop_front();
	}

	m_sort = m_data;
	std::sort(m_sort.begin(),m_sort.end());
	m_v = m_sort.at(m_iMid);

	this->FilterBase::input(v);
}

void Median::reset(void)
{
	this->FilterBase::reset();
	m_sort.clear();
}

}
