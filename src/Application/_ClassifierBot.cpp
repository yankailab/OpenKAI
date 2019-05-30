/*
 * _ClassifierBot.cpp
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#include "_ClassifierBot.h"

namespace kai
{

_ClassifierBot::_ClassifierBot()
{
	m_pDet = NULL;
	m_speed = 0.0;
	m_nClass = 0;
	m_cLen = 2.0;
	m_kD = 1.0;
	m_bbOverlap = 0.8;
}

_ClassifierBot::~_ClassifierBot()
{
}

bool _ClassifierBot::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	KISSm(pK, nClass);
	KISSm(pK, speed);
	KISSm(pK, cLen);
	KISSm(pK, kD);
	pK->array("dropPos", m_pDropPos, CB_N_CLASS);

	Kiss* pA;
	Kiss** pItr;
	string iName;
	int i;
	int pClass[CB_N_CLASS];

	pA = pK->o("armSet");
	if (pA)
	{
		pItr = pA->getChildItr();
		i = 0;
		Kiss* pC;

		while ((pC = pItr[i++]))
		{
			CBOT_ARMSET c;
			c.init();

			pC->v("gripX1", &c.m_rGripX.x);
			pC->v("gripX2", &c.m_rGripX.y);
			pC->v("gripY1", &c.m_rGripY.x);
			pC->v("gripY2", &c.m_rGripY.y);
			pC->v("iActionGripStandby", &c.m_iActionGripStandby);
			pC->v("iActionDrop", &c.m_iActionDrop);
			pC->v("iActuatorX", &c.m_iActuatorX);

			int nC = pC->array("class", pClass, CB_N_CLASS);
			for (int j = 0; j < nC; j++)
				c.m_classFlag |= (1 << pClass[j]);

			iName = "";
			F_ERROR_F(pC->v("_Sequencer", &iName));
			c.m_pSeq = (_Sequencer*) (pK->root()->getChildInst(iName));
			IF_Fl(!c.m_pSeq, iName + " not found");

			m_vArmSet.push_back(c);
		}
	}

	return true;
}

bool _ClassifierBot::start(void)
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

void _ClassifierBot::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateTarget();
		updateArmset();

		this->autoFPSto();
	}
}

int _ClassifierBot::check(void)
{
	return 0;
}

void _ClassifierBot::updateTarget(void)
{
	int i, j;

	//update existing target positions
	for (i = 0; i < m_vTarget.size(); i++)
	{
		CBOT_TARGET* pC = &m_vTarget[i];

		float spd = m_speed; //
		pC->m_bb.y += spd;
		pC->m_bb.w += spd;
	}

	//delete targets out of range
	while (m_vTarget.size() > 0)
	{
		CBOT_TARGET* pC = &m_vTarget[i];
		if (pC->m_bb.midY() > m_cLen)
			m_vTarget.erase(m_vTarget.begin());
	}

	//get new targets and compare to existing ones
	OBJECT* pO;
	i = 0;
	uint64_t tStamp = getTimeUsec();
	while ((pO = m_pDet->at(i++)) != NULL)
	{
		for (j = 0; j < m_vTarget.size(); j++)
		{
			CBOT_TARGET* pC = &m_vTarget[j];
//			if (bbOverlap(pC->m_bb, pO->m_bb) > m_bbOverlap)
//				break;

			Rect2f r1, r2;
			vFloat42rect(pO->m_bb, r1);
			vFloat42rect(pC->m_bb, r2);
			Rect2f rOR = r1 | r2;
			Rect2f rAND = r1 & r2;
			float IoU = rAND.area() / rOR.area();
			if (IoU > m_bbOverlap)
				break;
		}

		IF_CONT(j < m_vTarget.size());

		CBOT_TARGET c;
		c.init();
		c.m_bb = pO->m_bb;
		c.m_iClass = pO->m_topClass;
		c.m_tStamp = tStamp;
		m_vTarget.push_back(c);
	}
}

void _ClassifierBot::updateArmset(void)
{
	//assign armset target and drop destination, resume the armset, and delete the target from vector
	for (int i = 0; i < m_vArmSet.size(); i++)
	{
		CBOT_ARMSET* pA = &m_vArmSet[i];
		if(pA->m_pSeq->m_iAction == pA->m_iActionDrop)
		{
			pA->m_bTarget = false;
			pA->m_pSeq->wakeUp();
		}
		IF_CONT(pA->m_bTarget);
		IF_CONT(pA->m_pSeq->m_iAction != pA->m_iActionGripStandby);

		for (int j = 0; j < m_vTarget.size(); j++)
		{
			CBOT_TARGET* pT = &m_vTarget[j];
			IF_CONT(pA->m_classFlag & (1 << pT->m_iClass));
			IF_CONT(pT->m_bb.midY() < pA->m_rGripY.x);
			IF_CONT(pT->m_bb.midY() > pA->m_rGripY.y);
			pT->m_bb.y = m_cLen;
			pT->m_bb.w = m_cLen;

			SEQUENCER_ACTION* pS = pA->m_pSeq->getAction(pA->m_iActionDrop);
			IF_CONT(!pS);
			pS->m_pNpos[pA->m_iActuatorX] = m_pDropPos[pT->m_iClass];
			pA->m_bTarget = true;
			pA->m_pSeq->wakeUp();
		}
	}
}

bool _ClassifierBot::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _ClassifierBot::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
