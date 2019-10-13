/*
 * _SlamBase.cpp
 *
 *  Created on: May 29, 2018
 *      Author: yankai
 */

#include "_SlamBase.h"

namespace kai
{

_SlamBase::_SlamBase()
{
	m_bReady = false;
	reset();
}

_SlamBase::~_SlamBase()
{
}

bool _SlamBase::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

void _SlamBase::detect(void)
{
}

void _SlamBase::draw(void)
{
	this->_ThreadBase::draw();

	string msg;
	msg = "pos=(" + f2str(m_pos.x) + ", "
				  + f2str(m_pos.y) + ", "
				  + f2str(m_pos.z) + ")";

	addMsg(msg,1);
}

void _SlamBase::reset(void)
{
	m_pos.init();
}

}
