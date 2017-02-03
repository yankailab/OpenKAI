/*
 * Filter.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#include "Filter.h"

namespace kai
{

Filter::Filter()
{
	m_windowLength = 3;
	m_iMid = 1;
	m_v = 0.0;
}

Filter::~Filter()
{
}

bool Filter::startMedian(int windowLength)
{
	m_windowLength = windowLength;
	if(m_windowLength<3)
	{
		m_windowLength = 3;
	}
	m_iMid = windowLength/2;
	m_v = 0.0;
	m_data.clear();
	m_sort.clear();

	return true;
}

void Filter::input(double v)
{
	if(std::isnan(v))v = 0;
	m_data.push_back(v);
	CHECK_(m_data.size()<m_windowLength);

	while(m_data.size() > m_windowLength)
	{
		m_data.pop_front();
	}

	m_sort = m_data;
	std::sort(m_sort.begin(),m_sort.end());
	m_v = m_sort.at(m_iMid);
}

double Filter::v(void)
{
	return m_v;
}

void Filter::reset(void)
{
	m_v = 0.0;
	m_data.clear();
	m_sort.clear();
}


}
