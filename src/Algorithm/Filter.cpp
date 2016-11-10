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
	// TODO Auto-generated constructor stub
	m_iTraj = 0;
	m_windowLength = 0;
	m_iMedian = 0;
	m_vMid = 0;
}

Filter::~Filter()
{
	// TODO Auto-generated destructor stub
}


bool Filter::startMedian(int windowLength)
{
	if(windowLength >= FILTER_BUF)
	{
		return false;
	}

	m_iTraj = 0;
	m_windowLength = windowLength;
	m_iMedian = m_windowLength * 0.5;
	for(int i=0; i<m_windowLength;i++)
	{
		m_trajectory[i] = 0;
	}

	return true;
}

void Filter::input(double v)
{
	double data[FILTER_BUF];
	double tmp;
	int i, j;

	m_trajectory[m_iTraj] = v;
	if(++m_iTraj >= m_windowLength)
	{
		m_iTraj = 0;
	}


	for (i = 0; i<m_windowLength; i++)
	{
		data[i] = m_trajectory[i];

		for (j = i; j>0; j--)
		{
			if (data[j] < data[j - 1])
			{
				SWITCH(data[j], data[j - 1], tmp);
			}
			else
			{
				break;
			}
		}
	}

	m_vMid = data[m_iMedian];
}

double Filter::v(void)
{
	return m_vMid;
}


} /* namespace kai */
