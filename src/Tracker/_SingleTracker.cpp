/*
 * _SingleTracker.cpp
 *
 *  Created on: Aug 21, 2015
 *      Author: yankai
 */

#include "_SingleTracker.h"

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
//	if (m_trackerType == "mil")
//		m_pTracker = TrackerMIL::create();
//	else if (m_trackerType == "goturn")
//		m_pTracker = TrackerGOTURN::create();
//	else if (m_trackerType == "csrt")
//		m_pTracker = TrackerCSRT::create();
//	else
		m_pTracker = TrackerKCF::create();
}

bool _SingleTracker::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _SingleTracker::update(void)
{
	while(m_pT->bRun())
	{
		m_pT->autoFPSfrom();

		track();

		m_pT->autoFPSto();
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

//		m_pTracker->update(m, m_rBB);
//		m_bb = bbScale(rect2BB<vFloat4>(m_rBB), 1.0/m.cols, 1.0/m.rows);
	}
}

}
