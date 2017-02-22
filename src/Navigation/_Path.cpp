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

	pthread_mutex_init(&m_mutexWP, NULL);

}

_Path::~_Path()
{
	pthread_mutex_destroy(&m_mutexWP);

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
	F_INFO(pK->v("_GPS", &iName));
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

	pthread_mutex_lock(&m_mutexWP);
	m_vWP.push_back(newP);
	m_lastWP = newP;
	pthread_mutex_unlock(&m_mutexWP);
}

int _Path::markCurrentWayPoint(void)
{
	if(!m_pGPS)return -1;

	UTM_POS newP = *m_pGPS->getUTM();
	pthread_mutex_lock(&m_mutexWP);
	m_vWP.push_back(newP);
	m_lastWP = newP;
	int iLast = m_vWP.size()-1;
	pthread_mutex_unlock(&m_mutexWP);

	return iLast;
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

UTM_POS _Path::getLastWayPoint(void)
{
	return m_lastWP;
}

bool _Path::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->getCMat();
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
	pI.m_x = initWP.m_easting;
	pI.m_y = initWP.m_northing;

	for (unsigned int i = 2; i < m_vWP.size(); i++)
	{
		pWP2 = &m_vWP[i];
		vDouble2 p1,p2;
		p1.m_x = pWP1->m_easting;
		p1.m_y = pWP1->m_northing;
		p2.m_x = pWP2->m_easting;
		p2.m_y = pWP2->m_northing;
		p1 -= pI;
		p2 -= pI;
		pWP1 = pWP2;

		line(*pMat, pCenter + Point(p1.m_x, p1.m_y), pCenter + Point(p2.m_x, p2.m_y), col, bold);
	}

	return true;
}


}
