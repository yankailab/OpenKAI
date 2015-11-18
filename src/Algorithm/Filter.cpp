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

double Filter::input(double v)
{
	long data[FILTER_BUF];
	int i, j;

	m_trajectory[m_iTraj] = v;
	if(++m_iTraj >= m_windowLength)
	{
		m_iTraj = 0;
	}

	long tmp;

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

	return data[m_iMedian];
}





} /* namespace kai */
