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
	m_pOB = NULL;
	m_tStampOB = 0;
	m_pVision = NULL;
	m_trackerType = "";
	m_oBoundary = 0.0;
}

_MultiTracker::~_MultiTracker()
{
}

bool _MultiTracker::init(void* pKiss)
{
	IF_F(!this->_ObjectBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;

	KISSm(pK,trackerType);
	KISSm(pK,oBoundary);

	//link
	string iName = "";

	F_ERROR_F(pK->v("_ObjectBase",&iName));
	m_pOB = (_ObjectBase*)(pK->root()->getChildInstByName(&iName));

	F_ERROR_F(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _MultiTracker::start(void)
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

void _MultiTracker::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		this->_ObjectBase::update();
		addNewTarget();
		updateTarget();
		m_obj.update();

		this->autoFPSto();
	}
}

void _MultiTracker::addNewTarget(void)
{
	NULL_(m_pVision);
	NULL_(m_pOB);
	IF_(m_tStampOB >= m_pOB->m_tStamp);
	m_tStampOB = m_pOB->m_tStamp;

	OBJECT* pNewO;
	int i=0;
	while((pNewO = m_pOB->at(i++)) != NULL)
	{
		OBJECT* pO;
		int j=0;
		while((pO = m_obj.at(j++)) != NULL)
		{
			IF_CONT(overlapRatio(&pO->m_bb, &pNewO->m_bb) < m_dMaxTrack);

			//the obj is already being tracked, update its ROI size
			((_SingleTracker*)pO->m_pTracker)->updateBB(pNewO->m_bb);
			if(pNewO->m_topProb > pO->m_topProb)
			{
				pO->m_topProb = pNewO->m_topProb;
				pO->m_topClass = pNewO->m_topClass;
			}

			break;
		}
		IF_CONT(pO);

		//verify if the target is completely inside screen
		IF_CONT(pNewO->m_bb.x <= m_oBoundary);
		IF_CONT(pNewO->m_bb.y <= m_oBoundary);
		IF_CONT(pNewO->m_bb.z >= 1.0 - m_oBoundary);
		IF_CONT(pNewO->m_bb.w >= 1.0 - m_oBoundary);

		//new target found, allocate new tracker
		_SingleTracker* pT = new _SingleTracker();
		pT->setTargetFPS(m_FPS);
		pT->m_pVision = m_pVision;
		pT->m_trackerType = m_trackerType;
		if(!pT->start())
		{
			delete pT;
			continue;
		}
		if(!pT->updateBB(pNewO->m_bb))
		{
			delete pT;
			continue;
		}

		pNewO->m_pTracker = pT;
		m_obj.add(pNewO);
	}
}

void _MultiTracker::updateTarget(void)
{
	OBJECT* pO;
	int i=0;
	vInt2 cSize = m_pVision->getSize();
	while((pO = m_obj.at(i++)) != NULL)
	{
		_SingleTracker* pT = (_SingleTracker*)pO->m_pTracker;
		IF_CONT(!pT);

		vInt4 iBB;
		rect2vInt4(pT->m_rBB, iBB);
		pO->setBB(iBB, cSize);

		m_obj.add(pO);
	}
}

bool _MultiTracker::draw(void)
{
	IF_F(!this->_ObjectBase::draw());
	Window* pWin = (Window*)this->m_pWindow;
	Mat* pMat = pWin->getFrame()->m();

	m_nClass = m_pOB->m_nClass;

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
