/*
 * _DNNodometryTrain.cpp
 *
 *  Created on: May 17, 2017
 *      Author: yankai
 */

#include "../Odometry/_DNNodometryTrain.h"

namespace kai
{

_DNNodometryTrain::_DNNodometryTrain()
{
	m_pIN = NULL;
	m_pZED = NULL;
	m_attiRad.init();
	m_tNow = 0;
}

_DNNodometryTrain::~_DNNodometryTrain()
{

}

bool _DNNodometryTrain::init(void* pKiss)
{
	IF_F(!_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	return true;
}

bool _DNNodometryTrain::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName;

	iName = "";
	F_INFO(pK->v("_ImageNet", &iName));
	m_pIN = (_ImageNet*) (pK->root()->getChildInstByName(&iName));

	iName = "";
	F_ERROR_F(pK->v("_ZED", &iName));
	m_pZED = (_ZED*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _DNNodometryTrain::start(void)
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

void _DNNodometryTrain::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();
		m_tNow = get_time_usec();

		this->autoFPSto();
	}
}

bool _DNNodometryTrain::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Mat* pMat = ((Window*) this->m_pWindow)->getFrame()->getCMat();
	IF_F(pMat->empty());

	return true;
}

}
