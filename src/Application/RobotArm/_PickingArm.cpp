/*
 * _PickingArm.cpp
 *
 *  Created on: June 30, 2020
 *      Author: yankai
 */

#include "_PickingArm.h"

#ifdef USE_OPENCV

namespace kai
{

_PickingArm::_PickingArm()
{
	m_pA = NULL;
	m_pG = NULL;
	m_pD = NULL;
	m_pU = NULL;
	m_mode = paMode_unknown;
	m_state = pa_standby;

	m_baseAngle = 0.0;
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

	pK->v("mode", (int*)&m_mode);

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
	NULL_Fl(m_pA, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_StepperGripper", &iName));
	m_pG = (_StepperGripper*)pK->getInst(iName);
	NULL_Fl(m_pG, iName + ": not found");

	iName = "";
	F_ERROR_F(pK->v("_Universe", &iName));
	m_pU = (_Universe*) (pK->getInst(iName));
	NULL_Fl(m_pU, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_DistSensorBase", &iName));
	m_pD = (_DistSensorBase*) (pK->getInst(iName));
	NULL_Fl(m_pD, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("PIDx", &iName));
	m_pXpid = (PIDctrl*) (pK->getInst(iName));
	NULL_Fl(m_pXpid, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("PIDy", &iName));
	m_pYpid = (PIDctrl*) (pK->getInst(iName));
	NULL_Fl(m_pYpid, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("PIDz", &iName));
	m_pZpid = (PIDctrl*) (pK->getInst(iName));
	NULL_Fl(m_pZpid, iName + " not found");

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
	NULL__(m_pG, -1);
	NULL__(m_pD, -1);
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

		m_pA->setStarget(3, m_vR.x);
		m_pA->setStarget(4, m_vR.y);
		m_pA->setStarget(5, m_vR.z);
	}
	else if(m_mode == paMode_auto)
	{
		_Object *pO;
		_Object *tO = NULL;
		float topProb = 0.0;
		int i = 0;
		while ((pO = m_pU->get(i++)) != NULL)
		{
			if(!m_vClass.empty())
			{
				IF_CONT(pO->getTopClass() != m_vClass[0].m_iClass);
			}
			IF_CONT(pO->getTopClassProb() < topProb);

			tO = pO;
			topProb = pO->getTopClassProb();
		}

		if (tO)
		{
			m_baseAngle = -m_pA->getPraw(3);
			if(m_baseAngle == FLT_MAX)m_baseAngle = 0.0;
			float rad = m_baseAngle * DEG_2_RAD;
			float s = sin(rad);
			float c = cos(rad);

			vFloat3 vP = tO->getPos();
			float x = vP.x - m_vTargetP.x;
			float y = vP.y - m_vTargetP.y;
			m_vP.x = x*c - y*s + m_vTargetP.x;
			m_vP.y = x*s + y*c + m_vTargetP.y;

			m_vM.y = m_pXpid->update(m_vP.x, m_vTargetP.x, m_tStamp);
			m_vM.x = m_pYpid->update(m_vP.y, m_vTargetP.y, m_tStamp);
			m_vM.z = 0.5;//m_pZpid->update(m_vP.z, m_vTargetP.z, m_tStamp);
		}
		else
		{
			m_vM.init(0.5);
		}

		m_pA->setStarget(0, m_vM.x);
		m_pA->setStarget(1, m_vM.y);
		m_pA->setStarget(2, m_vM.z);
	}
}

void _PickingArm::setMode(PICKINGARM_MODE m)
{
	m_mode = m;
}

void _PickingArm::move(vFloat3& vM)
{
	m_vM = vM;
}

void _PickingArm::rotate(vFloat3& vR)
{
	m_vR = vR;
}

void _PickingArm::grip(bool bOpen)
{
	IF_(check() < 0);

	m_pG->grip(bOpen);
}

void _PickingArm::draw(void)
{
	this->_MissionBase::draw();

	addMsg("vM = (" + f2str(m_vM.x) + ", " + f2str(m_vM.y) + ", " + f2str(m_vM.z) + ")");
	addMsg("vR = (" + f2str(m_vR.x) + ", " + f2str(m_vR.y) + ", " + f2str(m_vR.z) + ")");
	addMsg("vP = (" + f2str(m_vP.x) + ", " + f2str(m_vP.y) + ")");
	addMsg("baseAngle = " + f2str(m_baseAngle));

	IF_(!checkWindow());
	Mat* pM = ((Window*) this->m_pWindow)->getFrame()->m();
	Point pC = Point(m_vP.x * pM->cols, m_vP.y * pM->rows);
	circle(*pM, pC, 5.0, Scalar(255, 255, 0), 2);
}

}
#endif
