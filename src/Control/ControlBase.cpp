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

void ControlBase::update(void)
{

}

bool ControlBase::draw(void)
{
	return this->BASE::draw();
}

bool ControlBase::cli(int& iY)
{
	return this->BASE::cli(iY);
}

void ControlBase::reset(void)
{

}

}
