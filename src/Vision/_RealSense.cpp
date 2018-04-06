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

//	KISSm(pK,);

	return true;
}

void _RealSense::reset()
{
	this->_DepthVisionBase::reset();

}

bool _RealSense::open(void)
{
	m_rsPipe.start();

//	m_width = ;
//	m_height = ;
//	m_centerH = m_width / 2;
//	m_centerV = m_height / 2;

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
		Mat  mTemp;

		//Color
		rs2::frameset rsFrame = m_rsPipe.wait_for_frames();
		rs2::frame rsColor = rsFrame.get_color_frame();
		m_w = rsColor.as<rs2::video_frame>().get_width();
		m_h = rsColor.as<rs2::video_frame>().get_height();

		Mat mColor(Size(m_w, m_h), CV_8UC3, (void*)rsColor.get_data(), Mat::AUTO_STEP);
		pSrc = &mColor;
		pDest = &mTemp;

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
		rs2::colorizer rsColorMap;
		rs2::frame rsDepth = rsColorMap(rsFrame.get_depth_frame());
		m_wD = rsDepth.as<rs2::video_frame>().get_width();
		m_hD = rsDepth.as<rs2::video_frame>().get_height();
		Mat mDepth(Size(m_wD, m_hD), CV_8UC3, (void*) rsDepth.get_data(), Mat::AUTO_STEP);

		pSrc = &mDepth;
		pDest = &mTemp;

		if (m_bFlip)
		{
			cv::flip(*pSrc, *pDest, -1);
			SWAP(pSrc, pDest, pTmp);
		}
		m_pDepth->update(pSrc);

//		//Normalized Depth
//		if(m_pDepthNorm)
//		{
//			if (m_depthNormInt.x != m_depthNormInt.y)
//			{
//				double alpha = 255.0/(m_depthNormInt.y-m_depthNormInt.x);
//				double beta = -alpha * m_depthNormInt.x;
//
//				m_pDepth->getGMat()->convertTo(*m_pDepthNorm->getGMat(), CV_8U, alpha, beta);
//				m_pDepthNorm->updatedGMat();
//			}
//		}

		this->autoFPSto();
	}
}

}
#endif
