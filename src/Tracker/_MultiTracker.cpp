/*
 * _MultiTracker.cpp
 *
 *  Created on: June 15, 2018
 *      Author: yankai
 */

#include "_MultiTracker.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

_MultiTracker::_MultiTracker()
{
	m_pTracker = NULL;
	m_nTracker = 8;
}

_MultiTracker::~_MultiTracker()
{
}

bool _MultiTracker::init(void* pKiss)
{
	IF_F(!this->_ObjectBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	Kiss* pT = pK->o("Tracker");
	NULL_F(pT);

	KISSm(pK, nTracker);
	m_pTracker = new _SingleTracker[m_nTracker];
	for(int i=0; i<m_nTracker; i++)
	{
		IF_F(!m_pTracker->init(pT));
	}

	return true;
}

bool _MultiTracker::link(void)
{
	IF_F(!this->_ObjectBase::link());
	Kiss* pK = (Kiss*)m_pKiss;

	string iName = "";
//	F_ERROR_F(pK->v("_VisionBase",&iName));
//	m_pVision = (_VisionBase*)(pK->root()->getChildInstByName(&iName));

	for(int i=0; i<m_nTracker; i++)
	{
		IF_F(!m_pTracker->link());
	}

	return true;
}

bool _MultiTracker::start(void)
{
	for(int i=0; i<m_nTracker; i++)
	{
		IF_F(!m_pTracker->start());
		m_pTracker->sleep();
	}

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _MultiTracker::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		track();

		this->autoFPSto();
	}
}

void _MultiTracker::track(void)
{
	OBJECT obj;
	double dx,dy;

	for (int i = 0; i < m_nTracker; i++)
	{
//		obj.init();
//		obj.m_tStamp = m_tStamp;
//		obj.setTopClass(0);
//
//		add(&obj);
//		LOG_I("ID: "+ i2str(obj.m_iClass));
	}
}

bool _MultiTracker::addROI(vInt4 roi)
{
	_SingleTracker* pT = getTracker();
	NULL_F(pT);

	IF_F(!pT->setROI(roi));
	pT->wakeUp();

	return true;
}

_SingleTracker* _MultiTracker::getTracker(void)
{
	for(int i=0; i<m_nTracker; i++)
	{
		_SingleTracker* pT = &m_pTracker[i];
		IF_CONT(pT->bTracking());
		return pT;
	}

	return NULL;
}

bool _MultiTracker::draw(void)
{
	IF_F(!this->_ObjectBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	return true;
}

bool _MultiTracker::cli(int& iY)
{
	IF_F(!this->_ObjectBase::cli(iY));

//	string msg = "| ";
//	OBJECT* pO;
//	int i=0;
//	while((pO = m_obj.at(i++)) != NULL)
//	{
//		msg += i2str(pO->m_iClass) + " | ";
//	}
//
//	COL_MSG;
//	iY++;
//	mvaddstr(iY, CLI_X_MSG, msg.c_str());

	return true;
}

}
#endif
