/*
 * _DetReceiver.cpp
 *
 *  Created on: Sept 3, 2019
 *      Author: yankai
 */

#include "_DetReceiver.h"

namespace kai
{

_DetReceiver::_DetReceiver()
{
	m_pOK = NULL;
	m_timeOut = USEC_1SEC;
}

_DetReceiver::~_DetReceiver()
{
}

bool _DetReceiver::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	pK->v("timeOut",&m_timeOut);

	m_nClass = 1;

	string iName;
	iName = "";
	F_ERROR_F(pK->v("_OKlinkAPcopter", &iName));
	m_pOK = (_OKlinkAPcopter*) (pK->root()->getChildInst(iName));
	NULL_Fl(m_pOK, iName+": not found");

	return true;
}

bool _DetReceiver::start(void)
{
	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _DetReceiver::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		receive();
		updateObj();

		if(m_bGoSleep)
		{
			m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _DetReceiver::check(void)
{
	IF__(!m_pOK,-1);

	return 0;
}

void _DetReceiver::receive(void)
{
	IF_(check()<0);

	IF_(m_tStamp - m_pOK->m_tPos > m_timeOut);
	IF_(m_pOK->m_tPos < 0.0);

	float d = 0.05;
	OBJECT o;
	o.init();
	o.m_tStamp = m_tStamp;
	o.m_bb.x = m_pOK->m_vPos.x - d;
	o.m_bb.y = m_pOK->m_vPos.y - d;
	o.m_bb.z = m_pOK->m_vPos.x + d;
	o.m_bb.w = m_pOK->m_vPos.y + d;
	o.m_dist = m_pOK->m_vPos.z; //dAlt
	o.setTopClass(0, 1.0);

	add(&o);
}

bool _DetReceiver::draw(void)
{
	IF_F(!this->_DetectorBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _DetReceiver::console(int& iY)
{
	IF_F(!this->_DetectorBase::console(iY));

	string msg;
	OBJECT* pO;
	int i=0;
	while((pO = at(i++)) != NULL)
	{
		C_MSG("vPos=(" + f2str(pO->m_bb.midX()) + ", " + f2str(pO->m_bb.midY()) + ")");
	}

	return true;
}


}
