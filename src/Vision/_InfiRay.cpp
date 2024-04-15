/*
 * _InfiRay.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_InfiRay.h"

namespace kai
{

	_InfiRay::_InfiRay()
	{
		m_type = vision_infiRay;
		m_deviceID = 0;
		m_nInitRead = 1;
		m_bResetCam = false;
	}

	_InfiRay::~_InfiRay()
	{
		close();
	}

	bool _InfiRay::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("deviceID", &m_deviceID);
		pK->v("nInitRead", &m_nInitRead);
		pK->v("bResetCam", &m_bResetCam);

		return true;
	}

	bool _InfiRay::open(void)
	{
		IF_F(check() < 0);
		IF_T(m_bOpen);

		m_InfiRay.open(m_deviceID, CAP_V4L2);
		if (!m_InfiRay.isOpened())
		{
			LOG_E("Cannot open camera: " + i2str(m_deviceID));
			return false;
		}

		m_InfiRay.set(CAP_PROP_CONVERT_RGB, 0);
//		m_InfiRay.set(CAP_PROP_FORMAT, -1);
		m_InfiRay.set(CAP_PROP_ZOOM, 0x8004);
//		m_InfiRay.set(CAP_PROP_ZOOM, 0x8005);
//		m_InfiRay.set(CAP_PROP_ZOOM, 0x8000);
		// m_InfiRay.set(CAP_PROP_FRAME_WIDTH, m_vSize.x);
		// m_InfiRay.set(CAP_PROP_FRAME_HEIGHT, m_vSize.y);
		// m_InfiRay.set(CAP_PROP_FPS, m_pT->getTargetFPS());

		Mat mCam;
		for (int i = 0; i < m_nInitRead; i++)
		{
			while (!m_InfiRay.read(mCam))
				;
		}
//		m_fRGB.copy(mCam);

		m_vSizeRGB.x = mCam.cols;
		m_vSizeRGB.y = mCam.rows;

		m_bOpen = true;
		return true;
	}

	void _InfiRay::close(void)
	{
		this->_VisionBase::close();
		m_InfiRay.release();
	}

	bool _InfiRay::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _InfiRay::update(void)
	{
		while (m_pT->bAlive())
		{
			if (!m_bOpen)
			{
				if (!open())
				{
					m_pT->sleepT(SEC_2_USEC);
					continue;
				}
			}

			m_pT->autoFPSfrom();

			Mat mCam;
			while (!m_InfiRay.read(mCam))
				;

        	Mat mG, mRGB;

			//Mat mYUV = cv::Mat(m_vSize.y, m_vSize.x, CV_8UC2, mCam.data, Mat::AUTO_STEP);
			//cv::cvtColor(mYUV, mRGB, COLOR_YUV2RGB_YUY2);
			int length = mCam.dataend - mCam.datastart;
			Mat mThermal = cv::Mat(m_vSizeRGB.y, m_vSizeRGB.x, CV_16UC1, mCam.data, Mat::AUTO_STEP);
			mThermal.convertTo(mG, CV_8UC1, 1.0/257);
			cv::cvtColor(mG, mRGB, COLOR_GRAY2BGR);

//			normalize(mThermal, mRGB);

//			m_fRGB.copy(mThermal);
			m_fRGB.copy(mRGB);
//			m_fRGB.copy(mCam);

			if (m_bResetCam)
			{
				m_InfiRay.release();
				m_bOpen = false;
			}

			m_pT->autoFPSto();
		}
	}

}
