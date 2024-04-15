/*
 * _Camera.cpp
 *
 *  Created on: Aug 22, 2015
 *      Author: yankai
 */

#include "_Camera.h"

namespace kai
{

	_Camera::_Camera()
	{
		m_type = vision_camera;
		m_deviceID = 0;
		m_nInitRead = 1;
		m_bResetCam = false;
	}

	_Camera::~_Camera()
	{
		close();
	}

	bool _Camera::init(void *pKiss)
	{
		IF_F(!_VisionBase::init(pKiss));
		Kiss *pK = (Kiss *)pKiss;

		pK->v("deviceID", &m_deviceID);
		pK->v("nInitRead", &m_nInitRead);
		pK->v("bResetCam", &m_bResetCam);

		return true;
	}

	bool _Camera::open(void)
	{
		IF_F(check() < 0);
		IF_T(m_bOpen);

		m_camera.open(m_deviceID);
		if (!m_camera.isOpened())
		{
			LOG_E("Cannot open camera: " + i2str(m_deviceID));
			return false;
		}

		m_camera.set(CAP_PROP_FRAME_WIDTH, m_vSizeRGB.x);
		m_camera.set(CAP_PROP_FRAME_HEIGHT, m_vSizeRGB.y);
		m_camera.set(CAP_PROP_FPS, m_pT->getTargetFPS());

		Mat mCam;
		for (int i = 0; i < m_nInitRead; i++)
		{
			while (!m_camera.read(mCam))
				;
		}
		m_fRGB.copy(mCam);

		m_vSizeRGB.x = mCam.cols;
		m_vSizeRGB.y = mCam.rows;

		m_bOpen = true;
		return true;
	}

	void _Camera::close(void)
	{
		this->_VisionBase::close();
		m_camera.release();
	}

	bool _Camera::start(void)
	{
		NULL_F(m_pT);
		return m_pT->start(getUpdate, this);
	}

	void _Camera::update(void)
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
			while (!m_camera.read(mCam))
				;
			m_fRGB.copy(mCam);

			if (m_bResetCam)
			{
				m_camera.release();
				m_bOpen = false;
			}

			m_pT->autoFPSto();
		}
	}

}
