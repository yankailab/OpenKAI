/*
 * _ROITracker.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_ROITracker.h"

#include "stdio.h"
#include "../Base/common.h"
#include "../Base/cvplatform.h"

namespace kai
{

_ROITracker::_ROITracker()
{
	_ThreadBase();

	m_pCamStream = NULL;
	m_pFrame = NULL;
	m_bTracking = false;
}

_ROITracker::~_ROITracker()
{
}

bool _ROITracker::init(Config* pConfig, string* pName)
{
	if (this->_ThreadBase::init(pConfig,pName)==false)
		return false;

	m_ROI.width = 0;
	m_ROI.height = 0;
	m_ROI.x = 0;
	m_ROI.y = 0;

	m_newROI = m_ROI;
	m_bTracking = false;
	m_pFrame = new Frame();

	// create a tracker object
	m_pTracker = Tracker::create("KCF");

	return true;
}

bool _ROITracker::start(void)
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

void _ROITracker::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		track();

		this->autoFPSto();
	}

}

void _ROITracker::setROI(Rect2d roi)
{
	Mat* pMat;
	pMat = m_pFrame->getCMat();

	if (pMat->empty())
		return;
	if (roi.width == 0 || roi.height == 0)
		return;

	m_newROI = roi;
}

void _ROITracker::tracking(bool bTracking)
{
	m_bTracking = bTracking;
}

void _ROITracker::track(void)
{
	Frame* pFrame;
	Mat* pMat;

	if (m_pCamStream == NULL)
		return;
	if (m_bTracking == false)
		return;
	if (m_pTracker.empty())
		return;

	pFrame = m_pCamStream->getBGRFrame();
	if (!pFrame->isNewerThan(m_pFrame))
		return;
	m_pFrame->update(pFrame);

	pMat = m_pFrame->getCMat();
	if (pMat->empty())
		return;

	if (m_newROI.width > 0)
	{
		m_pTracker.release();
		m_pTracker = Tracker::create("KCF");

		m_ROI = m_newROI;
		m_pTracker->init(*pMat, m_ROI);
		m_newROI.width = 0;
	}

	// update the tracking result
	m_pTracker->update(*pMat, m_ROI);

}

} /* namespace kai */
