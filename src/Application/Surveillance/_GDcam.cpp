/*
 * _GDcam.cpp
 *
 *  Created on: Feb 26, 2019
 *      Author: yankai
 */

#include "_GDcam.h"

namespace kai
{

_GDcam::_GDcam()
{
	m_pD = NULL;

	m_imgFile = "anr.jpg";
	m_shAlpr = "alpr.sh";

}

_GDcam::~_GDcam()
{
}

bool _GDcam::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

//	KISSm(pK,);

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_DetectorBaseCN", &iName));
	m_pD = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pD, iName + " not found");

	return true;
}

bool _GDcam::start(void)
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

void _GDcam::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->autoFPSto();
	}
}

int _GDcam::check(void)
{
	NULL__(m_pD,-1);

	return 0;
}

bool _GDcam::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _GDcam::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
