/*
 * _Gripper.cpp
 *
 *  Created on: May 26, 2019
 *      Author: yankai
 */

#include "_PLC.h"

namespace kai
{

_PLC::_PLC()
{
	m_pDet = NULL;
	m_iAction = 0;

	m_pAX = NULL;
	m_pAY = NULL;
	m_pAZ = NULL;
}

_PLC::~_PLC()
{
}

bool _PLC::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	Kiss** pItr = pK->getChildItr();
	PLC_ACTION a;
	unsigned int i = 0;
	while (pItr[i])
	{
		Kiss* pC = pItr[i++];
		a.init();
		pC->v("nX", &a.m_nPosX);
		pC->v("nY", &a.m_nPosY);
		pC->v("nZ", &a.m_nPosZ);
		pC->v("dT", &a.m_dT);
		m_vAction.push_back(a);
	}

	string iName;

//	iName = "";
//	F_ERROR_F(pK->v("_DetectorBase", &iName));
//	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
//	IF_Fl(!m_pDet, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_ActuatorBaseX", &iName));
	m_pAX = (_ActuatorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAX, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_ActuatorBaseY", &iName));
	m_pAY = (_ActuatorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAY, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_ActuatorBaseZ", &iName));
	m_pAZ = (_ActuatorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pAZ, iName + " not found");

	return true;
}

bool _PLC::start(void)
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

void _PLC::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateAction();

		this->autoFPSto();
	}
}

int _PLC::check(void)
{
//	NULL__(m_pDet,-1);
	NULL__(m_pAX,-1);
	NULL__(m_pAY,-1);
	NULL__(m_pAZ,-1);

	return 0;
}

void _PLC::updateAction(void)
{
	IF_(m_vAction.size()<=0);

	PLC_ACTION* pA = &m_vAction[m_iAction];

	m_pAX->moveTo(pA->m_nPosX, 1.0);
	m_pAY->moveTo(pA->m_nPosY, 1.0);
	m_pAZ->moveTo(pA->m_nPosZ, 1.0);

	IF_(!EAQ(m_pAX->m_nCurrentPos , pA->m_nPosX));
	IF_(!EAQ(m_pAY->m_nCurrentPos , pA->m_nPosY));
	IF_(!EAQ(m_pAZ->m_nCurrentPos , pA->m_nPosZ));

	if(pA->m_dT > 0)
		usleep(pA->m_dT);

	m_iAction++;
	if(m_iAction >= m_vAction.size())
		m_iAction = 0;
}

bool _PLC::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();


	return true;
}

bool _PLC::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
