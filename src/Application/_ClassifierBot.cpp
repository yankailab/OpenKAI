/*
 * _ClassifierBot.cpp
 *
 *  Created on: May 26, 2019
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

			pC->v("tGrip", &c.m_tGrip);
			pC->v("iActionGripStandby", &c.m_iActionGripStandby);
			pC->v("iActionDrop", &c.m_iActionDrop);
			pC->v("iActuatorX", &c.m_iActuatorX);

			int nC = pC->array("class", pClass, CB_N_CLASS);
			for (int j = 0; j < nC; j++)
				c.m_classFlag |= (1 << pClass[j]);

			iName = "";
			F_ERROR_F(pC->v("_Sequencer", &iName));
			c.m_pArm = (_Sequencer*) (pK->root()->getChildInst(iName));
			IF_Fl(!c.m_pArm, iName + " not found");

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
	int i,j;

	//update existing target positions
	for (i = 0; i < m_vTarget.size(); i++)
	{
		CBOT_TARGET* pC = &m_vTarget[i];

		float spd = m_speed;//
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
			if(bbOverlap(pC->m_bb, pO->m_bb) > m_bbOverlap)
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
	for (int i = 0; i < m_vTarget.size(); i++)
	{
		CBOT_TARGET* pC = &m_vTarget[i];

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
