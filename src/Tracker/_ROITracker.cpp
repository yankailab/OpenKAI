/*
 * _ROITracker.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_ROITracker.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

_ROITracker::_ROITracker()
{
	m_pStream = NULL;
	m_pFrame = NULL;
	m_bTracking = false;
}

_ROITracker::~_ROITracker()
{
}

bool _ROITracker::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*)pKiss;
	pK->m_pInst = this;

	//format params
	m_ROI.width = 0;
	m_ROI.height = 0;
	m_ROI.x = 0;
	m_ROI.y = 0;

	m_newROI = m_ROI;
	m_bTracking = false;
	m_pFrame = new Frame();

	// create a tracker object
	m_pTracker = TrackerKCF::create();

	return true;
}

bool _ROITracker::link(void)
{
	NULL_F(m_pKiss);
	Kiss* pK = (Kiss*)m_pKiss;

	//link instance
	string iName = "";
	F_ERROR_F(pK->v("_VisionBase",&iName));
	m_pStream = (_VisionBase*)(pK->root()->getChildInstByName(&iName));

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
	pMat = m_pFrame->m();

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

	NULL_(m_pStream);
	IF_(!m_bTracking);
	IF_(m_pTracker.empty());

	pFrame = m_pStream->BGR();
	NULL_(pFrame);
	IF_(*pFrame <= *m_pFrame);
	*m_pFrame = *pFrame;

	pMat = m_pFrame->m();
	if (pMat->empty())
		return;

	if (m_newROI.width > 0)
	{
		m_pTracker.release();
		m_pTracker = TrackerKCF::create();

		m_ROI = m_newROI;
		m_pTracker->init(*pMat, m_ROI);
		m_newROI.width = 0;
	}

	// update the tracking result
	m_pTracker->update(*pMat, m_ROI);

}

}

#endif
