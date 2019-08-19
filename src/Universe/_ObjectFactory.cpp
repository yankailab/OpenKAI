/*
 *  Created on: June 25, 2019
 *      Author: yankai
 */
#include "_ObjectFactory.h"

namespace kai
{

_ObjectFactory::_ObjectFactory()
{
	m_nCap = 128;
	m_nO = 0;
	m_pO = NULL;
}

_ObjectFactory::~_ObjectFactory()
{
}

bool _ObjectFactory::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("nCap",&m_nCap);

	return true;
}

bool _ObjectFactory::start(void)
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

void _ObjectFactory::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->autoFPSto();
	}
}

void _ObjectFactory::updateObjects(void)
{

}

bool _ObjectFactory::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Frame* pFrame = pWin->getFrame();
	Mat* pMat = pFrame->m();
	IF_F(pMat->empty());

	return true;
}

bool _ObjectFactory::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
