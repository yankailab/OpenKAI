/*
 * _RStracking.cpp
 *
 *  Created on: Oct 29, 2019
 *      Author: yankai
 */

#include "_RStracking.h"

#ifdef USE_REALSENSE

namespace kai
{

_RStracking::_RStracking()
{
}

_RStracking::~_RStracking()
{
	close();
}

bool _RStracking::init(void* pKiss)
{
	IF_F(!this->_SlamBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

bool _RStracking::open(void)
{
	IF_T(m_bReady);

	try
	{
		rs2::config cfg;
	    cfg.enable_stream(RS2_STREAM_POSE, RS2_FORMAT_6DOF);

		m_rsPipe.start(cfg);

		rs2::frameset rsFrameset = m_rsPipe.wait_for_frames();

	} catch (const rs2::camera_disconnected_error& e)
	{
		LOG_E("Realsense tracking disconnected");
		return false;
	} catch (const rs2::recoverable_error& e)
	{
		LOG_E("Realsense tracking open failed");
		return false;
	} catch (const rs2::error& e)
	{
		LOG_E("Realsense tracking error");
		return false;
	} catch (const std::exception& e)
	{
		LOG_E("Realsense tracking exception");
		return false;
	}

	m_bReady = true;
	return true;
}

void _RStracking::close(void)
{
	IF_(!m_bReady);
	m_rsPipe.stop();
	m_bReady = false;
}

bool _RStracking::start(void)
{
    NULL_F(m_pT);
	return m_pT->start(getUpdate, this);
}

void _RStracking::update(void)
{
	while(m_pT->bRun())
	{
		if (!m_bReady)
		{
			if (!open())
			{
				LOG_E("Cannot open RealSense tracking");
				m_pT->sleepTime(USEC_1SEC);
				continue;
			}
		}

		if(m_bReset)
		{
			close();
			this->_SlamBase::resetAll();
			continue;
		}

		m_pT->autoFPSfrom();

		try
		{
	        auto frames = m_rsPipe.wait_for_frames();
	        auto f = frames.first_or_default(RS2_STREAM_POSE);
	        auto pose = f.as<rs2::pose_frame>().get_pose_data();

	        m_vT.x = pose.translation.x;
	        m_vT.y = pose.translation.y;
	        m_vT.z = pose.translation.z;

	        m_vQ.x = pose.rotation.x;
	        m_vQ.y = pose.rotation.y;
	        m_vQ.z = pose.rotation.z;
	        m_vQ.w = pose.rotation.w;

	        m_confidence = pose.tracker_confidence;

		} catch (const rs2::camera_disconnected_error& e)
		{
			LOG_E("Realsense tracking disconnected");
		} catch (const rs2::recoverable_error& e)
		{
			LOG_E("Realsense tracking open failed");
		} catch (const rs2::error& e)
		{
			LOG_E("Realsense tracking error");
		} catch (const std::exception& e)
		{
			LOG_E("Realsense exception");
		}

		m_pT->autoFPSto();
	}
}

void _RStracking::draw(void)
{
	this->_SlamBase::draw();
}

}
#endif
