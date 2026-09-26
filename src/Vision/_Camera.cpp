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
	}

	_Camera::~_Camera()
	{
		close();
	}

	bool _Camera::loadConfig(void)
	{
		IF_F(!_VisionBase::loadConfig());
		const json &j = *m_pJ;

		jKv(j, "deviceID", m_deviceID);
		jKv(j, "nInitRead", m_nInitRead);
		jKv(j, "bResetCam", m_bResetCam);

		return true;
	}

	bool _Camera::saveConfig(bool bExport)
	{
		IF_F(!_VisionBase::saveConfig(false));

		json &j = *m_pJ;
		j["deviceID"] = m_deviceID;
		j["nInitRead"] = m_nInitRead;
		j["bResetCam"] = m_bResetCam;

		IF__(!bExport, true);
		return m_pJcfg->saveToFile();
	}

	bool _Camera::open(void)
	{
		IF_F(!check());
		IF__(m_bOpened, true);

		m_camera.open(m_deviceID);
		if (!m_camera.isOpened())
		{
			LOG_E("Cannot open camera: " + i2str(m_deviceID));
			return false;
		}

		m_camera.set(CAP_PROP_FRAME_WIDTH, m_vSizeRGB.x());
		m_camera.set(CAP_PROP_FRAME_HEIGHT, m_vSizeRGB.y());
		m_camera.set(CAP_PROP_FPS, m_pT->getTargetFPS());

		Mat mCam;
		for (int i = 0; i < m_nInitRead; i++)
		{
			while (!m_camera.read(mCam))
				;
		}
		
		{
			std::lock_guard<std::mutex> lock(m_mutexRGB);
			mCam.copyTo(m_mRGB);
		}

		m_vSizeRGB.x() = mCam.cols;
		m_vSizeRGB.y() = mCam.rows;

		m_bOpened = true;
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
		return m_pT->startThread(getUpdate, this);
	}

	void _Camera::update(void)
	{
		while (m_pT->bRun())
		{
			if (!m_bOpened)
			{
				if (!open())
				{
					m_pT->sleepT(NSEC_SEC);
					continue;
				}
			}

			m_pT->autoFPS();

			Mat mCam;
			while (!m_camera.read(mCam))
				;
			{
				std::lock_guard<std::mutex> lock(m_mutexRGB);
				mCam.copyTo(m_mRGB);
			}

			if (m_bResetCam)
			{
				m_camera.release();
				m_bOpened = false;
			}
		}
	}

}
