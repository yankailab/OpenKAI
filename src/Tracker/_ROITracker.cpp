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
	m_pMat = NULL;
}

_ROITracker::~_ROITracker()
{
}

bool _ROITracker::init(JSON* pJson, string roiName)
{
	m_ROI.width = 0;
	m_ROI.height = 0;
	m_ROI.x = 0;
	m_ROI.y = 0;

	m_newROI = m_ROI;
	m_bTracking = false;

	// create a tracker object
	m_pTracker = Tracker::create( "KCF" );

	double FPS = DEFAULT_FPS;
	CHECK_ERROR(pJson->getVal("ROITRACKER_"+roiName+"_FPS", &FPS));
	this->setTargetFPS(FPS);

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
	if(m_pMat==NULL)return;
	if(m_pMat->empty())return;
	if(roi.width==0 || roi.height==0)return;

	m_newROI = roi;
}

void _ROITracker::tracking(bool bTracking)
{
	m_bTracking = bTracking;
}

void _ROITracker::track(void)
{
	if(m_pCamStream == NULL)return;
	m_pMat = m_pCamStream->getFrame()->getCMat();

	if(m_pMat->empty())return;
	if(m_bTracking==false)return;
	if(m_pTracker.empty())return;

	if(m_newROI.width > 0)
	{
		m_pTracker.release();
		m_pTracker = Tracker::create( "KCF" );

		m_ROI = m_newROI;
		m_pTracker->init(*m_pMat,m_ROI);
		m_newROI.width = 0;
	}

	// update the tracking result
    m_pTracker->update(*m_pMat,m_ROI);

}


} /* namespace kai */
