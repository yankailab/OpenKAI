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
	m_pGPS = NULL;
	m_vWP.clear();
	m_dInterval = 0;
	m_lastWP.init();
	m_bRecord = false;
	m_showScale = 1.0;	//1m = 1pixel;
	m_distCompress = 0.0;
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
	F_INFO(pK->v("distCompress", &m_distCompress));

	reset();

	//link
	string iName;

	iName = "";
	F_ERROR_F(pK->v("_GPS", &iName));
	m_pGPS = (_GPS*) (pK->root()->getChildInstByName(&iName));

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

		updateGPS();

		this->autoFPSto();
	}
}

void _Path::updateGPS(void)
{
	NULL_(m_pGPS);
	IF_(!m_bRecord);

	UTM_POS newP = *m_pGPS->getUTM();
	IF_(newP.dist(&m_lastWP) < m_dInterval);

	m_vWP.push_back(newP);
	m_lastWP = newP;
}

UTM_POS* _Path::getCurrentPos(void)
{
	NULL_N(m_pGPS);
	return m_pGPS->getUTM();
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

void _Path::compress(void)
{

}

UTM_POS* _Path::getLastWayPoint(void)
{
	return &m_lastWP;
}

UTM_POS* _Path::getWayPoint(int iWP)
{
	IF_N(iWP < 0);
	IF_N(iWP >= m_vWP.size());

	return &m_vWP[iWP];
}

bool _Path::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();
	string msg;

	//Plot center as vehicle position
	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	circle(*pMat, pCenter, 10, Scalar(0, 0, 255), 2);

	IF_T(m_vWP.size() < 3);

	//Plot trajectory
	Scalar col = Scalar(0, 255, 0);
	int bold = 2;

	UTM_POS* pWP1 = &m_vWP[1];
	UTM_POS* pWP2;
	UTM_POS initWP = *m_pGPS->getInitUTM();
	vDouble2 pI;
	pI.x = initWP.m_easting;
	pI.y = initWP.m_northing;

	for (unsigned int i = 2; i < m_vWP.size(); i++)
	{
		pWP2 = &m_vWP[i];
		vDouble2 p1,p2;
		p1.x = pWP1->m_easting;
		p1.y = pWP1->m_northing;
		p2.x = pWP2->m_easting;
		p2.y = pWP2->m_northing;
		p1 -= pI;
		p2 -= pI;
		pWP1 = pWP2;

		line(*pMat, pCenter + Point(p1.x, p1.y), pCenter + Point(p2.x, p2.y), col, bold);
	}

	return true;
}


}
