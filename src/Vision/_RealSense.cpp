/*
 * _RealSense.cpp
 *
 *  Created on: Apr 6, 2018
 *      Author: yankai
 */

#include "_RealSense.h"

#ifdef USE_REALSENSE

namespace kai
{

_RealSense::_RealSense()
{
	m_type = vision_realsense;
	m_pTPP = new _ThreadBase();

	m_vPreset = "High Density";
	m_rsRGB = true;
	m_rsFPS = 30;
	m_rsDFPS = 30;
	m_fDec = 0.0;
	m_fSpat = 0.0;
	m_bAlign = false;
	m_rspAlign = NULL;

	m_iHistFrom = 0;
	m_dScale = 1.0;
}

_RealSense::~_RealSense()
{
	DEL(m_pTPP);
	DEL(m_rspAlign);
}

bool _RealSense::init(void* pKiss)
{
	IF_F(!_DepthVisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;

	pK->v("rsFPS", &m_rsFPS);
	pK->v("rsDFPS", &m_rsDFPS);
	pK->v("vPreset", &m_vPreset);
	pK->v("rsRGB", &m_rsRGB);
	pK->v("fDec", &m_fDec);
	pK->v("fSpat", &m_fSpat);
	pK->v("bAlign", &m_bAlign);

	return true;
}

bool _RealSense::open(void)
{
	IF_T(m_bOpen);

	try
	{
		rs2::config cfg;
		cfg.enable_stream(RS2_STREAM_DEPTH, m_wD, m_hD, RS2_FORMAT_Z16, m_rsDFPS);
		if (m_rsRGB)
		{
			cfg.enable_stream(RS2_STREAM_COLOR, m_w, m_h, RS2_FORMAT_BGR8, m_rsFPS);
		}

		auto profile = m_rsPipe.start(cfg);

		auto sensor = profile.get_device().first<rs2::depth_sensor>();

		// TODO: At the moment the SDK does not offer a closed enum for D400 visual presets
		// (because they keep changing)
		// As a work-around we try to find the High-Density preset by name
		// We do this to reduce the number of black pixels
		// The hardware can perform hole-filling much better and much more power efficient then our software
		auto range = sensor.get_option_range(RS2_OPTION_VISUAL_PRESET);
		for (auto i = range.min; i < range.max; i += range.step)
		{
			IF_CONT(
					std::string(
							sensor.get_option_value_description(
									RS2_OPTION_VISUAL_PRESET, i)) != m_vPreset);

			sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);
			break;
		}

		if (m_fDec > 0.0)
			m_rsfDec.set_option(RS2_OPTION_FILTER_MAGNITUDE, m_fDec);
		if (m_fSpat > 0.0)
			m_rsfSpat.set_option(RS2_OPTION_HOLES_FILL, m_fSpat);

		rs2::frameset rsFrameset = m_rsPipe.wait_for_frames();

		if (m_rsRGB)
		{
			if (m_bAlign)
			{
				m_rspAlign = new rs2::align(rs2_stream::RS2_STREAM_COLOR);
				rs2::frameset rsFramesetAlign = m_rspAlign->process(rsFrameset);
				m_rsColor = rsFramesetAlign.get_color_frame();
				m_rsDepth = rsFramesetAlign.get_depth_frame();
			}
			else
			{
				m_rsColor = rsFrameset.get_color_frame();
				m_rsDepth = rsFrameset.get_depth_frame();
			}

			m_w = m_rsColor.as<rs2::video_frame>().get_width();
			m_h = m_rsColor.as<rs2::video_frame>().get_height();
			m_cW = m_w / 2;
			m_cH = m_h / 2;
		}
		else
		{
			m_rsDepth = rsFrameset.get_depth_frame();
		}

		if (m_fDec > 0.0)
			m_rsDepth = m_rsfDec.process(m_rsDepth);
		if (m_fSpat > 0.0)
			m_rsDepth = m_rsfSpat.process(m_rsDepth);

		m_wD = m_rsDepth.as<rs2::video_frame>().get_width();
		m_hD = m_rsDepth.as<rs2::video_frame>().get_height();

	} catch (const rs2::camera_disconnected_error& e)
	{
		LOG_E("Realsense disconnected");
		return false;
	} catch (const rs2::recoverable_error& e)
	{
		LOG_E("Realsense open failed");
		return false;
	} catch (const rs2::error& e)
	{
		LOG_E("Realsense error");
		return false;
	} catch (const std::exception& e)
	{
		LOG_E("Realsense exception");
		return false;
	}

	m_bOpen = true;
	return true;
}

void _RealSense::close(void)
{
	if (m_threadMode == T_THREAD)
	{
		goSleep();
		while (!bSleeping())
			;
		while (!m_pTPP->bSleeping())
			;
	}

	m_rsPipe.stop();
	this->_VisionBase::close();
}

bool _RealSense::start(void)
{
	IF_F(!this->_ThreadBase::start());

	m_bThreadON = true;
	int retCode = pthread_create(&m_threadID, 0, getUpdateThread, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	retCode = pthread_create(&m_pTPP->m_threadID, 0, getTPP, this);
	if (retCode != 0)
	{
		m_bThreadON = false;
		return false;
	}

	return true;
}

void _RealSense::update(void)
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
			rs2::frameset rsFrameset = m_rsPipe.wait_for_frames();

			if (m_rsRGB)
			{
				if (m_bAlign)
				{
					rs2::frameset rsFramesetAlign = m_rspAlign->process(rsFrameset);
					m_rsColor = rsFramesetAlign.get_color_frame();
					m_rsDepth = rsFramesetAlign.get_depth_frame();
				}
				else
				{
					m_rsColor = rsFrameset.get_color_frame();
					m_rsDepth = rsFrameset.get_depth_frame();
				}

				m_fBGR.copy(
						Mat(Size(m_w, m_h), CV_8UC3, (void*) m_rsColor.get_data(),
								Mat::AUTO_STEP));
			}
			else
			{
				m_rsDepth = rsFrameset.get_depth_frame();
			}

		} catch (const rs2::camera_disconnected_error& e)
		{
			LOG_E("Realsense disconnected");
		} catch (const rs2::recoverable_error& e)
		{
			LOG_E("Realsense open failed");
		} catch (const rs2::error& e)
		{
			LOG_E("Realsense error");
		} catch (const std::exception& e)
		{
			LOG_E("Realsense exception");
		}

		m_pTPP->wakeUp();

		this->autoFPSto();
	}
}

void _RealSense::updateTPP(void)
{
	while (m_bThreadON)
	{
		m_pTPP->sleepTime(0);

		if (m_fDec > 0.0)
			m_rsDepth = m_rsfDec.process(m_rsDepth);
		if (m_fSpat > 0.0)
			m_rsDepth = m_rsfSpat.process(m_rsDepth);
		if (m_pDepthWin)
			m_rsDepthShow = m_rsDepth;

		Mat mZ = Mat(Size(m_wD, m_hD), CV_16UC1, (void*) m_rsDepth.get_data(), Mat::AUTO_STEP);
		Mat mD;
		mZ.convertTo(mD, CV_32FC1);

		auto depth_scale = m_rsPipe.get_active_profile().get_device().first<rs2::depth_sensor>().get_depth_scale();
		m_fDepth = mD * depth_scale;
	}
}

void _RealSense::draw(void)
{
	if (m_pDepthWin)
	{
		IF_(m_fDepth.bEmpty());
		rs2::colorizer rsColorMap;
		rs2::frame dColor = rsColorMap.process(m_rsDepthShow);
		Mat mDColor(Size(m_wD, m_hD), CV_8UC3, (void*) dColor.get_data(),
				Mat::AUTO_STEP);
		m_depthShow = mDColor;
	}

	this->_DepthVisionBase::draw();
}

}
#endif
