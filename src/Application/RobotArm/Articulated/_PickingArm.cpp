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
	m_mode = paMode_unknown;
	m_state = pa_standby;

	m_pXpid = NULL;
	m_pYpid = NULL;
	m_pZpid = NULL;
	m_tO.init();
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
		if (pClass->empty())
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

	iName = "";
	F_ERROR_F(pK->v("PIDy", &iName));
	m_pYpid = (PIDctrl*) (pK->getInst(iName));

	iName = "";
	F_ERROR_F(pK->v("PIDz", &iName));
	m_pZpid = (PIDctrl*) (pK->getInst(iName));

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

	vFloat4 vM;
	vM.init(-1.0);
	m_pA->pos(vM);

	if(m_mode == paMode_external)
	{
		vM = m_vM;
	}
	else if(m_mode == paMode_auto)
	{
		_Object *pO;
		_Object *tO = NULL;
		float topProb = 0.0;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
	//		IF_CONT(pO->getTopClass() != m_vClass);
			IF_CONT(pO->getTopClassProb() < topProb);

			tO = pO;
			topProb = pO->getTopClassProb();
		}

		vM.init(0.5);
		if (tO)
		{
			vM.x = m_pXpid->update(m_vP.x, m_vTargetP.x, m_tStamp);
			vM.y = m_pXpid->update(m_vP.y, m_vTargetP.y, m_tStamp);
			vM.z = m_pXpid->update(m_vP.z, m_vTargetP.z, m_tStamp);
		}
	}

	m_pA->speed(vM);
}

void _PickingArm::setMode(PICKINGARM_MODE m)
{
	m_mode = m;
}

void _PickingArm::move(vFloat4& vM)
{
	m_vM = vM;
}

void _PickingArm::draw(void)
{
	this->_MissionBase::draw();

}

}
#endif
