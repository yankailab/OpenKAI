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
		IF_F(!_TrackerBase::saveConfig(false));

		IF__(!bExport, true);
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
		std::lock_guard lock(m_mutex);
		IF_(m_trackState == track_stop);

		Mat m;
		const uint64_t tStamp = m_pRGBin->get(m);
		if (m.empty())
		{
			m_trackState = track_stop;
			m_bb.setZero();
			return;
		}

		const bool bInit = m_iSet > m_iInit;
		IF_(!bInit && tStamp == m_tFrame);
		m_tFrame = tStamp;
		bool bTracked = false;
		try
		{
			if (bInit)
			{
				if (!m_pTracker.empty())
					m_pTracker.release();

				createTracker();
				m_rBB = m_newBB & Rect(0, 0, m.cols, m.rows);
				if (!m_pTracker.empty() && m_rBB.area() > 0)
				{
					m_pTracker->init(m, m_rBB);
					bTracked = true;
				}
				m_iInit = m_iSet;
			}
			else if (!m_pTracker.empty())
			{
				bTracked = m_pTracker->update(m, m_rBB);
			}

		}
		catch (const cv::Exception &)
		{
			bTracked = false;
			m_iInit = m_iSet;
		}
		m_rBB &= Rect(0, 0, m.cols, m.rows);
		bTracked = bTracked && m_rBB.area() > 0;

		m_trackState = bTracked ? track_update : track_stop;
		m_bb.setZero();
		if (bTracked)
		{
			BBOX_OBJ bb;
			bb.setType(obj_bbox);
			bb.setPos(Vector3f(m_rBB.x, m_rBB.y, 0));
			bb.setDim(Vector3f(m_rBB.width, m_rBB.height, 0));
			m_bb = bbScale(bb.getBB2D(), 1.0f / m.cols, 1.0f / m.rows);
			// A restart may initialize the same capture again; publish each capture once.
			if (tStamp != m_tPublished)
			{
				m_pBBout->setContainerDim(Vector3f(m.cols, m.rows, 0));
				m_pBBout->add({bb}, tStamp);
				m_tPublished = tStamp;
			}
		}
	}

}
