/*
 * Path.cpp
 *
 *  Created on: Jan 6, 2017
 *      Author: yankai
 */

#include "_Path.h"

namespace kai
{

_Path::_Path()
{
	m_pZed = NULL;
	m_vWP.clear();
	m_dInterval = 0;
	m_lastWP.init();
}

_Path::~_Path()
{
}

bool _Path::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	F_INFO(pK->v("dInterval", &m_dInterval));

	m_vWP.clear();
	m_vWP.push_back(m_lastWP);

	return true;
}

bool _Path::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("_ZED", &iName));
	m_pZed = (_ZED*) (pK->root()->getChildInstByName(&iName));

	return true;
}

void _Path::update(void)
{
	NULL_(m_pZed);

	if(!m_pZed->isTracking())
	{
		m_pZed->startTracking();
		return;
	}

	vDouble3 dP = m_pZed->getAccumulatedPos();
	IF_(dP.len() < m_dInterval);

	WAY_POINT wP = m_vWP[m_vWP.size()-1];
	wP.m_p += dP;
	m_vWP.push_back(wP);
}

void _Path::reset(void)
{
	m_vWP.clear();
}

bool _Path::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	string msg;

	msg = "";
	pWin->addMsg(&msg);

	return true;
}


}
