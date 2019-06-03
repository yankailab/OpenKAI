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
	m_pDet = NULL;
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
			pC->v("iActionStandby", &c.m_iActionStandby);
			pC->v("iActionGrip", &c.m_iActionGrip);
			pC->v("iActionDrop", &c.m_iActionDrop);
			pC->v("iActuatorX", &c.m_iActuatorX);
			pC->v("iActuatorZ", &c.m_iActuatorZ);

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
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDet, iName + " not found");

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
	NULL__(m_pDet,-1);

	return 0;
}

void _SortingBot::updateTarget(void)
{
	IF_(check()<0);

	//update existing target positions
	float spd = m_cSpeed * ((float)m_dTime) * 1e-6;
	for (SB_TARGET t : m_vTarget)
		t.moveY(spd);

	//delete targets out of range
	while(m_vTarget.front().m_bb.midY() > m_cLen)
	{
		m_vTarget.pop_front();
		if(m_vTarget.empty())break;
	}

	//get new targets and compare to existing ones
	OBJECT* pO;
	int i = 0;
	uint64_t tStamp = getTimeUsec();
	while ((pO = m_pDet->at(i++)) != NULL)
	{
		IF_CONT(pO->m_topClass < 0);

		int j;
		for (j = 0; j < m_vTarget.size(); j++)
		{
			SB_TARGET* pT = &m_vTarget[j];
			if (nIoU(pT->m_bb, pO->m_bb) > m_minOverlap)
				break;
		}
		IF_CONT(j < m_vTarget.size());

		SB_TARGET t;
		t.init();
		t.m_bb = pO->m_bb;
		t.m_iClass = pO->m_topClass;
		t.m_d = pO->m_dist;
		t.m_tStamp = tStamp;
		m_vTarget.push_back(t);
	}
}

void _SortingBot::updateArmset(void)
{
	//assign armset target and drop destination, resume the armset
	for (int i = 0; i < m_vArmSet.size(); i++)
	{
		SB_ARMSET* pA = &m_vArmSet[i];
		if(pA->bDrop())
		{
			pA->m_bTarget = false;
			pA->m_pSeq->wakeUp();
		}
		IF_CONT(pA->m_bTarget);
		IF_CONT(!pA->bStandby());
		IF_CONT(!pA->bComplete());

		for (int j = 0; j < m_vTarget.size(); j++)
		{
			SB_TARGET* pT = &m_vTarget[j];
			IF_CONT(!pA->bClass(pT->m_iClass));
			IF_CONT(pT->m_bb.midY() < pA->m_rGripY.x);
			IF_CONT(pT->m_bb.midY() > pA->m_rGripY.y);
			pT->moveY(m_cLen);

			SEQUENCER_ACTION* pS;
			pS = pA->getAction(pA->m_iActionGrip);
			IF_CONT(!pS);
			pS->m_pNpos[pA->m_iActuatorX] = (1.0 - pT->m_bb.midX()) * pA->m_rGripX.len() + pA->m_rGripX.x;
			pS->m_pNpos[pA->m_iActuatorZ] = pT->m_d * pA->m_rGripZ.len() + pA->m_rGripZ.x;

			pS = pA->getAction(pA->m_iActionDrop);
			IF_CONT(!pS);
			pS->m_pNpos[pA->m_iActuatorX] = m_pDropPos[pT->m_iClass];

			pA->m_bTarget = true;
			pA->m_pSeq->wakeUp();
			break;
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
