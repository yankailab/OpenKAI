/*
 * ControlBase.cpp
 *
 *  Created on: May 18, 2015
 *      Author: yankai
 */

#include "ControlBase.h"

namespace kai
{

ControlBase::ControlBase()
{
}

ControlBase::~ControlBase()
{
}

bool ControlBase::init(void* pKiss)
{
	IF_F(!this->BASE::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

//	KISSm(pK, );

	return true;
}

bool ControlBase::link(void)
{
	IF_F(!this->BASE::link());
	Kiss* pK = (Kiss*)m_pKiss;

	return true;
}

void ControlBase::update(void)
{

}

bool ControlBase::draw(void)
{
	return this->BASE::draw();
}

bool ControlBase::cli(int& iY)
{

	return true;
}

void ControlBase::reset(void)
{

}

}
