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
	m_pD = NULL;
	m_nClass = 0;
	m_classFlag = 0;
	m_tO.init();
}

_PickingArm::~_PickingArm()
{
}

bool _PickingArm::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	vector<int> vClass;
	m_nClass = pK->a("vClass", &vClass);
	for (int i = 0; i < m_nClass; i++)
		m_classFlag |= (1 << vClass[i]);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_ActuatorBase", &iName));
	m_pA = (_ActuatorBase*) (pK->getInst(iName));
	IF_Fl(!m_pA, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pD = (_DetectorBase*) (pK->getInst(iName));
	IF_Fl(!m_pD, iName + " not found");

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
	NULL__(m_pD, -1);

	return 0;
}

void _PickingArm::updateArm(void)
{
	IF_(check() < 0);

}

void _PickingArm::draw(void)
{
	this->_ThreadBase::draw();


}

}
#endif
