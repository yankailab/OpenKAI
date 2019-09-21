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
}

_SingleTracker::~_SingleTracker()
{
	if(!m_pTracker.empty())
		m_pTracker.release();
}

bool _SingleTracker::init(void* pKiss)
{
	IF_F(!this->_TrackerBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

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

void _SingleTracker::track(void)
{
	IF_(check()<0);

	Frame* pFrame = m_pV->BGR();
	IF_(pFrame->bEmpty());
	Mat m = *pFrame->m();

	if(m_iSet > m_iInit)
	{
		//init a new track target
		if(!m_pTracker.empty())
			m_pTracker.release();

		createTracker();
		m_pTracker->init(m, m_newBB);
		m_trackState = track_update;
		m_rBB = m_newBB;
		m_iInit = m_iSet;
	}
	else
	{
		//track update
		IF_(m_trackState != track_update);
		IF_(m_pTracker.empty());

		m_pTracker->update(m, m_rBB);

		vFloat4 dBB = convertBB<vFloat4>(m_rBB);
		m_bb.x = dBB.x / (float)m.cols;
		m_bb.y = dBB.y / (float)m.rows;
		m_bb.z = dBB.z / (float)m.cols;
		m_bb.w = dBB.w / (float)m.rows;
	}

}

}
#endif
