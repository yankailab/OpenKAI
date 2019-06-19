/*
 * _SortingBot.cpp
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#include "_SortingBot.h"

namespace kai
{

_SortingBot::_SortingBot()
{
	m_pSW = NULL;
	m_pC = NULL;
	m_cSpeed = 0.0;
	m_nClass = 0;
	m_cLen = 2.0;
	m_minOverlap = 0.8;
}

_SortingBot::~_SortingBot()
{
}

bool _SortingBot::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, nClass);
	KISSm(pK, cSpeed);
	KISSm(pK, cLen);
	KISSm(pK, minOverlap);
	pK->array("dropPos", m_pDropPos, SB_N_CLASS);

	Kiss* pA;
	Kiss** pItr;
	string iName;
	int i;
	int pClass[SB_N_CLASS];

	pA = pK->o("armSet");
	if (pA)
	{
		pItr = pA->getChildItr();
		i = 0;
		Kiss* pC;

		while ((pC = pItr[i++]))
		{
			SB_ARMSET c;
			c.init();

			pC->v("gripX1", &c.m_rGripX.x);
			pC->v("gripX2", &c.m_rGripX.y);
			pC->v("gripY1", &c.m_rGripY.x);
			pC->v("gripY2", &c.m_rGripY.y);
			pC->v("gripZ1", &c.m_rGripZ.x);
			pC->v("gripZ2", &c.m_rGripZ.y);
			pC->v("actuatorX", &c.m_actuatorX);
			pC->v("actuatorZ", &c.m_actuatorZ);
			pC->v("iROI", &c.m_iROI);

			int nC = pC->array("class", pClass, SB_N_CLASS);
			for (int j = 0; j < nC; j++)
				c.m_classFlag |= (1 << pClass[j]);

			iName = "";
			F_ERROR_F(pC->v("_Sequencer", &iName));
			c.m_pSeq = (_Sequencer*) (pK->root()->getChildInst(iName));
			IF_Fl(!c.m_pSeq, iName + " not found");

			m_vArmSet.push_back(c);
		}
	}

	iName = "";
	F_ERROR_F(pK->v("_SlideWindow", &iName));
	m_pSW = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pSW, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_DNNclassifier", &iName));
	m_pC = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pC, iName + " not found");

	return true;
}

bool _SortingBot::start(void)
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

void _SortingBot::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateTarget();
		updateArmset();

		this->autoFPSto();
	}
}

int _SortingBot::check(void)
{
	NULL__(m_pSW,-1);
	NULL__(m_pC,-1);

	return 0;
}

void _SortingBot::updateTarget(void)
{
	IF_(check()<0);

	int i;

	//update existing target positions
	float spd = m_cSpeed * ((float)m_dTime) * 1e-6;
	for (i=0; i<m_vTarget.size(); i++)
	{
		OBJECT* pO = &m_vTarget[i];
		pO->m_bb.y += spd;
		pO->m_bb.w += spd;
	}

	//delete targets out of range
	if(!m_vTarget.empty())
	{
		while(m_vTarget.front().m_bb.midY() > m_cLen)
		{
			m_vTarget.pop_front();
			if(m_vTarget.empty())return;
		}
	}

	//get new targets and compare to existing ones
	OBJECT* pO;
	i = 0;
	uint64_t tStamp = getTimeUsec();
	while ((pO = m_pSW->at(i++)) != NULL)
	{
		IF_CONT(pO->m_topClass < 0);

		int j;
		for (j = 0; j < m_vTarget.size(); j++)
		{
			OBJECT* pT = &m_vTarget[j];
			IF_CONT(nIoU(pT->m_bb, pO->m_bb) < m_minOverlap);

			if(pT->m_topProb < pO->m_topProb)
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

void _SortingBot::updateArmset(void)
{
	IF_(check()<0);

	for (int i = 0; i < m_vArmSet.size(); i++)
	{
		SB_ARMSET* pArm = &m_vArmSet[i];

		if(pArm->getCurrentActionName() == "standby")
		{
			for (int j = 0; j < m_vTarget.size(); j++)
			{
				OBJECT* pT = &m_vTarget[j];
				IF_CONT(!pArm->bClass(pT->m_topClass));
				IF_CONT(pT->m_bb.midY() < pArm->m_rGripY.x);
				IF_CONT(pT->m_bb.midY() > pArm->m_rGripY.y);
				pT->m_bb.y += m_cLen;
				pT->m_bb.w += m_cLen;

				SEQ_ACTION* pAction;
				SEQ_ACTUATOR* pSA;

				pAction = pArm->getAction("descent");
				IF_CONT(!pAction);

				pSA = pAction->getActuator(pArm->m_actuatorX);
				IF_CONT(pSA);
				pSA->m_pos = (1.0 - pT->m_bb.midX()) * pArm->m_rGripX.len() + pArm->m_rGripX.x;

				pSA = pAction->getActuator(pArm->m_actuatorZ);
				IF_CONT(pSA);
				pSA->m_pos = (pT->m_dist - pArm->m_rGripZ.x) / pArm->m_rGripZ.len();

				pAction = pArm->getAction("move");
				IF_CONT(!pAction);

				pSA = pAction->getActuator(pArm->m_actuatorX);
				IF_CONT(pSA);
				pSA->m_pos = m_pDropPos[pT->m_topClass];

				pArm->m_pSeq->wakeUp();
				break;
			}
		}
		else if(pArm->getCurrentActionName() == "verify")
		{
			OBJECT o = *m_pC->at(pArm->m_iROI);

		}
	}
}

bool _SortingBot::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _SortingBot::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
