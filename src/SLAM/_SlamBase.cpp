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

void _SlamBase::reset(void)
{
	m_vPos.init();
	m_vAtti.init();
	m_confidence = 0.0;
}

void _SlamBase::draw(void)
{
	this->_ThreadBase::draw();

	string msg;
	msg = "pos=(" + f2str(m_vPos.x,3) + ", "
				  + f2str(m_vPos.y,3) + ", "
				  + f2str(m_vPos.z,3) + ")";
	addMsg(msg,1);

	msg = "attitude=(" + f2str(m_vAtti.x,3) + ", "
				  + f2str(m_vAtti.y,3) + ", "
				  + f2str(m_vAtti.z,3) + ")";
	addMsg(msg,1);

	msg = "confidence=" + f2str(m_confidence);
	addMsg(msg,1);
}

}
