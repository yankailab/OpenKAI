/*
 * _ECO.cpp
 *
 *  Created on: July 15, 2018
 *      Author: yankai
 */

#include "_ECO.h"

namespace kai
{

_ECO::_ECO()
{
	m_pVision = NULL;
	m_bTracking = false;
	m_roi.init();
}

_ECO::~_ECO()
{
}

bool _ECO::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

//	Rect2f ecobbox(x, y, w, h);
//    m_tracker.init(frame, ecobbox);

    m_bTracking = false;

	return true;
}

bool _ECO::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*)m_pKiss;

//	string iName = "";
//	F_ERROR_F(pK->v("_VisionBase",&iName));
//	m_pVision = (_VisionBase*)(pK->root()->getChildInstByName(&iName));
//	NULL_Fl(m_pVision, iName + " not found");

	return true;
}

bool _ECO::start(void)
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

void _ECO::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		track();

		this->autoFPSto();
	}

}

void _ECO::startTrack(vDouble4& roi)
{
//	Mat* pMat;
//	pMat = m_pFrame->m();
//
//	if (pMat->empty())
//		return;
//	if (roi.width == 0 || roi.height == 0)
//		return;

}

void _ECO::stopTrack(void)
{
}

void _ECO::track(void)
{
//	Frame* pFrame;
//	Mat* pMat;
//
//	NULL_(m_pVision);
//	IF_(!m_bTracking);
//
//	pFrame = m_pVision->BGR();
//	NULL_(pFrame);
//	IF_(pFrame->tStamp() <= m_pFrame->tStamp());
//	*m_pFrame = *pFrame;
//
//	pMat = m_pFrame->m();
//	if (pMat->empty())
//		return;
//
//	if (m_newROI.width > 0)
//	{
//		m_pTracker.release();
//		m_pTracker = TrackerKCF::create();
//
//		m_ROI = m_newROI;
//		m_pTracker->init(*pMat, m_ROI);
//		m_newROI.width = 0;
//	}
//
//	// update the tracking result
//	m_pTracker->update(*pMat, m_ROI);
//
}

}
