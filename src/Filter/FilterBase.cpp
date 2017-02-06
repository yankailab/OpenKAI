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
	reset();
}

FilterBase::~FilterBase()
{
}

bool FilterBase::init(void* pKiss)
{
	CHECK_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

void FilterBase::input(double v)
{
}

double FilterBase::v(void)
{
	return m_v;
}

void FilterBase::reset(void)
{
	m_v = 0.0;
	m_data.clear();
}


}
