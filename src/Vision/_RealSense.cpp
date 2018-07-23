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
	m_type = realsense;

	m_pDepthWin = NULL;
	m_vPreset = "High Density";
	m_rsRGB = true;
	m_rsFPS = 30;
	m_rsDFPS = 30;
}

_RealSense::~_RealSense()
{
}

bool _RealSense::init(void* pKiss)
{
	IF_F(!_DepthVisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,rsFPS);
	KISSm(pK,rsDFPS);
	KISSm(pK,vPreset);
	KISSm(pK,rsRGB);

	return true;
}

bool _RealSense::open(void)
{
	rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_DEPTH, m_wD, m_hD, RS2_FORMAT_Z16, m_rsDFPS);
    if(m_rsRGB)
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
        IF_CONT(std::string(sensor.get_option_value_description(RS2_OPTION_VISUAL_PRESET, i)) != m_vPreset);

        sensor.set_option(RS2_OPTION_VISUAL_PRESET, i);
        break;
    }

    auto stream = profile.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();
    auto intrinsics = stream.get_intrinsics();

	rs2::frameset rsFrame = m_rsPipe.wait_for_frames();

    if(m_rsRGB)
    {
    	m_rsColor = rsFrame.get_color_frame();
    	m_w = m_rsColor.as<rs2::video_frame>().get_width();
    	m_h = m_rsColor.as<rs2::video_frame>().get_height();
    	m_cW = m_w / 2;
    	m_cH = m_h / 2;
    }

	m_rsDepth = rsFrame.get_depth_frame();
	m_wD = m_rsDepth.as<rs2::video_frame>().get_width();
	m_hD = m_rsDepth.as<rs2::video_frame>().get_height();

	m_bOpen = true;
	return true;
}

bool _RealSense::start(void)
{
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

		rs2::frameset rsFrame = m_rsPipe.wait_for_frames();

		//Color
		if(m_rsRGB)
		{
			m_rsColor = rsFrame.get_color_frame();
			m_fBGR.copy(Mat(Size(m_w, m_h), CV_8UC3, (void*)m_rsColor.get_data(), Mat::AUTO_STEP));
		}

		//Depth
		m_rsDepth = rsFrame.get_depth_frame();
		m_mZ = Mat(Size(m_wD, m_hD), CV_16UC1, (void*)m_rsDepth.get_data(), Mat::AUTO_STEP);
	    m_mZ.convertTo(m_mD, CV_32FC1);

	    auto depth_scale = m_rsPipe.get_active_profile()
	        .get_device()
	        .first<rs2::depth_sensor>()
	        .get_depth_scale();

	    m_mD *= depth_scale;
	    m_fDepth = m_mD;

		m_pTPP->wakeUp();

		this->autoFPSto();
	}
}

void _RealSense::updateTPP(void)
{
	while (m_bThreadON)
	{
		m_pTPP->sleepTime(0);

		if(m_rsRGB)
		{
			postProcess();
		}

		postProcessDepth();
		updateFilteredDistance();
	}
}

bool _RealSense::draw(void)
{
	if(m_bShowRawDepth && !m_fDepth.bEmpty())
	{
		rs2::colorizer rsColorMap;
		rs2::frame dColor = rsColorMap(m_rsDepth);
		Mat mDColor(Size(m_wD, m_hD), CV_8UC3, (void*)dColor.get_data(), Mat::AUTO_STEP);
		m_depthShow = mDColor;

		if (m_bFlip)
			m_depthShow = m_depthShow.flip(-1);
	}

	return this->_DepthVisionBase::draw();
}

}
#endif
