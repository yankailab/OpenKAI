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
	m_bRecord = false;
	m_showScale = 1.0;	//1m = 1pixel;
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
	F_INFO(pK->v("showScale", &m_showScale));

	reset();
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

bool _Path::start(void)
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

void _Path::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		trackZED();

		this->autoFPSto();
	}
}

void _Path::trackZED(void)
{
	NULL_(m_pZed);
	IF_(!m_bRecord);

	if(!m_pZed->isTracking())
	{
		m_pZed->startTracking();
		return;
	}

	vDouble4 dPh = m_pZed->getAccumulatedPos();
	vDouble3 dP;
	dP.m_x = dPh.m_x;
	dP.m_y = dPh.m_y;
	dP.m_z = dPh.m_z;

	IF_(dP.len() < m_dInterval);

	m_lastWP= m_vWP[m_vWP.size()-1];
	m_lastWP.m_p += dP;
	m_vWP.push_back(m_lastWP);
}

void _Path::startRecord(void)
{
	m_bRecord = true;
}

void _Path::stopRecord(void)
{
	m_bRecord = false;
}

void _Path::reset(void)
{
	m_lastWP.init();
	m_vWP.clear();
	m_vWP.push_back(m_lastWP);
}

WAY_POINT _Path::getLastWayPoint(void)
{
	return m_lastWP;
}

bool _Path::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
	string msg;

	pWin->tabNext();
	vDouble3 p = m_lastWP.m_p;
	msg = "Pos: X=" + f2str(p.m_x) + ", Y=" + f2str(p.m_y) + ", Z=" + f2str(p.m_z) + ", Hdg=" + f2str(m_lastWP.m_hdg);
	pWin->addMsg(&msg);
	pWin->tabPrev();

	//Plot center as vehicle position
	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	circle(*pMat, pCenter, 10, Scalar(0, 0, 255), 2);

	IF_T(m_vWP.size() < 2);

	Scalar col = Scalar(0, 255, 0);
	int bold = 2;

	WAY_POINT* pWP1 = &m_vWP[0];
	WAY_POINT* pWP2;

	//Plot trajectory
	for (int i = 1; i < m_vWP.size(); i++)
	{
		pWP2 = &m_vWP[i];
		vDouble3 p1 = pWP1->m_p - m_lastWP.m_p;
		vDouble3 p2 = pWP2->m_p - m_lastWP.m_p;
		pWP1 = pWP2;

		line(*pMat, pCenter + Point(p1.m_x, p1.m_y), pCenter + Point(p2.m_x, p2.m_y), col, bold);
	}

	return true;
}


}
