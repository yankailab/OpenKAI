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
	m_vT.init();
	m_vV.init();
	m_vQ.init();
	m_confidence = 0.0;
}

vFloat3 _SlamBase::t(void)
{
	return m_vT;
}

vFloat3 _SlamBase::v(void)
{
	return m_vV;
}

vFloat4 _SlamBase::q(void)
{
	return m_vQ;
}

void _SlamBase::draw(void)
{
	this->_ThreadBase::draw();

	string msg;
	msg = "vT = (" + f2str(m_vT.x,3) + ", "
				  + f2str(m_vT.y,3) + ", "
				  + f2str(m_vT.z,3) + ")";
	addMsg(msg,1);

	msg = "vV = (" + f2str(m_vV.x,3) + ", "
				  + f2str(m_vV.y,3) + ", "
				  + f2str(m_vV.z,3) + ")";
	addMsg(msg,1);

	msg = "vQ = (" + f2str(m_vQ.x,3) + ", "
				  + f2str(m_vQ.y,3) + ", "
				  + f2str(m_vQ.z,3) + ", "
				  + f2str(m_vQ.w,3) + ")";
	addMsg(msg,1);

	msg = "confidence=" + f2str(m_confidence);
	addMsg(msg,1);
}

}
