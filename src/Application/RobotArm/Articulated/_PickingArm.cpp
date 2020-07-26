/*
 * _PickingArm.cpp
 *
 *  Created on: June 30, 2020
 *      Author: yankai
 */

#include "../../RobotArm/Articulated/_PickingArm.h"

#ifdef USE_OPENCV

namespace kai
{

_PickingArm::_PickingArm()
{
	m_pA = NULL;
	m_pU = NULL;
	m_tO.init();
	m_mode = paMode_unknown;
	m_state = pa_standby;

	m_vTargetBB.init();
	m_vP.init();
	m_vP.x = 0.5;
	m_vP.y = 0.5;
	m_vTargetP.init();
	m_vTargetP.x = 0.5;
	m_vTargetP.y = 0.5;

	m_pXpid = NULL;
	m_pYpid = NULL;
	m_pZpid = NULL;

	m_vM.init(0.5);
	m_vR.init(0.5);
}

_PickingArm::~_PickingArm()
{
}

bool _PickingArm::init(void *pKiss)
{
	IF_F(!this->_MissionBase::init(pKiss));
	Kiss *pK = (Kiss*) pKiss;

	Kiss *pClass = pK->child("class");
	NULL_Fl(pClass, "class not found");

	int i = 0;
	while (1)
	{
		Kiss *pC = pClass->child(i++);
		if (pC->empty())
			break;

		PICKINGARM_CLASS pc;
		pc.init();
		pC->v("iClass", &pc.m_iClass);

		m_vClass.push_back(pc);
	}

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_ActuatorBase", &iName));
	m_pA = (_ActuatorBase*) (pK->getInst(iName));
	IF_Fl(!m_pA, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_Universe", &iName));
	m_pU = (_Universe*) (pK->getInst(iName));
	IF_Fl(!m_pU, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("PIDx", &iName));
	m_pXpid = (PIDctrl*) (pK->getInst(iName));
	IF_Fl(!m_pXpid, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("PIDy", &iName));
	m_pYpid = (PIDctrl*) (pK->getInst(iName));
	IF_Fl(!m_pYpid, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("PIDz", &iName));
	m_pZpid = (PIDctrl*) (pK->getInst(iName));
	IF_Fl(!m_pZpid, iName + " not found");

	return true;
}

bool _PickingArm::start(void)
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

void _PickingArm::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateArm();

		this->autoFPSto();
	}
}

int _PickingArm::check(void)
{
	NULL__(m_pA, -1);
	NULL__(m_pU, -1);
	NULL__(m_pXpid, -1);
	NULL__(m_pYpid, -1);
	NULL__(m_pZpid, -1);

	return 0;
}

void _PickingArm::updateArm(void)
{
	IF_(check() < 0);

	if(m_mode == paMode_external)
	{
		m_pA->setStarget(0, m_vM.x);
		m_pA->setStarget(1, m_vM.y);
		m_pA->setStarget(2, m_vM.z);

		m_pA->setStarget(6, m_vR.x);
		m_pA->setStarget(7, m_vR.y);
		m_pA->setStarget(8, m_vR.z);
	}
	else if(m_mode == paMode_auto)
	{
		_Object *pO;
		_Object *tO = NULL;
		float topProb = 0.0;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			IF_CONT(pO->getTopClass() != m_vClass);
			IF_CONT(pO->getTopClassProb() < topProb);

			tO = pO;
			topProb = pO->getTopClassProb();
		}

		if (tO)
		{
			m_vTargetBB = tO->getBB2D();
			float x = m_vTargetBB.midX() - m_vTargetP.x;
			float y = m_vTargetBB.midY() - m_vTargetP.y;
			float angle = m_pA->getRawP(3);

			float s = sin(angle);
			float c = cos(angle);
			m_vP.x = x*c - y*s + m_vTargetP.x;
			m_vP.y = x*s + y*c + m_vTargetP.y;

			m_pA->setStarget(0, m_pXpid->update(m_vP.x, m_vTargetP.x, m_tStamp));
			m_pA->setStarget(1, m_pYpid->update(m_vP.y, m_vTargetP.y, m_tStamp));
			m_pA->setStarget(2, m_pZpid->update(m_vP.z, m_vTargetP.z, m_tStamp));
		}
	}
}

void _PickingArm::setMode(PICKINGARM_MODE m)
{
	m_mode = m;
}

void _PickingArm::move(vFloat4& vM)
{
	m_vM = vM;
}

void _PickingArm::rotate(vFloat4& vR)
{
	m_vR = vR;
}

void _PickingArm::draw(void)
{
	this->_MissionBase::draw();

	addMsg("vM = (" + f2str(m_vM.x) + ", " + f2str(m_vM.y) + ", " + f2str(m_vM.z) + ", " + f2str(m_vM.w) + ")");
}

}
#endif
