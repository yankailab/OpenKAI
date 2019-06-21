/*
 * _SortingDetector.cpp
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#include "_ConveyerDetector.h"

namespace kai
{

_ConveyerDetector::_ConveyerDetector()
{
	m_cSpeed = 0.0;
	m_cLen = 2.0;
	m_minOverlap = 0.8;
}

_ConveyerDetector::~_ConveyerDetector()
{
}

bool _ConveyerDetector::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, cSpeed);
	KISSm(pK, cLen);
	KISSm(pK, minOverlap);

	return true;
}

bool _ConveyerDetector::start(void)
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

void _ConveyerDetector::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateTarget();

		updateObj();
		if (m_bGoSleep)
		{
			m_pPrev->reset();
		}

		this->autoFPSto();
	}
}

int _ConveyerDetector::check(void)
{
	NULL__(m_pDB, -1);

	return 0;
}

void _ConveyerDetector::updateTarget(void)
{
	IF_(check() < 0);

	//update existing target positions
	float spd = m_cSpeed * ((float) m_dTime) * 1e-6;
	int i=0;
	OBJECT* pO;
	while ((pO = at(i++)))
	{
		pO->m_bb.y += spd;
		pO->m_bb.w += spd;
		IF_CONT(pO->m_bb.midY() > m_cLen);

		add(pO);
	}

	//get new targets and compare to existing ones
	i = 0;
	while ((pO = m_pDB->at(i++)))
	{
		IF_CONT(pO->m_topClass < 0);

		int j=0;
		OBJECT* pT;
		while ((pT = m_pNext->at(j++)))
		{
			IF_CONT(nIoU(pT->m_bb, pO->m_bb) < m_minOverlap);

			if (pT->m_topProb < pO->m_topProb)
			{
				pT->m_topClass = pO->m_topClass;
				pT->m_topProb = pO->m_topProb;
			}
			break;
		}
		IF_CONT(pT);

		add(pO);
	}
}

bool _ConveyerDetector::draw(void)
{
	IF_F(!this->_DetectorBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _ConveyerDetector::console(int& iY)
{
	IF_F(!this->_DetectorBase::console(iY));

	string msg;

	return true;
}

}
