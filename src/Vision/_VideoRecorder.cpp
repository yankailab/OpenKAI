/*
 * _VideoRecorder.cpp
 *
 *  Created on: June 11, 2026
 *      Author: yankai
 */

#include "_VideoRecorder.h"

namespace kai
{

	_VideoRecorder::_VideoRecorder()
	{
		m_type = vision_videoRecorder;
		m_pV = nullptr;

		m_gstOutput = "appsrc ! videoconvert ! fbdevsink";
		m_vSize.set(1280, 720);
	}

	_VideoRecorder::~_VideoRecorder()
	{
	}

	bool _VideoRecorder::init(const json &j)
	{
		IF_F(!_VisionBase::init(j));

		//		jKv<float>(j, "vTrange", m_vTrange);
		jKv<int>(j, "vSize", m_vSize);
		IF_F(m_vSize.area() <= 0);

		jKv(j, "gstOutput", m_gstOutput);
		if (!m_gstOutput.empty())
		{
			if (!m_gst.open(m_gstOutput,
							CAP_GSTREAMER,
							0,
							m_pT->getTargetFPS(),
							cv::Size(m_vSize.x, m_vSize.y),
							true))
			{
				LOG_E("Cannot open GStreamer output");
				return false;
			}
		}

		return true;
	}

	bool _VideoRecorder::link(const json &j, ModuleMgr *pM)
	{
		IF_F(!this->_VisionBase::link(j, pM));

		string n = "";
		jKv(j, "_VisionBase", n);
		m_pV = (_VisionBase *)(pM->findModule(n));
		NULL_F(m_pV);

		return true;
	}

	bool _VideoRecorder::start(void)
	{
		NULL_F(m_pT);
		return m_pT->startThread(getUpdate, this);
	}

	void _VideoRecorder::update(void)
	{
		while (m_pT->bAlive())
		{
			m_pT->autoFPS();

			recFrame();
		}
	}

	void _VideoRecorder::recFrame(void)
	{
		NULL_(m_pV);
		Frame *pF = m_pV->getFrameRGB();
		NULL_(pF);

		Mat mT = *pF->m();
		IF_(mT.empty());
		IF_(mT.type() != CV_32FC1);

		Mat mRaw;
		mT.convertTo(mRaw, CV_16C1);


		m_gst << m;
	}

	void _VideoRecorder::console(const json &j, void *pJSONbase)
	{
		string cmd;
		IF_(!jKv(j, "cmd", cmd));

		// if (cmd == "setThermal")
		// {
		// 	jKv<float>(j, "vTrange", m_vTrange);
		// }

		this->_VisionBase::console(j, pJSONbase);
	}

}
