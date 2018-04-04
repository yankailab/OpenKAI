/*
 * Average.cpp
 *
 *  Created on: Nov 18, 2015
 *      Author: yankai
 */

#include "Average.h"

namespace kai
{

Average::Average()
{
	m_windowLen = 1;
	reset();
}

Average::~Average()
{
}

bool Average::init(int wLen, int nTraj)
{
	IF_F(!FilterBase::init(nTraj));

	m_windowLen = wLen;
	if(m_windowLen<1)
		m_windowLen = 1;

	reset();

	return true;
}

void Average::input(double v)
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

	double tot = 0.0;
	for(int i=0; i<m_windowLen; i++)
		tot += m_data.at(i);

	m_v = tot / (double)m_windowLen;
	this->FilterBase::input(v);
}

void Average::reset(void)
{
	this->FilterBase::reset();
}

}
