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

bool Median::init(void* pKiss)
{
	CHECK_F(!FilterBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("windowLen", (int*)&m_windowLen));
	if(m_windowLen<3)
	{
		m_windowLen = 3;
	}

	m_iMid = m_windowLen/2;
	reset();

	return true;
}

void Median::input(double v)
{
	if(std::isnan(v))v = 0;
	m_data.push_back(v);
	CHECK_(m_data.size()<m_windowLen);

	while(m_data.size() > m_windowLen)
	{
		m_data.pop_front();
	}

	m_sort = m_data;
	std::sort(m_sort.begin(),m_sort.end());
	m_v = m_sort.at(m_iMid);
}

double Median::v(void)
{
	return m_v;
}

void Median::reset(void)
{
	this->FilterBase::reset();
	m_sort.clear();
}

double Median::variance(void)
{

}

double Median::diff(void)
{

}

}
