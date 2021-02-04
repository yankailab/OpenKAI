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
	m_dInterval = 0;
	m_bRecord = false;
	m_showScale = 1.0;	//1m = 1pixel;
}

_Path::~_Path()
{
}

bool _Path::init(void *pKiss)
{
	IF_F(!this->_ModuleBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	pK->v("dInterval", &m_dInterval);
	pK->v("showScale", &m_showScale);

	return true;
}

bool _Path::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _Path::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		updateGPS();

		m_pT->autoFPSto();
	}
}

void _Path::updateGPS(void)
{
	IF_(!m_bRecord);

	UTM_POS utm = m_coord.LL2UTM(m_pos);

	if(m_vWP.size()>0)
	{
		UTM_POS lastUTM = m_coord.LL2UTM(m_lastPos);
		IF_(utm.dist(&lastUTM) < m_dInterval);
	}

	m_vWP.push_back(m_pos);
	m_lastPos = m_pos;
}

void _Path::startRec(void)
{
	m_vWP.clear();
	m_bRecord = true;
}

void _Path::pauseRec(void)
{
	m_bRecord = false;
}

void _Path::resumeRec(void)
{
	m_bRecord = true;
}

void _Path::stopRec(void)
{
	m_bRecord = false;
}

void _Path::draw(void)
{
	this->_ModuleBase::draw();

	string msg;

#ifdef USE_OPENCV
	IF_(!checkWindow());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->m();

	//Plot center as vehicle position
	Point pCenter(pMat->cols / 2, pMat->rows / 2);
	circle(*pMat, pCenter, 10, Scalar(0, 0, 255), 2);

	IF_(m_vWP.size() < 3);

	//Plot trajectory
	Scalar col = Scalar(0, 255, 0);
	int bold = 2;

//	UTM_POS *pWP1 = &m_vWP[1];
//	UTM_POS *pWP2;
//	UTM_POS initWP;	// = *m_pGPS->getInitUTM();
//	vDouble2 pI;
//	pI.x = initWP.m_easting;
//	pI.y = initWP.m_northing;
//
//	for (unsigned int i = 2; i < m_vWP.size(); i++)
//	{
//		pWP2 = &m_vWP[i];
//		vDouble2 p1, p2;
//		p1.x = pWP1->m_easting;
//		p1.y = pWP1->m_northing;
//		p2.x = pWP2->m_easting;
//		p2.y = pWP2->m_northing;
//		p1 -= pI;
//		p2 -= pI;
//		pWP1 = pWP2;
//
//		line(*pMat, pCenter + Point(p1.x, p1.y), pCenter + Point(p2.x, p2.y),
//				col, bold);
//	}
#endif
}

}
