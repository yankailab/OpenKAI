/*
 * _Gripper.cpp
 *
 *  Created on: May 26, 2019
 *      Author: yankai
 */

#include "_Gripper.h"

namespace kai
{

_Gripper::_Gripper()
{
	m_pDet = NULL;

}

_Gripper::~_Gripper()
{
}

bool _Gripper::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;


	string iName;

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDet, iName + " not found");

	return true;
}

bool _Gripper::start(void)
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

void _Gripper::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();


		this->autoFPSto();
	}
}

int _Gripper::check(void)
{
	NULL__(m_pDet,-1);

	return 0;
}

bool _Gripper::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();


	return true;
}

bool _Gripper::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
