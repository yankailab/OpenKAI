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

	m_rsFPS = 30;
	m_rsDFPS = 30;
}

_RealSense::~_RealSense()
{
	reset();
}

bool _RealSense::init(void* pKiss)
{
	IF_F(!_DepthVisionBase::init(pKiss));
	Kiss* pK = (Kiss*) pKiss;
	pK->m_pInst = this;

	KISSm(pK,rsFPS);
	KISSm(pK,rsDFPS);

	return true;
}

void _RealSense::reset()
{
	this->_DepthVisionBase::reset();

}

bool _RealSense::open(void)
{
	rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_COLOR, m_w, m_h, RS2_FORMAT_BGR8, m_rsFPS);
    cfg.enable_stream(RS2_STREAM_DEPTH, m_wD, m_hD, RS2_FORMAT_Z16, m_rsDFPS);

	m_rsPipe.start(cfg);

	rs2::frameset rsFrame = m_rsPipe.wait_for_frames();

	m_rsColor = rsFrame.get_color_frame();
	m_w = m_rsColor.as<rs2::video_frame>().get_width();
	m_h = m_rsColor.as<rs2::video_frame>().get_height();
	m_cW = m_w / 2;
	m_cH = m_h / 2;

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

		Mat* pSrc;
		Mat* pDest;
		Mat* pTmp;

		//Color
		rs2::frameset rsFrame = m_rsPipe.wait_for_frames();
		m_rsColor = rsFrame.get_color_frame();

		Mat mColor(Size(m_w, m_h), CV_8UC3, (void*)m_rsColor.get_data(), Mat::AUTO_STEP);
		pSrc = &mColor;
		pDest = &m_mTemp;

		if (m_bFlip)
		{
			cv::flip(*pSrc, *pDest, -1);
			SWAP(pSrc, pDest, pTmp);
		}
		m_pBGR->update(pSrc);
		if(m_pGray)
			m_pGray->getGrayOf(m_pBGR);
		if (m_pHSV)
			m_pHSV->getHSVOf(m_pBGR);

		//Depth
		m_rsDepth = rsFrame.get_depth_frame();
		m_mZ = Mat(Size(m_wD, m_hD), CV_16UC1, (void*) m_rsDepth.get_data(), Mat::AUTO_STEP);
	    m_mZ.convertTo(m_mD, CV_32FC1);
	    auto depth_scale = m_rsPipe.get_active_profile()
	        .get_device()
	        .first<rs2::depth_sensor>()
	        .get_depth_scale();
	    m_mD *= depth_scale;

		pSrc = &m_mD;
		pDest = &m_mTemp;

		if (m_bFlip)
		{
			cv::flip(*pSrc, *pDest, -1);
			SWAP(pSrc, pDest, pTmp);
		}
		m_pDepth->update(pSrc);

		updateFilter();

		this->autoFPSto();
	}
}

bool _RealSense::draw(void)
{
	IF_F(m_pDepth->empty());

	rs2::colorizer rsColorMap;
	rs2::frame dColor = rsColorMap(m_rsDepth);
	Mat mDColor(Size(m_wD, m_hD), CV_8UC3, (void*)dColor.get_data(), Mat::AUTO_STEP);
	m_pDepthShow->update(&mDColor);

	return this->_DepthVisionBase::draw();
}

}
#endif
