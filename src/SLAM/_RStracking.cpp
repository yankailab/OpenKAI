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
	m_bOpen = false;
}

_RStracking::~_RStracking()
{
}

bool _RStracking::init(void* pKiss)
{
	IF_F(!_SlamBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	return true;
}

bool _RStracking::open(void)
{
	IF_T(m_bOpen);

	try
	{
		rs2::config cfg;
	    cfg.enable_stream(RS2_STREAM_POSE, RS2_FORMAT_6DOF);

		auto profile = m_rsPipe.start(cfg);

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

	m_bOpen = true;
	return true;
}

void _RStracking::close(void)
{
	if (m_threadMode == T_THREAD)
	{
		goSleep();
		while (!bSleeping());
	}

	m_rsPipe.stop();
	this->_SlamBase::close();
}

bool _RStracking::start(void)
{
	IF_F(!this->_SlamBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _RStracking::update(void)
{
	while (m_bThreadON)
	{
		if (!m_bOpen)
		{
			if (!open())
			{
				LOG_E("Cannot open RealSense");
				this->sleepTime(USEC_1SEC);
				continue;
			}
		}

		this->autoFPSfrom();

		try
		{
	        auto frames = m_rsPipe.wait_for_frames();
	        auto f = frames.first_or_default(RS2_STREAM_POSE);

	        // Cast the frame to pose_frame and get its data
	        auto pose_data = f.as<rs2::pose_frame>().get_pose_data();
	        float r[16];

	        // Calculate current transformation matrix
	        calc_transform(pose_data, r);

	        // From the matrix we found, get the new location point
	        rs2_vector tr{ r[12], r[13], r[14] };

	        // Create a new point to be added to the trajectory
	        tracked_point p{ tr , pose_data.tracker_confidence };

	        // Register the new point
	        tracker.add_to_trajectory(p);

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

		this->autoFPSto();
	}
}

void _RStracking::draw(void)
{
	this->_SlamBase::draw();
}

}
#endif
