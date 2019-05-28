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
}

_ClassifierBot::~_ClassifierBot()
{
}

bool _ClassifierBot::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

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

		updateAction();

		this->autoFPSto();
	}
}

int _ClassifierBot::check(void)
{
	return 0;
}

void _ClassifierBot::updateAction(void)
{
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
