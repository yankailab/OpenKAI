/*
 * _SortingImg.cpp
 *
 *  Created on: May 28, 2019
 *      Author: yankai
 */

#include "_SortingImg.h"

namespace kai
{

_SortingImg::_SortingImg()
{
	m_pOL = NULL;
	m_pDet = NULL;
}

_SortingImg::~_SortingImg()
{
}

bool _SortingImg::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	string iName;

	iName = "";
	F_ERROR_F(pK->v("_OKlink", &iName));
	m_pOL = (_OKlink*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pOL, iName + " not found");

	iName = "";
	F_ERROR_F(pK->v("_DetectorBase", &iName));
	m_pDet = (_DetectorBase*) (pK->root()->getChildInst(iName));
	IF_Fl(!m_pDet, iName + " not found");

	return true;
}

bool _SortingImg::start(void)
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

void _SortingImg::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		updateImg();

		this->autoFPSto();
	}
}

int _SortingImg::check(void)
{
	NULL__(m_pOL, -1);
	NULL__(m_pDet, -1);

	return 0;
}

void _SortingImg::updateImg(void)
{
	IF_(check() < 0);

}

bool _SortingImg::draw(void)
{
	IF_F(!this->_ThreadBase::draw());
	Window* pWin = (Window*) this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _SortingImg::console(int& iY)
{
	IF_F(!this->_ThreadBase::console(iY));

	string msg;

	return true;
}

}
