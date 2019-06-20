/*
 * _SortingDetector.cpp
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#include "_SortingDetector.h"

namespace kai
{

_SortingDetector::_SortingDetector()
{
	m_cSpeed = 0.0;
	m_cLen = 2.0;
	m_minOverlap = 0.8;
}

_SortingDetector::~_SortingDetector()
{
}

bool _SortingDetector::init(void* pKiss)
{
	IF_F(!this->_DetectorBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, cSpeed);
	KISSm(pK, cLen);
	KISSm(pK, minOverlap);

	return true;
}

bool _SortingDetector::start(void)
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

void _SortingDetector::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateTarget();

		this->autoFPSto();
	}
}

int _SortingDetector::check(void)
{
	NULL__(m_pDB, -1);

	return 0;
}

void _SortingDetector::updateTarget(void)
{
	IF_(check() < 0);

	unsigned int i,j;

	//update existing target positions
	float spd = m_cSpeed * ((float) m_dTime) * 1e-6;
	for (i = 0; i < m_vTarget.size(); i++)
	{
		OBJECT* pO = &m_vTarget[i];
		pO->m_bb.y += spd;
		pO->m_bb.w += spd;
	}

	//delete targets out of range
	if (!m_vTarget.empty())
	{
		while (m_vTarget.front().m_bb.midY() > m_cLen)
		{
			m_vTarget.pop_front();
			if (m_vTarget.empty())
				return;
		}
	}

	//get new targets and compare to existing ones
	OBJECT* pO;
	i = 0;
	while ((pO = m_pDB->at(i++)))
	{
		IF_CONT(pO->m_topClass < 0);

		for (j = 0; j < m_vTarget.size(); j++)
		{
			OBJECT* pT = &m_vTarget[j];
			IF_CONT(nIoU(pT->m_bb, pO->m_bb) < m_minOverlap);

			if (pT->m_topProb < pO->m_topProb)
			{
				pT->m_topClass = pO->m_topClass;
				pT->m_topProb = pO->m_topProb;
			}
			break;
		}
		IF_CONT(j < m_vTarget.size());

		m_vTarget.push_back(*pO);
	}
}

bool _SortingDetector::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _SortingDetector::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
