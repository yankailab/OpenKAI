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
	m_bReset = false;
	m_vAxisIdx.init(0,1,2);
	m_vRoffset.init(0.0);
	resetAll();
}

_SlamBase::~_SlamBase()
{
}

bool _SlamBase::init(void* pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("vAxisIdx", &m_vAxisIdx);
	pK->v("vRoffset", &m_vRoffset);

	return true;
}

bool _SlamBase::bReady(void)
{
	return m_bReady && (!m_bReset);
}

void _SlamBase::reset(void)
{
	m_bReset = true;
}

void _SlamBase::resetAll(void)
{
	m_vT.init();
	m_vV.init();
	m_vR.init();
	m_vQ.init();
	m_mT = Matrix4d::Identity();
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

vFloat3 _SlamBase::r(void)
{
	return m_vR;
}

vFloat4 _SlamBase::q(void)
{
	return m_vQ;
}

const Matrix4d& _SlamBase::mT(void)
{
	return m_mT;
}

void _SlamBase::draw(void)
{
	this->_ModuleBase::draw();

	string msg;
	msg = "vT = (" + f2str(m_vT.x,3) + ", "
				  + f2str(m_vT.y,3) + ", "
				  + f2str(m_vT.z,3) + ")";
	addMsg(msg,1);

	msg = "vV = (" + f2str(m_vV.x,3) + ", "
				  + f2str(m_vV.y,3) + ", "
				  + f2str(m_vV.z,3) + ")";
	addMsg(msg,1);

	msg = "vR = (" + f2str(m_vR.x,3) + ", "
				  + f2str(m_vR.y,3) + ", "
				  + f2str(m_vR.z,3) + ")";
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
