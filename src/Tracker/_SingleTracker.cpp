/*
 * _SingleTracker.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_SingleTracker.h"

#ifdef USE_OPENCV_CONTRIB

namespace kai
{

_SingleTracker::_SingleTracker()
{
	m_pVision = NULL;
	m_trackerType = "";
	m_tStampBGR = 0;
	m_bTracking = false;
}

_SingleTracker::~_SingleTracker()
{
	if(!m_pTracker.empty())
		m_pTracker.release();
}

bool _SingleTracker::init(void* pKiss)
{
	IF_F(!this->_ThreadBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,trackerType);

	return true;
}

void _SingleTracker::createTracker(void)
{
	if (m_trackerType == "boosting")
		m_pTracker = TrackerBoosting::create();
	else if (m_trackerType == "mil")
		m_pTracker = TrackerMIL::create();
	else if (m_trackerType == "tld")
		m_pTracker = TrackerTLD::create();
	else if (m_trackerType == "medianFlow")
		m_pTracker = TrackerMedianFlow::create();
	else if (m_trackerType == "goturn")
		m_pTracker = TrackerGOTURN::create();
	else if (m_trackerType == "mosse")
		m_pTracker = TrackerMOSSE::create();
	else if (m_trackerType == "csrt")
		m_pTracker = TrackerCSRT::create();
	else
		m_pTracker = TrackerKCF::create();
}

bool _SingleTracker::link(void)
{
	IF_F(!this->_ThreadBase::link());
	Kiss* pK = (Kiss*) m_pKiss;

	string iName = "";
	F_ERROR_F(pK->v("_VisionBase", &iName));
	m_pVision = (_VisionBase*) (pK->root()->getChildInstByName(&iName));

	return true;
}

bool _SingleTracker::start(void)
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

void _SingleTracker::update(void)
{
	while (m_bThreadON)
	{
		this->autoFPSfrom();

		track();

		this->autoFPSto();
	}
}

bool _SingleTracker::updateROI(vInt4 roi)
{
	NULL_F(m_pVision);
	Mat* pMat = m_pVision->BGR()->m();
	IF_F(pMat->empty());

	Rect2d rRoi;
	vInt42rect(roi,rRoi);

	IF_F(rRoi.width == 0 || rRoi.height == 0);

	if(!m_pTracker.empty())
		m_pTracker.release();

	createTracker();
	m_pTracker->init(*pMat, rRoi);
	m_ROI = rRoi;
	m_bTracking = true;

	return true;
}

void _SingleTracker::track(void)
{
	NULL_(m_pVision);
	IF_(!m_bTracking);
	IF_(m_pTracker.empty());

	Frame* pFrame = m_pVision->BGR();
	IF_(pFrame->tStamp() <= m_tStampBGR);
	m_tStampBGR = pFrame->tStamp();

	Mat* pMat = pFrame->m();
	IF_(pMat->empty());

	m_pTracker->update(*pMat, m_ROI);
}

bool _SingleTracker::bTracking(void)
{
	return m_bTracking;
}

}
#endif
