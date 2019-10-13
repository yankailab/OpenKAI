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

void ControlBase::draw(void)
{
	this->BASE::draw();
}

void ControlBase::reset(void)
{

}

}
