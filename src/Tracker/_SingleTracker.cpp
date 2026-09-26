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
		if (!m_pTracker.empty())
			m_pTracker.release();
	}

	bool _SingleTracker::loadConfig(void)
	{
		IF_F(!this->_TrackerBase::loadConfig());

		return true;
	}

	bool _SingleTracker::saveConfig(bool bExport)
	{
		if (!_TrackerBase::saveConfig(false))
		{
			return false;
		}

		if (!bExport)
		{
			return true;
		}
		return m_pJcfg->saveToFile();
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
		return m_pT->startThread(getUpdate, this);
	}

	void _SingleTracker::update(void)
	{
		while (m_pT->bRun())
		{
			m_pT->autoFPS();

			track();
		}
	}

	void _SingleTracker::track(void)
	{
		IF_(!check());

		Mat *pM = m_pV->getMatRGB();
		IF_(pM->empty());
		Mat m = *pM;

		if (m_iSet > m_iInit)
		{
			// init a new track target
			if (!m_pTracker.empty())
				m_pTracker.release();

			createTracker();
			m_pTracker->init(m, m_newBB);
			m_trackState = track_update;
			m_rBB = m_newBB;
			m_iInit = m_iSet;
		}
		else
		{
			// track update
			IF_(m_trackState != track_update);
			IF_(m_pTracker.empty());

			//		m_pTracker->update(m, m_rBB);
			//		m_bb = bbScale(rect2BB<Vector4f>(m_rBB), 1.0/m.cols, 1.0/m.rows);
		}
	}

}
